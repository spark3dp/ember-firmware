require 'ipaddress'

module Smith
  module Config
    class ApModeConfig

      LAST_OCTET_REGEX = /\d{1,3}\z/

      attr_reader :ssid, :interface_name

      def initialize(cidr_ip_address, ssid, interface_name)
        @ip_address = ::IPAddress.parse(cidr_ip_address)
        @ssid, @interface_name = ssid, interface_name
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

    end
  end
end
