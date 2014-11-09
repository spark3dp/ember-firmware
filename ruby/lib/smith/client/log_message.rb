# Formates log message strings given an ERB template and parameters

require 'erb'

module Smith
  module Client
    class LogMessage

      class << self
        def format(*args)
          new(*args).format
        end
      end

      attr_reader :args

      def initialize(template, *args)
        @template, @args = template, args
      end

      # Format an exception in a manner similar to how ruby formats them
      def format_exception(e)
        "#{e.backtrace.first}: #{e.message} (#{e.class})\n#{e.backtrace.drop(1).map { |l| "\t#{l}" }.join("\n")}"
      end

      def format
        ERB.new(@template).result(binding)
      end

    end
  end
end
