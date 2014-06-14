require 'ipaddress'

module Smith
  module Config
    class IPAddress

      LAST_OCTET_REGEX = /\d{1,3}\z/

      def initialize(ip_address)
        @ip_address = ::IPAddress.parse(ip_address)
      end

      def subnet_mask
        @ip_address.netmask
      end

      def broadcast_address
        @ip_address.broadcast.to_s
      end

      def network_address
        @ip_address.network.to_s
      end

      def range_start
        @ip_address.to_s.sub(LAST_OCTET_REGEX, '5')
      end

      def range_end
        @ip_address.to_s.sub(LAST_OCTET_REGEX, '150')
      end

      def address
        @ip_address.to_s
      end

      def get_binding
        binding
      end
      
    end
  end
end
