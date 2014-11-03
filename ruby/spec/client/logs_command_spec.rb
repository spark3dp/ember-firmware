require 'client_helper'
require 'aws-sdk-core'

module Smith
  describe 'Printer web client when logs command is received', :client, :vcr do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include LogsCommandSteps

    before { allow_primary_registration }

    let(:s3) { Aws::S3::Client.new(region: Settings.aws_region, access_key_id: aws_access_key_id, secret_access_key: aws_secret_access_key) }
    
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

      it 'acknowledges error' do
        assert_error_acknowledgement_sent_when_log_command_handling_fails_fails
      end

    end

  end
end
