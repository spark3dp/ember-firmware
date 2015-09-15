#  File: status_update_spec.rb
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

require 'client_helper'

module Smith
  module Client
    describe 'Printer web client when status update is written to status pipe', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include ClientStatusUpdateSteps
      include PrintEngineHelper

      before { allow_primary_registration }

      it 'makes request to server with contents of status update' do
        assert_status_update_request_made_when_status_written_to_status_pipe
      end

    end
  end
end
