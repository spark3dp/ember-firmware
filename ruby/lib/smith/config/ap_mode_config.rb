require 'ipaddress'

module Smith
  module Config
    class ApModeConfig

      LAST_OCTET_REGEX = /\d{1,3}\z/

      attr_reader :interface_name

      def initialize(cidr_ip_address, ssid_prefix, interface_name)
        @ip_address = ::IPAddress.parse(cidr_ip_address)
        @ssid_prefix, @interface_name = ssid_prefix, interface_name
      end

      def dhcp_range_start
        @ip_address.to_s.sub(LAST_OCTET_REGEX, '5')
      end

      def dhcp_range_end
        @ip_address.to_s.sub(LAST_OCTET_REGEX, '150')
      end

      def ip_address
        @ip_address.to_s
      end

      def get_binding
        binding
      end

      def ssid
        if File.exists?(Config.ssid_suffix_file)
          ssid_suffix = File.read(Config.ssid_suffix_file)
        else
          ssid_suffix = (('A'..'Z').to_a + (2..9).to_a - ['O','I']).shuffle[0, 6].join
          File.write(Config.ssid_suffix_file, ssid_suffix)
        end
        
        "#{@ssid_prefix} #{ssid_suffix}"
      end

    end
  end
end
