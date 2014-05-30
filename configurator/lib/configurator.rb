require 'configurator/version'
require 'configurator/system'

ENV['WPA_ROAM_PATH'] ||= '/etc/wpa_supplicant'
ENV['WIRELESS_INTERFACE'] ||= 'ra0'
ENV['WIRED_INTERFACE'] ||= 'eth0'

module Configurator
  module_function

  def root
    @root ||= File.expand_path('../..', __FILE__)
  end

  def template_path
    File.join(root, 'lib/configurator/templates')
  end

  def wpa_roam_file
    File.join(ENV['WPA_ROAM_PATH'], 'wpa-roam.conf')
  end

  def wireless_interface
    ENV['WIRELESS_INTERFACE']
  end

  def wired_interface
    ENV['WIRED_INTERFACE']
  end
end

Dir["#{Configurator.root}/lib/**/*.rb"].each { |f| require(f) }
