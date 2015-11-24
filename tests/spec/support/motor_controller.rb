require 'mkfifo'
require 'support/definitions'

module Tests
  class MotorController

    # Records movement requests from main firmware
    Motion = Struct.new(:axis, :action, :parameter, :settings)

    # Transforms individual bytes comprising a command into the command's register, action, and parameter values
    class Command

      attr_reader :register, :action, :parameter

      def initialize(register, action, param0, param1, param2, param3)
        @register = register.unpack('C').first
        @action = action.unpack('C').first
        @parameter = [param0, param1, param2, param3].join.unpack('l').first
      end

    end

    # Buffers command bytes as they arrive from the main firmware
    # Very similar to actual command buffer used in motor controller firmware
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

    # Holds current settings for a single axis
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

    # i2c_write_pipe_path - path to the named pipe that the main firmware writes I2C data to, the tests read from this pipe
    # i2c_read_pipe_path - path to the named pipe that the main firmware reads I2C data from, the tests write to this pipe
    # interrupt_read_pipe_path - path to the named pipe that the main firmware monitors for activity indicating that the
    # motor controller has generated an interrupt, the tests write to this pipe
    def initialize(i2c_write_pipe_path, i2c_read_pipe_path, interrupt_read_pipe_path)
      File.mkfifo(i2c_write_pipe_path)      unless File.pipe?(i2c_write_pipe_path)
      File.mkfifo(i2c_read_pipe_path)       unless File.pipe?(i2c_read_pipe_path)
      File.mkfifo(interrupt_read_pipe_path) unless File.pipe?(interrupt_read_pipe_path)

      @i2c_write_pipe      = File.open(i2c_write_pipe_path, 'r+')
      @i2c_read_pipe       = File.open(i2c_read_pipe_path, 'w+')
      @interrupt_read_pipe = File.open(interrupt_read_pipe_path, 'w+')

      @buffer = CommandBuffer.new         # buffer to hold command bytes as they are sent to the motor controller
      @status = MC_STATUS_SUCCESS         # current motor controller status
      @r_axis_settings = AxisSettings.new # holds the current r axis settings
      @z_axis_settings = AxisSettings.new # holds the current z axis settings
      @enabled = false                    # flag recording whether or not the motor controller is enabled
      @movements = []                     # array holding movement requests sent to motor controller
      @interrupt_requests = 0             # number of interrupt requests sent to motor controller
      @read_status_requested = false      # synchronization flag
      @cleared = false                    # has motor controller received an MC_CLEAR command
      @paused = false                     # is motor controller paused
    end

    # Retrieves the next movement request sent to the motor controller from the main firmware
    # Raises an exception if the motor controller did not receive a movement request within the synchronization timeout
    # Blocks (with timeout) until the main firmware sends an entire command to an action register
    def next_movement
      movement = synchronize { @movements.shift }
      raise 'motor controller did not receive a movement request' unless movement
      movement
    end

    # Returns whether or not the main firmware enabled the motor controller
    # Blocks (with timeout) until the main firmware enables the motor controller
    def enabled?
      synchronize { @enabled }
    end

    # Returns whether or not the main firmware disabled the motor controller (or true if the main firmware never
    # disabled the motor controller)
    # Blocks (with timeout) until the main firmware disables the motor controller (does not block if the main firmware
    # never disabled the motor controller)
    def disabled?
      synchronize { !@enabled }
    end

    # Returns whether or not the main firmware cleared the motor controller (or true if the main firmware never
    # cleared the motor controller)
    # Blocks (with timeout) until the main firmware clears the motor controller (does not block if the main firmware
    # never cleared the motor controller)
    def cleared?
      synchronize { @cleared }
      @cleared = false
    end

    # Returns true if and only if the main firmware paused the motor controller, and hasn't yet resumed it.
    # Blocks (with timeout) until the main firmware pauses the motor controller (does not block if the main firmware
    # never paused the motor controller)
    def paused?
      synchronize { @paused }
    end

    # Generates an interrupt if the motor controller received an interrupt request from the main firmware
    # Raises an exception if the motor controller did not receive an interrupt request within the synchronization timeout
    # Blocks (with timeout) until the motor controller receives an interrupt request
    def respond_to_interrupt_request
      if synchronize { @interrupt_requests > 0 }
        @interrupt_read_pipe.write('1')
        @interrupt_read_pipe.flush
        @interrupt_requests -= 1
        synchronize { @read_status_requested }
        @read_status_requested = false
      else
        raise 'motor controller instructed to respond to interrupt request but did not receive interrupt request command from firmware'
      end
    end

    private

    # Handles incoming data (1 byte per call) from main firmware
    def update
      # read a single byte from the pipe that the main firmware writes I2C data to
      data = @i2c_write_pipe.read_nonblock(1)
      unpacked_data = data.unpack('C').first

      if unpacked_data == MC_STATUS_REG && !@buffer.has_partial_command?
        @read_status_requested = true
        # write status byte to pipe that main firmware reads I2C data from
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

    class ConditionNotMet < StandardError; end

    # Runs a specified block of code repeatedly until the block returns true or total execution time exceeds a timeout
    # Incoming data is handled after each time the block fails to return true
    # Allows synchronization between tests and main firmware
    def synchronize(timeout_seconds = 5)
      start_time_seconds = Time.now.to_f

      begin
        success = yield
        return success if success
        raise ConditionNotMet
      rescue ConditionNotMet
        return false if Time.now.to_f - start_time_seconds >= timeout_seconds
        sleep(0.05)
        update
        retry
      end
    end


    # Processes commands sent to the general register
    def general_command(action)
      case action
        when MC_INTERRUPT
          @interrupt_requests += 1
        when MC_RESET
          @r_axis_settings.reset
          @z_axis_settings.reset
        when MC_CLEAR
          @cleared = true
        when MC_PAUSE
          @paused = true
        when MC_RESUME
          @paused = false
        when MC_ENABLE
          @enabled = true
        when MC_DISABLE
          @enabled = false
        else
          @status = MC_STATUS_COMMAND_UNKNOWN
      end
    end

    # Processes commands sent to a settings register
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

    # Processes commands sent to an action register
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
end