require 'open3'
require 'fcntl'
require 'mkfifo'
require 'json'

require 'spec_helper'
require 'support/definitions'

module Tests


  class Smith

    def start(working_dir)
      Dir.chdir(working_dir) do
        @stdin, @stdout, @stderr, @wait_thr = Open3.popen3('/usr/local/bin/smith')
      end

      # Write the stdout from the dummy server to a file in another thread
      Thread.new do
        Thread.current.abort_on_exception = true
        log = File.open('smith.out', 'w')
        IO.copy_stream(@stdout, log)
        IO.copy_stream(@stderr, log)
      end
    end

    def stop
      if @wait_thr && @wait_thr.alive?
        Process.kill('INT', @wait_thr.pid)
        puts 'Timeout attempting to stop smith' unless @wait_thr.join(5)
      end
    end

  end

  class MotorController

    Motion = Struct.new(:axis, :action, :parameter, :settings)

    class Command

      attr_reader :register, :action, :parameter

      def initialize(register, action, param0, param1, param2, param3)
        @register = register.unpack('C').first
        @action = action.unpack('C').first
        @parameter = [param0, param1, param2, param3].join.unpack('l').first
      end

    end

    class CommandBuffer
      # Size of the buffer in bytes
      BUFFER_SIZE = 128

      # Number of bytes per command message
      COMMAND_SIZE = 6

      # Number of command messages the buffer can hold
      COMMAND_CAPACITY = BUFFER_SIZE / COMMAND_SIZE;

      def initialize
        @head = 0
        @tail = 0
        @bytes_remaining = COMMAND_SIZE
        @received_command_count = 0
        @buffer = []
      end

      def has_command?
        @received_command_count > 0
      end

      def has_partial_command?
        @bytes_remaining != COMMAND_SIZE
      end

      # store the specified byte as a string in the buffer
      # storing as string allows the command to unpack the parameter bytes directly as a signed 32-bit integer
      def add(data)
        unpacked_data = data.unpack('C').first


        if unpacked_data > MC_GENERAL_LOW_FENCEPOST && unpacked_data < MC_GENERAL_HIGH_FENCEPOST &&
            @bytes_remaining == COMMAND_SIZE

          # data represents a general command
          add_byte([MC_GENERAL_REG].pack('C'))
          add_byte(data)
          add_byte("\x00")
          add_byte("\x00")
          add_byte("\x00")
          add_byte("\x00")
        else
          # data is part of a multi-byte command
          add_byte(data)
        end
      end

      def command
        return nil if @received_command_count == 0
        @received_command_count -= 1
        Command.new(remove_byte, remove_byte, remove_byte, remove_byte, remove_byte, remove_byte)
      end

      private

      def add_byte(data)
        # check if the buffer has room for an entire command
        # the buffer might have space for a single byte but the buffer can
        # only accept the data if the capacity exists for the entire command
        raise 'command buffer capacity exceeded' if @received_command_count == COMMAND_CAPACITY

        next_head = (@head + 1) % BUFFER_SIZE
        @buffer[@head] = data
        @head = next_head
        @bytes_remaining -= 1

        if @bytes_remaining == 0
          @received_command_count += 1
          @bytes_remaining = COMMAND_SIZE
        end
      end

      def remove_byte
        data = @buffer[@tail]
        @tail = (@tail + 1) % BUFFER_SIZE
        data
      end

    end

    class AxisSettings
      attr_accessor :step_angle, :units_per_revolution, :microstepping_factor, :speed, :max_jerk

      def initialize
        reset
      end

      def reset
        @step_angle = 0.0
        @units_per_revolution = 0.0
        @microstepping_factor = 0
        @max_jerk = 0.0
        @speed = 0.0
      end
    end

    # i2c_write_pipe_path - path to the named pipe that the firmware writes I2C data to, the tests read from this pipe
    # i2c_read_pipe_path - path to the named pipe that the firmware reads I2C data from, the tests write to this pipe
    # interrupt_read_pipe_path - path to the named pipe that the firmware monitors for activity indicating that the
    # motor controller has generated an interrupt, the tests write to this pipe
    def initialize(i2c_write_pipe_path, i2c_read_pipe_path, interrupt_read_pipe_path)
      File.mkfifo(i2c_write_pipe_path)      unless File.pipe?(i2c_write_pipe_path)
      File.mkfifo(i2c_read_pipe_path)       unless File.pipe?(i2c_read_pipe_path)
      File.mkfifo(interrupt_read_pipe_path) unless File.pipe?(interrupt_read_pipe_path)

      @i2c_write_pipe      = File.open(i2c_write_pipe_path, 'r+')
      @i2c_read_pipe       = File.open(i2c_read_pipe_path, 'w+')
      @interrupt_read_pipe = File.open(interrupt_read_pipe_path, 'w+')

      @buffer = CommandBuffer.new         # buffer to hold commands as they are sent to the motor controller
      @status = MC_STATUS_SUCCESS         # the current motor controller status
      @r_axis_settings = AxisSettings.new # holds the current r axis settings
      @z_axis_settings = AxisSettings.new # holds the current z axis settings
      @enabled = false                    # flag recording whether or not the motor controller is enabled
      @movements = []                     # array holding movement requests sent to motor controller
      @interrupt_requests = 0             # number of interrupt requests sent to motor controller
    end

    def update
      # read a single byte from the pipe that the firmware writes I2C data to
      data = @i2c_write_pipe.read_nonblock(1)
      unpacked_data = data.unpack('C').first

      if unpacked_data == MC_STATUS_REG && !@buffer.has_partial_command?
        # write status byte to pipe that firmware reads I2C data from
        @i2c_read_pipe.write([@status].pack('C'))
        @i2c_read_pipe.flush
        # don't handle byte as part of multi-byte command message
        return
      end

      @buffer.add(data)

      if @buffer.has_command?
        command = @buffer.command
        #puts "got command, register: #{command.register.to_s(16)}, action: #{command.action.to_s(16)}, parameter: #{command.parameter}"
        case command.register
          when MC_GENERAL_REG
            general_command(command.action)
          when MC_ROT_SETTINGS_REG
            settings_command(command.action, command.parameter, @r_axis_settings)
          when MC_ROT_ACTION_REG
            action_command(command.action, command.parameter, @r_axis_settings, :r)
          when MC_Z_SETTINGS_REG
            settings_command(command.action, command.parameter, @z_axis_settings)
          when MC_Z_ACTION_REG
            action_command(command.action, command.parameter, @z_axis_settings, :z)
          else
            @status = MC_STATUS_COMMAND_UNKNOWN
        end
      end
    rescue IO::WaitReadable
      # pipe did not contain data
    end

    def next_movement
      @movements.shift
    end

    def enabled?
      @enabled
    end

    def disabled?
      !@enabled
    end

    def respond_to_interrupt_request
      if @interrupt_requests > 0
        @interrupt_read_pipe.write('1')
        @interrupt_read_pipe.flush
        @interrupt_requests -= 1
      else
        raise 'motor controller instructed to respond to interrupt request but did not receive interrupt request command from firmware'
      end
    end

    private

    def general_command(action)
      case action
        when MC_INTERRUPT
          @interrupt_requests += 1
        when MC_RESET
          @r_axis_settings.reset
          @z_axis_settings.reset
        when MC_CLEAR
        when MC_PAUSE
        when MC_RESUME
        when MC_ENABLE
          @enabled = true
        when MC_DISABLE
          @enabled = false
        else
          @status = MC_STATUS_COMMAND_UNKNOWN
      end
    end

    def settings_command(action, parameter, axis_settings)
      case action
        when MC_STEP_ANGLE
          axis_settings.step_angle = parameter
        when MC_MICROSTEPPING
          axis_settings.microstepping_factor = parameter
        when MC_JERK
          axis_settings.max_jerk = parameter
        when MC_SPEED
          axis_settings.speed = parameter
        when MC_UNITS_PER_REV
          axis_settings.units_per_revolution = parameter
        else
          @status = MC_STATUS_COMMAND_UNKNOWN
      end
    end

    def action_command(action, parameter, axis_settings, axis)
      case action
        when MC_MOVE, MC_HOME
          # record the settings at the time of the action so the tests can make assertions regarding what settings the
          # motor controller carried out the action with
          @movements << Motion.new(axis, action, parameter, axis_settings.clone)
        else
          @status = MC_STATUS_COMMAND_UNKNOWN
      end
    end

  end

  class Settings

    def initialize(path)
      @settings = JSON.parse(File.read(path)).fetch(SETTINGS_ROOT_KEY)
    end

    def get(key)
      @settings.fetch(key)
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
    let(:settings) { Settings.new("#{ROOT_DIR}#{SETTINGS_SUB_DIR}#{SETTINGS_FILE}") }
    after(:each) { smith.stop }

    def poll
      count = 0
      timeout_seconds = 5.0
      poll_interval_seconds = 0.01
      max_count = timeout_seconds / poll_interval_seconds
      loop do
        break if count > max_count
        motor_controller.update
        sleep poll_interval_seconds
        count += 1
      end
    end

    scenario 'startup' do

      smith.start(tmp_dir_path)

      poll

=begin
      expect(front_panel).to display(
        'Ready.
        Load your prepped
        print file via
        network or USB.'
      )
=end

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

      poll

      expect(motor_controller).to be_disabled

      #expect(projector).to show_black
    end
  end

end
