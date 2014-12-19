require 'server_helper'

module Smith
  describe 'command interface', :tmp_dir do
    include PrintEngineHelper
    include Rack::Test::Methods
   
    let(:response_body) { JSON.parse(last_response.body, symbolize_names: true) }

    context 'when communication via command pipe is possible' do

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

      scenario 'receives command that does not return a response' do
        post '/command', command: CMD_CANCEL

        expect(next_command_in_command_pipe).to eq(CMD_CANCEL)
        expect(last_response.status).to eq(200)
        expect(response_body).to eq(command: CMD_CANCEL)
      end

      scenario 'receives a request without command parameter' do
        post '/command'

        expect(last_response.status).to eq(400)
        expect(response_body[:error]).to match(/command parameter is requred/i)
      end

      scenario 'receives get status command' do
        status = { state: HOME_STATE, substate: NO_SUBSTATE }
        write_get_status_command_response(status)

        post '/command', command: CMD_GET_STATUS

        expect(next_command_in_command_pipe).to eq(CMD_GET_STATUS)
        expect(last_response.status).to eq(200)
        expect(response_body[:command]).to eq(CMD_GET_STATUS)
        expect(response_body[:response]).to eq(keys_to_symbols(printer_status(status)))
      end

      scenario 'receives get firmware version command' do
        write_command_response('x.x.x.x')

        post '/command', command: CMD_GET_FW_VERSION
        
        expect(next_command_in_command_pipe).to eq(CMD_GET_FW_VERSION)
        expect(last_response.status).to eq(200)
        expect(response_body[:command]).to eq(CMD_GET_FW_VERSION)
        expect(response_body[:response]).to eq('x.x.x.x')
      end

      scenario 'receives get board serial number command' do
        write_command_response('12345678')
        
        post '/command', command: CMD_GET_BOARD_NUM

        expect(next_command_in_command_pipe).to eq(CMD_GET_BOARD_NUM)
        expect(last_response.status).to eq(200)
        expect(response_body[:command]).to eq(CMD_GET_BOARD_NUM)
        expect(response_body[:response]).to eq('12345678')
      end

    end

    context 'when communication via command pipe is not possible' do

      scenario 'receives a command' do
        post '/command', command: CMD_CANCEL
        
        expect(last_response.status).to eq(500)
        expect(response_body[:error]).to match(/Unable to communicate with printer/i)
      end

    end

  end
end
