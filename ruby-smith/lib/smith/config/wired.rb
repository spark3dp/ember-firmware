module Smith
  module Config
    module Wired
      extend System

      module_function

      def connected?
        link_beat?(interface)
      end

      def interface
        Config.wired_interface
      end
    end
  end
end
