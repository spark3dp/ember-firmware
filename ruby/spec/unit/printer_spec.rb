#  File: printer_spec.rb
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

require 'common_helper'

module Smith
  describe Printer, :tmp_dir do
    include PrintEngineHelper

    shared_context 'print engine ready' do
      before do
        create_command_pipe
        create_printer_status_file
        open_command_pipe
      end

      after do
        close_command_pipe
      end
    end

    context 'when sending command' do

      context 'when command pipe exists' do

        before { create_command_pipe }

        context 'when command pipe is open' do

          before { open_command_pipe }
          after { close_command_pipe }

          it 'writes command to command pipe' do
            subject.send_command(CMD_PAUSE)
            expect(next_command_in_command_pipe).to eq(CMD_PAUSE)
          end

        end

        context 'when command pipe is not open for reading' do

          it 'raises communication error' do
            expect { subject.send_command(CMD_PAUSE) }.to raise_error(Printer::CommunicationError)
          end

        end

      end

      context 'when command pipe does not exist' do
        
        it 'raises communication error' do
          expect { subject.send_command(CMD_PAUSE) }.to raise_error(Printer::CommunicationError)
        end

      end

    end

    context 'when getting status' do
     
      context 'when status file exists' do 

        before { create_printer_status_file }

        context 'when status file is successfully read before timeout expires' do
          
          it 'returns parsed response' do
            set_printer_status(state: HOME_STATE, ui_sub_state: NO_SUBSTATE)
            
            expect(subject.get_status).to eq(printer_status(state: HOME_STATE, ui_sub_state: NO_SUBSTATE))
          end

        end

        context 'when status file is not successfully read before timeout expires' do

          it 'raises communication error' do
            # Printer status file is empty, causes a JSON parse error
            # This occurs if the file is read in the middle of a write
            expect { subject.get_status }.to raise_error(Printer::CommunicationError)
          end

        end

      end

      context 'when status file does not exist' do

        it 'raises communication error' do
          expect { subject.get_status }.to raise_error(Printer::CommunicationError)
        end

      end

    end

    context 'when validating that printer is not in downloading or loading ui_sub_states' do

      include_context 'print engine ready'
      
      context 'when printer is in Home state' do

        context 'when printer is in Downloading ui_sub_state' do
          it 'raises invalid state error' do
            set_printer_status(state: HOME_STATE, ui_sub_state: DOWNLOADING_PRINT_DATA_SUBSTATE)
            expect { subject.validate_not_in_downloading_or_loading }.to raise_error(Printer::InvalidState)
          end
        end

        context 'when printer is in Loading ui_sub_state' do
          it 'raises invalid state error' do
            set_printer_status(state: HOME_STATE, ui_sub_state: LOADING_PRINT_DATA_SUBSTATE)
            expect { subject.validate_not_in_downloading_or_loading }.to raise_error(Printer::InvalidState)
          end
        end

        context 'when printer is not in Loading or Downloading ui_sub_states' do
          it 'does not raise invalid state error' do
            set_printer_status(state: HOME_STATE, ui_sub_state: NO_SUBSTATE)
            expect { subject.validate_not_in_downloading_or_loading }.not_to raise_error
          end
        end

      end

      context 'when printer is not in Home state' do

        it 'raises invalid state error' do
          set_printer_status(state: PRINTING_STATE)
          expect { subject.validate_not_in_downloading_or_loading }.to raise_error(Printer::InvalidState)
        end

      end

    end

  end
end
