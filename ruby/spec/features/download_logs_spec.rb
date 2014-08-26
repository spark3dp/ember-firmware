require 'spec_helper'

module Smith
  describe 'Download logs', :tmp_dir do

    let(:log_file) { tmp_dir 'log_file' }

    scenario 'configured log file is download' do
      ENV['LOG_FILE'] = log_file
      File.write(log_file, 'log file contents')

      visit '/'
      click_link 'Download logs'

      expect(page.response_headers['Content-Disposition']).to eq(%Q(attachment; filename="smith-syslog-#{Time.now.strftime('%m-%d-%Y')}"))
    end
  end
end
