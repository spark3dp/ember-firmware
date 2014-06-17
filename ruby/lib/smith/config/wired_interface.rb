module Smith
  module Config
    module WiredInterface
      extend System

      module_function

      def connected?
        link_beat?(name)
      end

      def name
        ENV['WIRED_INTERFACE']
      end

    end
  end
end
