#  File: wpa_roam_file_matchers.rb
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

RSpec::Matchers.define :include_ssid do |ssid|
  match do |wpa_roam_file|
    wpa_roam_file.include?(%Q(ssid="#{ssid}"))
  end
end

RSpec::Matchers.define :include_no_security do
  match do |wpa_roam_file|
    wpa_roam_file.include?('key_mgmt=NONE')
  end
end

RSpec::Matchers.define :include_psk do |psk|
  match do |wpa_roam_file|
    wpa_roam_file.include?(%Q(psk=#{psk}))
  end
end

RSpec::Matchers.define :include_eap_credentials do |username, password, domain|
  match do |wpa_roam_file|
    wpa_roam_file.include?(%Q(identity="#{domain}\\#{username}")) && wpa_roam_file.include?(%Q(password=hash:#{password}))
  end
end

RSpec::Matchers.define :include_wep_key do |key|
  match do |wpa_roam_file|
    wpa_roam_file.include?('key_mgmt=NONE') && wpa_roam_file.include?('wep_tx_keyidx=0') && wpa_roam_file.include?(%Q(wep_key0=#{key}))
  end
end
