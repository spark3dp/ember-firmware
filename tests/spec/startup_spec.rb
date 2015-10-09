require 'open3'
require 'fcntl'
require 'mkfifo'

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

      # store the specified byte as a string in the buffer
      # storing as string allows the command to unpack the parameter bytes directly as a signed 32-bit integer
      def add(data)
        unpacked_data = data.unpack('C').first

        # ignore read register address
        return if unpacked_data == MC_STATUS_REG && @bytes_remaining == COMMAND_SIZE

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

    def initialize(i2c_read_pipe_path, i2c_write_pipe_path, interrupt_write_pipe_path)
      File.mkfifo(i2c_read_pipe_path)        unless File.pipe?(i2c_read_pipe_path)
      File.mkfifo(i2c_write_pipe_path)       unless File.pipe?(i2c_write_pipe_path)
      File.mkfifo(interrupt_write_pipe_path) unless File.pipe?(interrupt_write_pipe_path)
      @i2c_read_pipe        = File.open(i2c_read_pipe_path, 'r+')
      @i2c_write_pipe       = File.open(i2c_write_pipe_path, 'w+')
      @interrupt_write_pipe = File.open(interrupt_write_pipe_path, 'w+')
      @buffer = CommandBuffer.new
    end


    def update
      @buffer.add(@i2c_read_pipe.read_nonblock(1))
      if @buffer.has_command?
        command = @buffer.command
        puts "got command, register: #{command.register.to_s(16)}, action: #{command.action.to_s(16)}, parameter: #{command.parameter}"
      end
    rescue IO::WaitReadable
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
    after(:each) { smith.stop }

    scenario 'startup' do

      smith.start(tmp_dir_path)

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

=begin

      expect(front_panel).to display(
        'Ready.
        Load your prepped
        print file via
        network or USB.'
      )

      expect(motor_controller.r_axis).to move_home_with_max_travel(settings.get(R_HOMING_ANGLE)).with_settings(
        step_angle: settings.get(R_STEP_ANGLE),
        units_per_rev: settings.get(R_MILLIDEGREES_PER_REV),
        microstepping: settings.get(MICRO_STEPS_MODE),
        jerk: settings.get(R_HOMING_JERK),
        speed: settings.get(R_HOMING_SPEED)
      )

      expect(motor_controller.z_axis).to move_home_with_max_travel(-2 * settings.get(Z_START_PRINT_POSITION)).with_settings(
        step_angle: settings.get(Z_STEP_ANGLE),
        units_per_rev: settings.get(Z_MICRONS_PER_REV),
        microstepping: settings.get(MICRO_STEPS_MODE),
        jerk: settings.get(Z_HOMING_JERK),
        speed: settings.get(Z_HOMING_SPEED)
      )

      motor_controller.respond_to_interrupt_request

      expect(motor_controller).to be_disabled
=end
    end
  end

end
