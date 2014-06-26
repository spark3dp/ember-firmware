require 'spec_helper'

module Smith
  describe 'domain redirect' do
=begin
    scenario 'non-canonical domain redirects to canonical domain' do
      original_host = Capybara.default_host
      
      Capybara.default_host = 'http://other.com'
      
      visit '/'

      expect(current_host).to eq original_host

      Capybara.default_host = original_host
    end
=end
  end
end
