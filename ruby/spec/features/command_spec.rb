require 'server_helper'

module Smith
  describe 'command interface', :tmp_dir do
    include PrintEngineHelper
    include Rack::Test::Methods
   
    let(:response_body) { JSON.parse(last_response.body, symbolize_names: true) }

    context 'when communication via command pipe is possible' do

      before do
        create_command_pipe
        create_printer_status_file
        open_command_pipe
      end

      after do
        close_command_pipe
      end

      scenario 'receives command that does not return a response' do
        post '/command', command: CMD_CANCEL

        expect(next_command_in_command_pipe).to eq(CMD_CANCEL)
        expect(last_response.status).to eq(200)
        expect(response_body).to eq(command: CMD_CANCEL)
      end

      scenario 'receives a request without command parameter' do
        post '/command'

        expect(last_response.status).to eq(400)
        expect(response_body[:error]).to match(/command parameter is required/i)
      end

      scenario 'receives get status command' do
        status = { state: HOME_STATE, substate: NO_SUBSTATE }
        set_printer_status(status)

        post '/command', command: CMD_GET_STATUS

        expect(last_response.status).to eq(200)
        expect(response_body[:command]).to eq(CMD_GET_STATUS)
        expect(response_body[:response]).to eq(keys_to_symbols(printer_status(status)))
      end

    end

    context 'when communication via command pipe is not possible' do

      scenario 'receives a command' do
        post '/command', command: CMD_CANCEL
        
        expect(last_response.status).to eq(500)
        expect(response_body[:error]).to match(/Unable to send command/i)
      end

    end

  end
end
