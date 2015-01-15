require 'server_helper'

module Smith
  describe 'Download logs', :tmp_dir do

    scenario 'log archive is downloaded' do
      FileUtils.touch(tmp_dir 'syslog')
      FileUtils.touch(tmp_dir 'smith-server.log')
      allow(Printer).to receive(:serial_number).and_return('abc123')

      visit '/'
      click_link 'Download logs'

      expect(page.response_headers['Content-Disposition']).to eq("attachment;filename=smith-#{VERSION}_logs_#{Time.now.strftime('%m-%d-%Y')}_abc123.tar.gz")
    end
  end
end
