#  File: print_data_command_spec.rb
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
    describe 'Printer web client when print_data command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include PrintDataCommandSteps

      let(:print_settings) { { SETTINGS_ROOT_KEY => { 'JobName' => 'my print' } } }
      let(:test_job_id) { '78910' }
      let(:final_print_settings) { { SETTINGS_ROOT_KEY => { 'JobName' => 'my print', JOB_ID_SETTING => test_job_id } } }

      # The downloaded print file is named according to the URL it was downloaded from
      let(:test_print_file) { dummy_server.test_print_file_url.split('/').last }

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
        create_print_data_dir_async
      end
      
      context 'when print file specified by command is not the same as the print file currently loaded' do
        
        before { set_smith_settings_async(PRINT_FILE_SETTING => '') }

        context 'when printer is in valid state before and after downloading' do
        
          let(:stray_print_file) { File.join(print_data_dir, 'old.tar.gz') }
          
          it 'downloads file from specified url to download directory, saves settings to file, and sends commands to process data and load settings' do
            set_printer_status_async(test_printer_status_values)

            touch_stray_print_file
            assert_print_data_command_handled_when_print_data_command_received_when_file_not_already_loaded_when_print_data_load_succeeds
            assert_print_data_dir_purged_before_print_file_download
          end

        end

        context 'when printer is in valid state before downloading but not in valid state after download is complete' do

          it 'acknowledges error' do
            set_printer_status_async(test_printer_status_values)

            assert_error_acknowledgement_sent_when_print_data_command_received_when_printer_not_in_valid_state_after_download
          end

        end

        context 'when printer is not in valid state before downloading' do

          it 'acknowledges error and does not download print data file' do
            set_printer_status_async(state: PRINTING_STATE, ui_sub_state: NO_SUBSTATE, spark_state: 'printing', error_code: 0, error_message: 'no error',
                                     spark_job_state: 'printing', job_id: '', layer: 25, total_layers: 100, spark_local_job_uuid: 'abcdef' )

            assert_error_acknowledgement_sent_when_print_data_command_received
            assert_print_file_not_downloaded
          end

        end

        context 'when print file download fails' do

          it 'acknowledges error and sends show downloading failed command' do
            set_printer_status_async(test_printer_status_values)

            assert_error_acknowledgement_sent_when_print_data_command_received_when_download_fails
          end

        end

      end

      context 'when print file specified by command is the same as the print file currently loaded' do

        let(:print_file_name) { 'test_print_file_already_downloaded' }

        before { set_smith_settings_async(PRINT_FILE_SETTING => print_file_name) }

        context 'when printer is in valid state' do

          it 'saves settings to file and sends commands to apply settings and indicate that print file is loaded' do
            set_printer_status_async(test_printer_status_values)

            assert_print_data_command_handled_when_print_data_command_received_when_file_already_loaded_when_load_settings_succeeds
          end

        end

        context 'when printer is not in valid state' do

          it 'acknowledges error' do
            set_printer_status_async(state: CALIBRATING_STATE, spark_state: 'maintenance', error_code: 0, error_message: 'no error', spark_job_state: '')

            assert_print_data_command_handled_when_print_data_command_received_when_file_already_loaded_when_printer_not_in_valid_state
          end

        end

      end

    end
  end
end
