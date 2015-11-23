require 'mkfifo'
require 'support/definitions'

module Tests
  class FrontPanel

    class ByteSequence
      extend Forwardable

      def_delegators :@sequence, :<<, :empty?, :length

      def initialize
        @sequence = []
      end

      def extract_uint8
        @sequence.shift.unpack('C').first
      end

      def extract_uint16
        [@sequence.shift, @sequence.shift].join.unpack('S').first
      end

      def remaining
        @sequence.dup
      end
    end

    class TextLine
      attr_reader :text, :x_position, :y_position, :alignment, :color, :size

      alias :eql? :==

      def initialize(contents)
        @text, @x_position, @y_position, @alignment, @color, @size =
            contents.values_at(:text, :x_position, :y_position, :alignment, :color, :size)
      end

      def ==(other)
        other.class == self.class && other.state == self.state
      end

      def sort_key
        "#{@text}#{@x_position}#{@y_position}"
      end

      protected

      def state
        self.instance_variables.map { |variable| self.instance_variable_get variable }
      end
    end

    attr_reader :led_ring_animation_sequence, :led_ring_brightnesses, :screen_lines

    LED_COUNT = 21

    OLED_ALIGNMENT = { CMD_OLED_SETTEXT => :left, CMD_OLED_CENTERTEXT => :center, CMD_OLED_RIGHTTEXT => :right }

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
      @command = ByteSequence.new

      # initialize to nil, meaning uninitialized
      @led_ring_animation_sequence = nil
      @led_ring_brightnesses = Array.new(LED_COUNT, nil)

      @screen_lines = []
      @status = 0
      @button_read_requested = false

      @logging_enabled = false
    end

    # Enable debug logging
    def enable_logging
      @logging_enabled = true
    end

    # Returns whether or not the front panel currently exactly displays the specified array of TextLines
    # Blocks (with timeout) until the front panel displays the specified screen lines
    def show_text?(screen_lines)
      synchronize { @screen_lines == screen_lines.sort { |a, b| a.sort_key <=> b.sort_key } }
    end

    # Returns whether or not the LED ring brightnesses match the specified values
    # Blocks (with timeout) until the LEDs match the specified values
    def has_led_ring_brightnesses?(values)
      synchronize { @led_ring_brightnesses == values }
    end

    # Returns whether or not the LED ring animation sequence matches the specified value
    # Blocks (with timeout) until the LED ring animation sequence matches the specified value
    def has_led_ring_animation_sequence?(value)
      synchronize { @led_ring_animation_sequence == value }
    end

    # Returns whether or not the main firmware cleared the front panel display
    # Blocks (with timeout) until the main firmware clears the display
    def cleared?
      synchronize { @screen_lines.empty? }
    end

    # Handles incoming data (1 byte per call) from main firmware
    def update
      # read a single byte from the pipe that the main firmware writes I2C data to
      data = @i2c_write_pipe.read_nonblock(1)
      unpacked_data = data.unpack('C').first

      #log "front panel got byte: 0x#{unpacked_data.to_s(16)}, receiving command: #{@receiving_command}"

      if @receiving_command
        if unpacked_data == CMD_END
          @receiving_command = false
          start_byte = @command.extract_uint8
          frame_length = @command.extract_uint8

          if start_byte != CMD_START
            fail "front panel received 0x#{start_byte.to_s(16)} as first command byte, not 0x#{CMD_START.to_s(16)} as expected"
          end

          if @command.empty?
            fail 'front panel received empty command frame'
          end

          if @command.length != frame_length
            fail "front panel received #{frame_length} as frame length, not #{@command.length} (length of received frame)"
          end

          handle_command(@command)

          @command = ByteSequence.new
        else
          @command << data
        end
      elsif unpacked_data == FP_COMMAND
        @receiving_command = true
      elsif unpacked_data == DISPLAY_STATUS
        # write status byte to pipe that main firmware reads I2C data from
        @i2c_read_pipe.write([0].pack('C'))
        @i2c_read_pipe.flush
      elsif unpacked_data == BTN_STATUS
          @button_read_requested = true
          # write status byte to pipe that main firmware reads I2C data from
          @i2c_read_pipe.write([@status].pack('C'))
          @i2c_read_pipe.flush
      end
    end

    # Reads all available data from read pipe and then checks if the specified block returns true
    # If the block returns false, check for more data in the read pipe and then evaluate the block again
    # This process repeats until total execution time exceeds the specified timeout
    def synchronize(timeout_seconds = 10)
      start_time_seconds = Time.now.to_f

      begin
        loop do
          update
          sleep(0.05)
        end
      rescue IO::WaitReadable
        # did updating result in the condition passing?
        success = yield

        # if yes, return
        return success if success

        # if not, try updating again until the timeout expires
        return false if Time.now.to_f - start_time_seconds >= timeout_seconds
        retry
      end
    end

    def
    button_action(button_event)
      @status = button_event
      @interrupt_read_pipe.write('1')
      @interrupt_read_pipe.flush
      synchronize { @button_read_requested }
      @button_read_requested = false
    end

    private

    def handle_command(sequence)
      case (command = sequence.extract_uint8)
        when CMD_SYNC
          log 'got sync command'
        when CMD_RESET
          log 'got reset command'
        when CMD_RING
          log 'got ring command'
          update_led_ring(sequence)
        when CMD_OLED
          log 'got oled command'
          update_oled(sequence)
        when CMD_SLEEP
          log 'got sleep command'
        else
          fail "unknown command: 0x#{command.to_s(16)}"
      end
    end

    def update_oled(sequence)
      case (command = sequence.extract_uint8)
        when CMD_OLED_CLEAR
          log "\tcleared oled"
          @screen_lines = []
        when CMD_OLED_SETTEXT, CMD_OLED_CENTERTEXT, CMD_OLED_RIGHTTEXT
          alignment = OLED_ALIGNMENT[command]
          log "\tdisplay #{alignment}-aligned text"
          x_position = sequence.extract_uint8
          y_position = sequence.extract_uint8
          size = sequence.extract_uint8
          color = sequence.extract_uint16
          length = sequence.extract_uint8
          text = sequence.remaining
          if length != text.length
            fail "command specified text length of #{length}, actual number of bytes: #{text.length}"
          end
          @screen_lines << TextLine.new(
              text:       text.join,
              x_position: x_position,
              y_position: y_position,
              alignment:  alignment,
              color:      color,
              size:       size
          )
          log "\t#{@screen_lines.last.inspect}"
          # keep the screen lines array sorted by text on each line to ease comparisons
          @screen_lines.sort! { |a, b| a.sort_key <=> b.sort_key }
        else
          fail "unknown type of OLED command: 0x#{command.to_s(16)}"
      end
    end

    def update_led_ring(sequence)
      case (command = sequence.extract_uint8)
        when CMD_RING_LED
          log "\tsetting indexed led to given value"
        when CMD_RING_LEDS
          @led_ring_brightnesses = Array.new(LED_COUNT, sequence.extract_uint16)
          log "\tsetting all leds to #{@led_ring_brightnesses[0]}"
        when CMD_RING_SEQUENCE
          @led_ring_animation_sequence = sequence.extract_uint8
          log "\tsetting leds to sequence: #{@led_ring_animation_sequence}"
        else
          fail "unknown type of ring command: 0x#{command.to_s(16)}"
      end
    end

    def log(message)
      puts message if @logging_enabled
    end

  end

  RSpec::Matchers.define :show_text do |*expected_lines|
    match do |front_panel|
      @expected_lines = expected_lines.map { |l| FrontPanel::TextLine.new(l) }
      front_panel.show_text?(@expected_lines)
    end

    failure_message do |front_panel|
      expected =        @expected_lines.map { |l| "\t#{l.inspect}" }.join("\n")
      actual = front_panel.screen_lines.map { |l| "\t#{l.inspect}" }.join("\n")
      "expected front panel to show text:\n#{expected}\nactual contents of screen:\n#{actual}"
    end
  end

  RSpec::Matchers.define :have_led_ring_brightnesses do |expected_brightnesses|
    match do |front_panel|
      front_panel.has_led_ring_brightnesses?(expected_brightnesses)
    end

    failure_message do |front_panel|
      "expected led brightnesses to equal:\n\t#{expected_brightnesses.inspect}\n"\
      "actual led brightnesses:\n\t#{front_panel.led_ring_brightnesses.inspect}"
    end
  end

  RSpec::Matchers.define :have_led_ring_animation_sequence do |expected_sequence|
    match do |front_panel|
      front_panel.has_led_ring_animation_sequence?(expected_sequence)
    end

    failure_message do |front_panel|
      "expected LED ring animation sequence to equal #{expected_sequence}, "\
      "actual sequence is #{front_panel.led_ring_animation_sequence}"
    end
  end

end
