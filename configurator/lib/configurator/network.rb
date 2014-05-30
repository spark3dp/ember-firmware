require 'yaml'
require 'erb'

module Configurator
  module Network
    module_function

    def configure(options)
      config = ConfigOptions.new(options)
      File.write(Configurator.wpa_roam_file, ERB.new(config.get_template).result(config.get_binding))
      Wireless.enable_managed_mode
      Wireless.disconnect if Wired.connected?
    end

    def configure_from_file(file_name)
      raise(RuntimeError, 'must specify config file') if file_name.nil?
      configure(YAML.load_file(file_name))
    end
  end
end
