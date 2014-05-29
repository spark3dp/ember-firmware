require 'configurator/version'

ENV['WPA_ROAM_PATH'] ||= '/etc/wpa_supplicant'
ENV['WIRELESS_INTERFACE'] ||= 'ra0'

module Configurator
  def self.root
    @root ||= File.expand_path('../..', __FILE__)
  end

  def self.template_path
    File.join(root, 'lib/configurator/templates')
  end

  def self.wpa_roam_file
    File.join(ENV['WPA_ROAM_PATH'], 'wpa-roam.conf')
  end

  def self.wireless_interface
    ENV['WIRELESS_INTERFACE']
  end
end

Dir["#{Configurator.root}/lib/**/*.rb"].each { |f| require(f) }
