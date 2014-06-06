require_relative 'wireless_helper'
require_relative 'app_helper'

module Smith
  module ConfigApp
    App.helpers WirelessHelper
    App.helpers AppHelper
  end
end
