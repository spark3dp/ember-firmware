module Configurator
  module Wired
    module_function

    def link_beat?
      System.link_beat?(Configurator.wired_interface)
    end
  end
end
