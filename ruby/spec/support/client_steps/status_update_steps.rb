#  File: status_update_steps.rb
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

module Smith
  module Client
    ClientStatusUpdateSteps = RSpec::EM.async_steps do
     
      def assert_status_update_request_made_when_status_written_to_status_pipe(&callback)
        add_http_request_expectation status_endpoint do |request_params|
          expect(request_params.to_json).to eq(test_status_payload.to_json)
          callback.call
        end

        # Simulate status update from smith
        File.write(status_pipe, "#{test_printer_status.to_json}\n")
      end

    end
  end
end
