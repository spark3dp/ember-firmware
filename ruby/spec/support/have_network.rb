RSpec::Matchers.define :have_network do |ssid, mode, security|
  match do |page|
    page.has_selector?('tr', text: /#{ssid}\s+#{mode}\s+#{security}/)
  end
end
