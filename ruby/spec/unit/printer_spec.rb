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
            set_printer_status(state: HOME_STATE, substate: NO_SUBSTATE)
            
            expect(subject.get_status).to eq(printer_status(state: HOME_STATE, substate: NO_SUBSTATE))
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

    context 'when validating that printer is not in downloading or loading substates' do

      include_context 'print engine ready'
      
      context 'when printer is in Home state' do

        context 'when printer is in Downloading substate' do
          it 'raises invalid state error' do
            set_printer_status(state: HOME_STATE, substate: DOWNLOADING_PRINT_DATA_SUBSTATE)
            expect { subject.validate_not_in_downloading_or_loading }.to raise_error(Printer::InvalidState)
          end
        end

        context 'when printer is in Loading substate' do
          it 'raises invalid state error' do
            set_printer_status(state: HOME_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)
            expect { subject.validate_not_in_downloading_or_loading }.to raise_error(Printer::InvalidState)
          end
        end

        context 'when printer is not in Loading or Downloading substates' do
          it 'does not raise invalid state error' do
            set_printer_status(state: HOME_STATE, substate: NO_SUBSTATE)
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
