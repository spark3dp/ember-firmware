module Smith
  module Config
    module WiredInterface
      extend System

      module_function

      def connected?
        link_beat?(name)
      end

      def name
        Smith::Settings.wired_interface
      end

    end
  end
end
