RSpec::Matchers.define :contain_ssid do |ssid|
  match do |wpa_roam_file|
    wpa_roam_file.include?(%Q(ssid="#{ssid}"))
  end
end

RSpec::Matchers.define :contain_no_security do
  match do |wpa_roam_file|
    wpa_roam_file.include?('key_mgmt=NONE')
  end
end

RSpec::Matchers.define :contain_psk do |psk|
  match do |wpa_roam_file|
    wpa_roam_file.include?(%Q(psk="#{psk}"))
  end
end

RSpec::Matchers.define :contain_eap_credentials do |username, password, domain|
  match do |wpa_roam_file|
    wpa_roam_file.include?(%Q(identity="#{domain}\\#{username}")) && wpa_roam_file.include?(%Q(password="#{password}"))
  end
end

RSpec::Matchers.define :contain_wep_key do |key|
  match do |wpa_roam_file|
    wpa_roam_file.include?('key_mgmt=NONE') && wpa_roam_file.include?('wep_tx_keyidx=0') && wpa_roam_file.include?(%Q(wep_key0=#{key}))
  end
end
