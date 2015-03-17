require 'common_helper'
require 'smith/client/payload_helper'
require 'support/print_engine_helper'

module Smith
  module Client
    describe PayloadHelper do

      include PrintEngineHelper
      include PayloadHelper

      let(:status) { printer_status({ spark_state: 'ready', error_code: 0, error_message: 'no error' }.merge(status_values)) }
      let(:payload) { status_payload(status) }

      context 'when Spark job status is empty' do
        let(:status_values) { { spark_job_state: '' } }
        it 'provides only status not specific to a job' do
          expect(payload[:printer_status]).to eq('ready')
          expect(payload[:error_code]).to eq(0)
          expect(payload[:error_message]).to eq('no error')
          expect(payload[:data]).to eq(status)
          expect(payload).not_to have_key(:job_id)
          expect(payload).not_to have_key(:job_status)
          expect(payload).not_to have_key(:job_progress)
        end
      end
      context 'when Spark job status is not empty' do
        let(:status_values) { { spark_job_state: 'received',  job_id: '', total_layers: 0} }
        it 'provides status not specific to a job' do
          expect(payload[:printer_status]).to eq('ready')
          expect(payload[:error_code]).to eq(0)
          expect(payload[:error_message]).to eq('no error')
          expect(payload[:data]).to eq(status)
        end
        it 'provides job-specific status' do
          expect(payload[:job_status]).to eq('received')
        end
        context 'when printer status job ID is empty' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '', total_layers: 0} }
          it 'provides job ID "local"' do
            expect(payload[:job_id]).to eq('local')
          end
        end
        context 'when printer status job ID is not empty' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 0 } }
          it 'provides job ID from printer status' do
            expect(payload[:job_id]).to eq('123')
          end
        end
        context 'when total layers is 0' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 0 } }
          it 'provides job progress of 0.0' do
            expect(payload[:job_progress]).to eq(0.0)
          end
        end
        context 'when total layers is non-zero' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 100, layer: 25 } }
          it 'provides job progress of layer/total layers' do
            expect(payload[:job_progress]).to eq(0.25)
          end
        end
      end

    end
  end
end
