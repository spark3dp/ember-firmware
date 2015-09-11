#  File: wireless_network.rb
#  Model for a wireless network
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#  
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'ostruct'
require 'json'

module Smith
  module Config
    class InvalidNetworkConfiguration < StandardError; end
    
    class WirelessNetwork < OpenStruct

      class << self
        def last_configured
          if params = State.load.last_configured_wireless_network
            new(params)
          end
        end
      end

      def initialize(hsh)
        # Ensure all keys are symbols
        hsh = hsh.each_with_object({}){ |(k, v), h| h[k.to_sym] = v }
        
        hsh.update(hsh) do |k, v|
          case k
          when :passphrase
            System.wpa_psk(hsh.fetch(:ssid) { raise(InvalidNetworkConfiguration, 'ssid is missing') }, v)
          when :password
            System.nt_hash(v)
          else
            v
          end
        end

        super(hsh)
      end

      def save_as_last_configured
        State.load.update(last_configured_wireless_network: marshal_dump.delete_if { |k, v| [:passphrase, :password, :key].include?(k) })
      end

      def try(key)
        send(key)
      rescue InvalidNetworkConfiguration
        nil
      end

      def method_missing(meth, *args)
        raise(InvalidNetworkConfiguration, "#{meth} is missing") unless meth.to_s.end_with?('=')
        super
      end

      def encrypted?
        security == 'none' ? false : true
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
        Config.get_template(file_name)
      end

      def get_binding
        binding
      end

    end
  end
end
