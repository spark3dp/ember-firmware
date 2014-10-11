require 'smith'
require 'smith/config/system'

Dir["#{Smith.root}/lib/smith/config/**/*.rb"].each { |f| require(f) }

module Smith
  module Config
    module_function

    def get_template(file_name)
      File.read(File.join(Smith.root, 'lib/smith/config/templates', file_name))
    end

    def available_wireless_networks
      Network.available_wireless_networks
    end

  end
end
