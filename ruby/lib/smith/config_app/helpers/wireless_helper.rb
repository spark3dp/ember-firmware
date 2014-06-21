module Smith
  module ConfigApp
    module WirelessHelper

      def format_mode(mode)
        if mode == 'Master'
          'Infrastructure'
        else
          mode
        end
      end
      
      def format_security(security)
        case security
        when 'wpa-personal'
          'WPA Personal (PSK)'
        when 'wpa-enterprise'
          'WPA Enterprise (EAP)'
        when 'none'
          'None'
        else
          security.upcase
        end
      end

    end
  end
end
