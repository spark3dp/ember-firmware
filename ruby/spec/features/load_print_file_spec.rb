require 'spec_helper'

module Smith
  describe 'Load print file', :tmp_dir do
    include FileHelper
    include PrintEngineHelper

    let(:command_pipe) { tmp_dir 'command_pipe' }
    let(:command_response_pipe) { tmp_dir 'command_response_pipe' }
    let(:print_file) { resource 'print.tar.gz' }

    before do
      %x(mkfifo #{command_pipe})
      %x(mkfifo #{command_response_pipe})
      ENV['COMMAND_PIPE'] = command_pipe
      ENV['COMMAND_RESPONSE_PIPE'] = command_response_pipe
    end
    
    context 'when command pipe is open' do

      before do
        @command_pipe_io = File.open(command_pipe, 'r+')
        @command_response_pipe_io = File.open(command_response_pipe, 'r+')
      end

      after do
        @command_pipe_io.close
        @command_response_pipe_io.close
      end


      scenario "user loads print file when printer is in Home state" do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file

        File.write(command_response_pipe, printer_status(state: 'Home', substate: 'NoUISubState').to_json + "\n")
        File.write(command_response_pipe, printer_status(state: 'Home', substate: 'Downloading').to_json + "\n")

        click_button 'Load'

        expect(page).to have_content /Print file loaded successfully/i
        expect(File.read(tmp_dir('print.tar.gz'))).to eq(File.read(print_file))
        expect(Timeout::timeout(0.1) { @command_pipe_io.gets }).to eq("GETSTATUS\n")
        expect(Timeout::timeout(0.1) { @command_pipe_io.gets }).to eq("STARTPRINTDATALOAD\n")
        expect(Timeout::timeout(0.1) { @command_pipe_io.gets }).to eq("GETSTATUS\n")
        expect(Timeout::timeout(0.1) { @command_pipe_io.gets }).to eq("PROCESSPRINTDATA\n")
      end

      scenario "user attempts to loads print file when printer is in Home state, but fails to show loading screen" do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file

        File.write(command_response_pipe, printer_status(state: 'Home', substate: 'NoUISubState').to_json + "\n")
        File.write(command_response_pipe, printer_status(state: 'Home', substate: 'DownloadFailed').to_json + "\n")

        click_button 'Load'

        expect(page).to have_content /Printer cannot accept file while in Home state and DownloadFailed substate/i
      end

      scenario 'user loads print file when printer is not in ready state' do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file

        File.write(command_response_pipe, printer_status(state: 'Printing', substate: 'NoUISubState').to_json + "\n")

        click_button 'Load'

        expect(page).to have_content /Printer cannot accept file while in Printing state and NoUISubState substate/i
      end


      scenario 'print engine does not return response to get status command' do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file 
        
        click_button 'Load'

        expect(page).to have_content /Did not receive printer status:/i
      end

      scenario 'configured response pipe does not exist' do
        ENV['COMMAND_RESPONSE_PIPE'] = tmp_dir('foo')
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file 
        
        click_button 'Load'
        
        expect(page).to have_content /Unable to communicate with printer/i
      end

    end

    scenario 'user loads print file when command pipe is not open' do
      visit '/print_file_uploads/new'
      attach_file 'Select print file to load', print_file 
     
      Timeout::timeout(0.2) { click_button 'Load' } 
    
      expect(page).to have_content /Unable to communicate with printer/i
    end
    
    scenario 'user loads print file when command pipe does not exist' do
      ENV['COMMAND_PIPE'] = tmp_dir('foo')
      visit '/print_file_uploads/new'
      attach_file 'Select print file to load', print_file 
      
      click_button 'Load'
      
      expect(page).to have_content /Unable to communicate with printer/i
    end

    scenario 'print file missing on upload' do
      visit '/print_file_uploads/new'

      click_button 'Load'
      
      expect(page).to have_content /Please select a print file/i
    end
   
  end
end
