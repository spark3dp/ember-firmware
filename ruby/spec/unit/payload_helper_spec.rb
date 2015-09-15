#  File: payload_helper_spec.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Richard Greene
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
      let(:cmd_payload) { command_payload(cmd_values[:command], cmd_values[:command_state], cmd_values[:message], status, cmd_values[:job_id]) }

      context 'when Spark job status is empty' do
        let(:status_values) { { spark_job_state: '' } }
        let(:cmd_values) { { command: 'Calibrate', command_state: Command::RECEIVED_ACK, message: 'none' } }

        it 'provides only status not specific to a job' do
          expect(payload[:printer_status]).to eq('ready')
          expect(payload[:error_code]).to eq(0)
          expect(payload[:error_message]).to eq('no error')
          status.delete(SPARK_STATE_PS_KEY)
          status.delete(SPARK_JOB_STATE_PS_KEY)
          status.delete(LOCAL_JOB_UUID_PS_KEY)
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
          expect(cmd_payload[:job_id]).to eq(nil)
          expect(cmd_payload[:job_status]).to eq(nil)
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
          status.delete(LOCAL_JOB_UUID_PS_KEY)
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
        context 'when total layers is 0 and not in PrintCompleted state' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 0, state: HOMING_STATE, ui_sub_state: NO_SUBSTATE } }
          it 'provides status job progress of 0.0' do
            expect(payload[:job_progress]).to eq(0.0)
          end
          it 'provides command acknowledgement job progress of 0.0' do
            expect(cmd_payload[:job_progress]).to eq(0.0)
          end
          it 'provides no settings' do
            expect(payload[:data][:settings]).to eq(nil)
          end
        end
        context 'when total layers is 0 and printer is in PrintCompleted state' do
          let(:status_values) { { spark_job_state: 'received',  job_id: '123', total_layers: 0, state: HOMING_STATE, ui_sub_state: PRINT_COMPLETED_SUBSTATE } }
          let(:smith_settings_file) { tmp_dir 'smith_settings_file' }
          before do
            Smith::Settings.smith_settings_file = smith_settings_file
            File.write(smith_settings_file, JSON.pretty_generate(Smith::SETTINGS_ROOT_KEY => { PRINT_FILE_SETTING => 'test.tar.gz' }))
          end
          it 'provides settings' do
            expect(payload[:data][:settings][PRINT_FILE_SETTING]).to eq('test.tar.gz')
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

      context 'when command job ID is present' do
        let(:status_values) { { spark_job_state: '' } }
        let(:cmd_values) { { command: 'print_data', command_state: Command::RECEIVED_ACK, message: 'none', job_id: 'xyz' } }

        it 'provides command acknowledgement specific to that job' do
          expect(cmd_payload[:printer_status]).to eq('ready')
          expect(cmd_payload[:progress]).to eq(0)
          expect(cmd_payload[:error_code]).to eq(0)
          expect(cmd_payload[:error_message]).to eq('none')
          expect(cmd_payload[:data][:command]).to eq('print_data')
          expect(cmd_payload[:data][:message]).to eq('none')
          expect(cmd_payload[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(cmd_payload[:job_id]).to eq('xyz')
          expect(cmd_payload[:job_status]).to eq('received')
          expect(payload).not_to have_key(:job_id)
          expect(payload).not_to have_key(:job_status)
          expect(payload).not_to have_key(:job_progress)
        end
      end
    end
  end
end
