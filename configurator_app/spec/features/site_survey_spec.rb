require 'spec_helper'
require 'ostruct'

describe 'Wireless network site survey' do
  scenario 'no wireless networks in range' do
    allow(Configurator).to receive(:available_networks).and_return([])

    visit '/wireless'

    expect(page).to have_content('No networks in range')
  end

  scenario 'wireless networks in range' do
    allow(Configurator).to receive(:available_networks).and_return(
      [OpenStruct.new(ssid: 'wifi network', security: 'none', type: 'infrastructure')]
    )

    visit '/wireless'

    expect(page).to have_content('wifi network')
    expect(page).to have_content('none')
    expect(page).to have_content('infrastructure')
  end
end