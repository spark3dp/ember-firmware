require 'spec_helper'

require 'support/settings'
require 'support/smith'
require 'support/definitions'
require 'support/motor_controller'

module Tests

  class FrontPanel

    attr_reader :buffer

    # i2c_write_pipe_path - path to the named pipe that the main firmware writes I2C data to, the tests read from this pipe
    # i2c_read_pipe_path - path to the named pipe that the main firmware reads I2C data from, the tests write to this pipe
    # interrupt_read_pipe_path - path to the named pipe that the main firmware monitors for activity indicating that the
    # front panel has generated an interrupt, the tests write to this pipe
    def initialize(i2c_write_pipe_path, i2c_read_pipe_path, interrupt_read_pipe_path)
      File.mkfifo(i2c_write_pipe_path)      unless File.pipe?(i2c_write_pipe_path)
      File.mkfifo(i2c_read_pipe_path)       unless File.pipe?(i2c_read_pipe_path)
      File.mkfifo(interrupt_read_pipe_path) unless File.pipe?(interrupt_read_pipe_path)

      @i2c_write_pipe      = File.open(i2c_write_pipe_path, 'r+')
      @i2c_read_pipe       = File.open(i2c_read_pipe_path, 'w+')
      @interrupt_read_pipe = File.open(interrupt_read_pipe_path, 'w+')

      @buffer = []
    end

    def update
      data = @i2c_write_pipe.read_nonblock(1)
      @buffer << data.unpack('C').first
    rescue IO::WaitReadable
      # pipe did not contain data
    end

  end

  describe 'firmware' do
    let(:smith) { Smith.new }

    let!(:motor_controller) {
      MotorController.new(
        tmp_dir(MOTOR_CONTROLLER_I2C_WRITE_PIPE),
        tmp_dir(MOTOR_CONTROLLER_I2C_READ_PIPE),
        tmp_dir(MOTOR_CONTROLLER_INTERRUPT_READ_PIPE)
      )
    }

    let(:front_panel) {
      FrontPanel.new(
        tmp_dir(FRONT_PANEL_I2C_WRITE_PIPE),
        tmp_dir(FRONT_PANEL_I2C_READ_PIPE),
        tmp_dir(FRONT_PANEL_INTERRUPT_READ_PIPE)
      )
    }

    let(:settings) { Settings.new("#{ROOT_DIR}#{SETTINGS_SUB_DIR}#{SETTINGS_FILE}") }

    after(:each) { smith.stop }

    def poll
      start_time = Time.now.to_f
      loop do
        front_panel.update
        return if Time.now.to_f - start_time >= 5
        sleep(0.05)
      end
    end

    scenario 'startup' do

      smith.start(tmp_dir_path)

      poll

      puts front_panel.buffer.inspect

      expect(front_panel.text).to eq("Ready\nLoad your prepped\nprint file via\nnetwork or USB.")
      expect(front_panel.led_sequence).to eq(0)
      expect(front_panel.left_button_text).to be_empty
      expect(front_panel.right_button_text).to be_empty

      expect(motor_controller).to be_enabled

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:r)
      expect(movement.action).to                        eq(MC_HOME)
      expect(movement.parameter).to                     eq(UNITS_PER_REVOLUTION)
      expect(movement.settings.step_angle).to           eq(settings.get(R_STEP_ANGLE))
      expect(movement.settings.units_per_revolution).to eq(settings.get(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR)
      expect(movement.settings.microstepping_factor).to eq(settings.get(MICRO_STEPS_MODE))
      expect(movement.settings.max_jerk).to             eq(settings.get(R_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(R_HOMING_SPEED) * R_SPEED_FACTOR)

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:r)
      expect(movement.action).to                        eq(MC_MOVE)
      expect(movement.parameter).to                     eq(settings.get(R_HOMING_ANGLE) / R_SCALE_FACTOR)
      expect(movement.settings.step_angle).to           eq(settings.get(R_STEP_ANGLE))
      expect(movement.settings.units_per_revolution).to eq(settings.get(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR)
      expect(movement.settings.microstepping_factor).to eq(settings.get(MICRO_STEPS_MODE))
      expect(movement.settings.max_jerk).to             eq(settings.get(R_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(R_HOMING_SPEED) * R_SPEED_FACTOR)

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:z)
      expect(movement.action).to                        eq(MC_HOME)
      expect(movement.parameter).to                     eq(-2 * settings.get(Z_START_PRINT_POSITION))
      expect(movement.settings.step_angle).to           eq(settings.get(Z_STEP_ANGLE))
      expect(movement.settings.units_per_revolution).to eq(settings.get(Z_MICRONS_PER_REV))
      expect(movement.settings.microstepping_factor).to eq(settings.get(MICRO_STEPS_MODE))
      expect(movement.settings.max_jerk).to             eq(settings.get(Z_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(Z_HOMING_SPEED) * Z_SPEED_FACTOR)

      motor_controller.respond_to_interrupt_request

      expect(motor_controller).to be_disabled

      #expect(projector).to show_black
    end
  end

end
