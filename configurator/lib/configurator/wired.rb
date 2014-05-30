module Configurator
  module Wired
    extend System

    module_function

    def connected?
      link_beat?(interface)
    end

    def interface
      Configurator.wired_interface
    end
  end
end
