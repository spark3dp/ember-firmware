#  File: download_logs_spec.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
