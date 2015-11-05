require 'spec_helper'

require 'support/settings'
require 'support/smith'
require 'support/definitions'
require 'support/motor_controller'

module Tests

  class FrontPanel

    LED_COUNT = 21

    class CommandBuffer
      def add(data)
      end
    end

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

      @receiving_command = false
      @command = []

      @led_animation_sequence = 0
      @led_brightnesses = Array.new(LED_COUNT, 0)
    end

    # Handles incoming data (1 byte per call) from main firmware
    def update
      # read a single byte from the pipe that the main firmware writes I2C data to
      data = @i2c_write_pipe.read_nonblock(1)
      unpacked_data = data.unpack('C').first

      #puts "front panel got byte: 0x#{unpacked_data.to_s(16)}"

      if @receiving_command
        if unpacked_data == CMD_END
          @receiving_command = false
          frame = @command.drop(2)

          #puts "frame: #{frame.map { |b| "0x#{b.to_s(16)}" }.inspect}"

          start_byte = @command[0].unpack('C').first
          frame_length = @command[1].unpack('C').first

          if start_byte =! CMD_START
            fail "front panel received 0x#{start_byte.to_s(16)} as first command byte, not 0x#{CMD_START.to_s(16)} as expected"
          end

          if frame.empty?
            fail 'front panel received empty command frame'
          end

          if frame.length != frame_length
            fail "front panel received #{frame_length} as frame length, not #{frame.length} (length of received frame)"
          end

          handle_command(frame)

          @command = []
        else
          @command << data
        end
      elsif unpacked_data == FP_COMMAND
        @receiving_command = true
      elsif unpacked_data == DISPLAY_STATUS
        # write status byte to pipe that main firmware reads I2C data from
        @i2c_read_pipe.write([0].pack('C'))
        @i2c_read_pipe.flush
      end
    rescue IO::WaitReadable
      # pipe did not contain data
    end

    private

    def handle_command(frame)
      case (command = frame.shift.unpack('C').first)
        when CMD_SYNC
          puts 'got sync command'
        when CMD_RESET
          puts 'got reset command'
        when CMD_RING
          puts 'got ring command'
          handle_ring_command(frame)
        when CMD_OLED
          puts 'got oled command'
        when CMD_SLEEP
          puts 'got sleep command'
        else
          fail "unknown command: 0x#{command.to_s(16)}"
      end
    end

    def handle_ring_command(ring_command_bytes)
      case (ring_command = ring_command_bytes.shift.unpack('C').first)
        when CMD_RING_LED
          puts 'setting indexed led to given value'
        when CMD_RING_LEDS
          puts "setting all leds to #{ring_command_bytes.join.unpack('S').first}"
          @led_brightnesses = Array.new(LED_COUNT, ring_command_bytes.join.unpack('S').first)
        when CMD_RING_SEQUENCE
          puts "setting leds to sequence: #{ring_command_bytes.first.unpack('C').first}"
          @led_animation_sequence = ring_command_bytes.first.unpack('C').first
        else
          fail "unknown type of ring command: 0x#{ring_command.to_s(16)}"
      end
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

    let(:settings) { Settings.new("#{ROOT_DIR}#{SETTINGS_FILE}") }

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

      # expect(front_panel.text).to eq("Ready\nLoad your prepped\nprint file via\nnetwork or USB.")
      # expect(front_panel.led_sequence).to eq(0)
      # expect(front_panel.left_button_text).to be_empty
      # expect(front_panel.right_button_text).to be_empty

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
