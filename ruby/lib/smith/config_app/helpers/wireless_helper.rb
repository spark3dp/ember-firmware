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
	security
      end
    end
  end
end
