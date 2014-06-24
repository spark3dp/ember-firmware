require_relative 'wireless_helper'
require_relative 'app_helper'

module Smith
  module App
    Application.helpers WirelessHelper
    Application.helpers AppHelper
  end
end
