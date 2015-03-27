require 'common_helper'
require 'smith/client/payload_helper'
require 'support/print_engine_helper'
require 'smith/client/command'

module Smith
  module Client
    describe PayloadHelper do

      include PrintEngineHelper
      include PayloadHelper

      let(:status) { printer_status({ spark_state: 'ready', error_code: 0, error_message: 'no error', spark_local_job_uuid: 'abcdef12-3456' }.merge(status_values)) }
      let(:payload) { status_payload(status) }
      let(:cmd_payload) { command_payload(cmd_values[:command], cmd_values[:command_state], cmd_values[:message], status) }

      context 'when Spark job status is empty' do
        let(:status_values) { { spark_job_state: '' } }
        let(:cmd_values) { { command: 'Calibrate', command_state: Command::RECEIVED_ACK, message: 'none' } }

        it 'provides only status not specific to a job' do
          expect(payload[:printer_status]).to eq('ready')
          expect(payload[:error_code]).to eq(0)
          expect(payload[:error_message]).to eq('no error')
          status.delete(SPARK_STATE_PS_KEY)
          status.delete(SPARK_JOB_STATE_PS_KEY)
          expect(payload[:data]).to eq(status)
          expect(payload).not_to have_key(:job_id)
          expect(payload).not_to have_key(:job_status)
          expect(payload).not_to have_key(:job_progress)
        end
        it 'provides only command acknowledgement not specific to a job' do
          expect(cmd_payload[:printer_status]).to eq('ready')
          expect(cmd_payload[:progress]).to eq(0)
          expect(cmd_payload[:error_code]).to eq(0)
          expect(cmd_payload[:error_message]).to eq('none')
          expect(cmd_payload[:data][:command]).to eq('Calibrate')
          expect(cmd_payload[:data][:message]).to eq('none')
          expect(cmd_payload[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(payload).not_to have_key(:job_id)
          expect(payload).not_to have_key(:job_status)
          expect(payload).not_to have_key(:job_progress)
        end
      end
      context 'when Spark job status is not empty' do
        let(:status_values) { { spark_job_state: 'received',  job_id: '', total_layers: 0} }
        let(:cmd_values) { { command: 'Calibrate', command_state: Command::RECEIVED_ACK, message: 'none' } }

        it 'provides status not specific to a job' do
          expect(payload[:printer_status]).to eq('ready')
          expect(payload[:error_code]).to eq(0)
          expect(payload[:error_message]).to eq('no error')
          status.delete(SPARK_STATE_PS_KEY)
          status.delete(SPARK_JOB_STATE_PS_KEY)
          expect(payload[:data]).to eq(status)
        end
        it 'provides command acknowledgement not specific to a job' do
          expect(cmd_payload[:printer_status]).to eq('ready')
          expect(cmd_payload[:progress]).to eq(0)
          expect(cmd_payload[:error_code]).to eq(0)
          expect(cmd_payload[:error_message]).to eq('none')
          expect(cmd_payload[:data][:command]).to eq('Calibrate')
          expect(cmd_payload[:data][:message]).to eq('none')
          expect(cmd_payload[:data][:state]).to eq(Command::RECEIVED_ACK)
        end
        it 'provides job-specific status' do
          expect(payload[:job_status]).to eq('received')
        end
        it 'provides job-specific command acknowledgement' do
          expect(cmd_payload[:job_status]).to eq('received')
        end
        context 'when printer status job ID is empty' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '', total_layers: 0 } }
          it 'provides status job ID "local_<uuid>"' do
            expect(payload[:job_id]).to eq('local_abcdef12-3456')
          end
          it 'provides command acknowledgement job ID "local_<uuid>"' do
            expect(cmd_payload[:job_id]).to eq('local_abcdef12-3456')
          end
        end
        context 'when printer status job ID is not empty' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 0 } }
          it 'provides status job ID from printer status' do
            expect(payload[:job_id]).to eq('123')
          end
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 0 } }
          it 'provides command acknowledgement job ID from printer status' do
            expect(cmd_payload[:job_id]).to eq('123')
          end
        end
        context 'when total layers is 0' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 0 } }
          it 'provides status job progress of 0.0' do
            expect(payload[:job_progress]).to eq(0.0)
          end
          it 'provides command acknowledgement job progress of 0.0' do
            expect(cmd_payload[:job_progress]).to eq(0.0)
          end
        end
        context 'when total layers is non-zero' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 100, layer: 25 } }
          it 'provides status job progress of layer/total layers' do
            expect(payload[:job_progress]).to eq(0.25)
          end
          it 'provides command acknowledgement job progress of layer/total layers' do
            expect(cmd_payload[:job_progress]).to eq(0.25)
          end
        end
      end

      context 'when command state is completed' do
        let(:status_values) { { spark_job_state: '' } }
        let(:cmd_values) { { command: 'Calibrate', command_state: Command::COMPLETED_ACK, message: 'none' } }
        it 'provides command acknowledgement with progress of 1 and error code of 0' do
          expect(cmd_payload[:progress]).to eq(1)
          expect(cmd_payload[:error_code]).to eq(0)
          expect(cmd_payload[:error_message]).to eq('none')
          expect(cmd_payload[:data][:state]).to eq(Command::COMPLETED_ACK)
        end
      end

      context 'when command state is failed' do
        let(:status_values) { { spark_job_state: '' } }
        let(:cmd_values) { { command: 'Calibrate', command_state: Command::FAILED_ACK, message: 'none' } }
        it 'provides command acknowledgement with progress of 1 and error code of 5000' do
          expect(cmd_payload[:progress]).to eq(1)
          expect(cmd_payload[:error_code]).to eq(500)
          expect(cmd_payload[:error_message]).to eq('none')
          expect(cmd_payload[:data][:state]).to eq(Command::FAILED_ACK)
        end
      end



    end
  end
end
