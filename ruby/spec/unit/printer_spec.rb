require 'common_helper'

module Smith
  describe Printer, :tmp_dir do
    include PrintEngineHelper

    shared_context 'print engine ready' do
      before do
        create_command_pipe
        create_command_response_pipe
        open_command_pipe
        open_command_response_pipe
      end

      after do
        close_command_pipe
        close_command_response_pipe
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

        context 'when command pipe is not open' do

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
      
      before { create_command_pipe }

      context 'when command response pipe exists' do

        before do
          create_command_response_pipe
          open_command_pipe
        end

        after { close_command_pipe }

        context 'when command response pipe is open' do

          before { open_command_response_pipe }
          after { close_command_response_pipe }

          context 'when command response is received' do

            it 'returns parsed response' do
              write_get_status_command_response(state: HOME_STATE, substate: NO_SUBSTATE)

              expect(subject.get_status).to eq(printer_status(state: HOME_STATE, substate: NO_SUBSTATE))
            end

          end

          context 'when command response is not received' do

            it 'raises communication error' do
              expect { subject.get_status }.to raise_error(Printer::CommunicationError)
            end

          end

        end

        context 'when command pipe is not open' do

          it 'raises communication error' do
            expect { subject.get_status }.to raise_error(Printer::CommunicationError)
          end

        end

      end

      context 'when command response pipe does not exist' do

        it 'raises communication error' do
          expect { subject.get_status }.to raise_error(Printer::CommunicationError)
        end

      end

    end

    context 'when showing loading print data' do

      include_context 'print engine ready'
      
      context 'when printer is in Home state and not in Downloading substate' do

        it 'sends print data load command' do
          write_get_status_command_response(state: HOME_STATE, substate: NO_SUBSTATE)
          
          subject.show_loading

          expect(next_command_in_command_pipe).to eq(CMD_GET_STATUS)
          expect(next_command_in_command_pipe).to eq(CMD_START_PRINT_DATA_LOAD)
        end

      end

      context 'when printer is in Home state and in Downloading substate' do

        it 'raises invalid state error' do
          write_get_status_command_response(state: HOME_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)

          expect { subject.show_loading }.to raise_error(Printer::InvalidState)
        end

      end

      context 'when printer is not in Home state' do

        it 'raises invalid state error' do
          write_get_status_command_response(state: PRINTING_STATE, substate: NO_SUBSTATE)
          
          expect { subject.show_loading }.to raise_error(Printer::InvalidState)
        end

      end

    end

    context 'when processing print data' do

      include_context 'print engine ready'

      context 'when printer is in Home state and in Downloading substate' do

        it 'sends process print data command' do
          write_get_status_command_response(state: HOME_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)

          subject.process_print_data

          expect(next_command_in_command_pipe).to eq(CMD_GET_STATUS)
          expect(next_command_in_command_pipe).to eq(CMD_PROCESS_PRINT_DATA)
        end

      end

      context 'when printer is in Home state and not in Downloading substate' do

        it 'raises invalid state error' do
          write_get_status_command_response(state: HOME_STATE, substate: NO_SUBSTATE)

          expect { subject.process_print_data }.to raise_error(Printer::InvalidState)
        end

      end

      context 'when printer is not in Home state' do

        it 'raises invalid state error' do
          write_get_status_command_response(state: PRINTING_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)
          
          expect { subject.process_print_data }.to raise_error(Printer::InvalidState)
        end

      end

    end

    context 'when showing loaded print data' do
      
      include_context 'print engine ready'

      context 'when printer is in Home state' do
        it 'sends show loaded command' do
          write_get_status_command_response(state: HOME_STATE, substate: NO_SUBSTATE)

          subject.show_loaded

          expect(next_command_in_command_pipe).to eq(CMD_GET_STATUS)
          expect(next_command_in_command_pipe).to eq(CMD_SHOW_PRINT_DATA_LOADED)
        end
      end

      context 'when printer is not in Home state' do
        it 'raises invalid state error' do
          write_get_status_command_response(state: ERROR_STATE, substate: NO_SUBSTATE)
          
          expect { subject.show_loaded }.to raise_error(Printer::InvalidState)
        end
      end
      
    end

    context 'when applying print settings file' do
      
      include_context 'print engine ready'

      it 'sends apply print settings command' do
        subject.apply_print_settings_file

        expect(next_command_in_command_pipe).to eq(CMD_APPLY_PRINT_SETTINGS)
      end

    end

  end
end
