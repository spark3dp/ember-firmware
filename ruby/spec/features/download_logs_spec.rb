require 'spec_helper'

module Smith
  describe 'Download logs', :tmp_dir do

    scenario 'log archive is downloaded' do
      FileUtils.touch(tmp_dir 'syslog')
      FileUtils.touch(tmp_dir 'smith-server.log')

      visit '/'
      click_link 'Download logs'

      expect(page.response_headers['Content-Disposition']).to eq("attachment;filename=smith-#{VERSION}_logs_#{Time.now.strftime('%m-%d-%Y')}.tar.gz")
    end
  end
end
