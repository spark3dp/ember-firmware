require 'client_helper'
require 'aws-sdk'

module Smith
  describe 'Printer web client when started when registered', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include ClientCommandSteps

    before { allow_primary_registration }

    context' when printer control command is received' do

      it 'forwards command to command pipe' do
        assert_command_forwarded_to_command_pipe(CMD_PAUSE)
      end

    end

    context 'when logs command is received', :vcr do

      let(:s3) { AWS::S3.new(access_key_id: aws_access_key_id, secret_access_key: aws_secret_access_key) }
      let(:bucket) { s3.buckets[bucket_name] }
      # Use a test specific S3 bucket
      let(:bucket_name) { 'test-36gw0r' }

      before do
        # Set settings so client uses test credentials and bucket
        set_settings_async(s3_log_bucket: bucket_name, aws_access_key_id: aws_access_key_id, aws_secret_access_key: aws_secret_access_key)
      end
      
      context 'when successful upload to S3 is possible' do

        # These AWS credentials are for an "admin" user that has full access
        let(:aws_access_key_id) { 'AKIAJRZDNGJDNBKRLLRA' }
        let(:aws_secret_access_key) { '5zNYnoarsc/8MxHujMxzOw+n7tfpE68BW8HPqvGs' }

        it 'uploads log archive and responds to server with location' do |example|
          assert_logs_command_handled_when_logs_command_received
        end

      end

      context 'when successful upload to S3 is not possible' do
        
        # Invalid credentials
        let(:aws_access_key_id) { 'abc' }
        let(:aws_secret_access_key) { '123' }

        it 'logs error' do
          assert_error_log_entry_written_when_log_upload_fails
        end

      end

    end

    context 'when print_data command is received' do
      before { create_print_data_dir_async }
      
      context 'when printer is in home state and substate does not indicate failure after issuing data load command' do
        let(:stray_print_file) { File.join(print_data_dir, 'old.tar.gz') }

        it 'downloads file from specified url to download directory, saves settings to file, and sends commands to process data and load settings' do
          # Prepare get status command responses for first two commands
          write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)
          write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)

          touch_stray_print_file
          assert_print_data_command_handled_when_print_data_command_received_when_print_data_load_succeeds
          assert_print_data_dir_purged_before_print_file_download
        end

      end

      context 'when printer is in home state and substate does indicate failure after issuing data load command' do

        it 'logs error' do
          # Prepare get status command responses for first two commands
          write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)
          write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)

          assert_error_log_entry_written_when_print_data_command_received_when_print_data_load_fails
        end

      end

      context 'when printer is not in home state' do

        it 'logs error and does not download print data file' do
          # Prepare get status command response indicating that printer is not in home state
          write_get_status_command_response_async(state: Smith::PRINTING_STATE, substate: Smith::NO_SUBSTATE)
  
          assert_error_log_entry_written_and_data_not_downloaded_when_print_data_command_received
        end

      end

    end
  end
end
