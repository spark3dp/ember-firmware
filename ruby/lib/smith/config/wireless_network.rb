require 'ostruct'

module Smith
  module Config
    class InvalidNetworkConfiguration < StandardError; end
    
    class WirelessNetwork < OpenStruct

      def method_missing(meth, *args)
        raise(InvalidNetworkConfiguration, "#{meth} is missing") unless meth.to_s.end_with?('=')
        super
      end

      def encrypted?
        encryption == 'on' ? true : false
      end

      def wpa_roam_template
        file_name =
          case security
          when 'none'           then 'wpa-roam_unsecured.conf.erb'
          when 'wpa-personal'   then 'wpa-roam_wpa_personal.conf.erb'
          when 'wpa-enterprise' then 'wpa-roam_wpa_enterprise.conf.erb'
          when 'wep'            then 'wpa-roam_wep.conf.erb'
          else raise(InvalidNetworkConfiguration, "#{security} is not a valid security type")
          end
        File.read(File.join(Config.template_path, file_name))
      end

      def get_binding
        binding
      end

    end
  end
end
