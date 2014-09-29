require 'server_helper'

module Smith
  describe 'Load print file' do
    include FileHelper
    include PrintEngineHelper

    let(:upload_dir) { tmp_dir 'uploads' }
    let(:print_file) { resource 'print.tar.gz' }
    let(:stale_print_file) { File.join(upload_dir, 'old_print.tar.gz') }
    let(:uploaded_print_file) { File.join(upload_dir, 'print.tar.gz') }

    before do
      create_command_pipe
      create_command_response_pipe
      FileUtils.mkdir(upload_dir)
      ENV['UPLOAD_DIR'] = upload_dir
    end
    
    context 'when command pipe is open' do

      before do
        open_command_pipe
        open_command_response_pipe
      end

      after do
        close_command_pipe
        close_command_response_pipe
      end


      scenario 'user loads print file when printer is in Home state' do
        # Create a stale print file
        FileUtils.touch(stale_print_file)

        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file

        write_get_status_command_response(state: HOME_STATE, substate: NO_SUBSTATE)
        write_get_status_command_response(state: HOME_STATE, substate: DOWNLOADING_SUBSTATE)

        click_button 'Load'

        expect(page).to have_content /Print file loaded successfully/i
        expect(File.read(uploaded_print_file)).to eq(File.read(print_file))
        expect(next_command_in_command_pipe).to eq(CMD_GET_STATUS)
        expect(next_command_in_command_pipe).to eq(CMD_PRINT_DATA_LOAD)
        expect(next_command_in_command_pipe).to eq(CMD_GET_STATUS)
        expect(next_command_in_command_pipe).to eq(CMD_PROCESS_PRINT_DATA)

        # Stale print files are removed
        expect(File.file?(stale_print_file)).to eq(false)
      end

      scenario 'user attempts to loads print file when printer is in Home state, but printer fails to show loading screen' do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file

        write_get_status_command_response(state: HOME_STATE, substate: NO_SUBSTATE)
        write_get_status_command_response(state: HOME_STATE, substate: DOWNLOAD_FAILED_SUBSTATE)

        click_button 'Load'

        expect(page).to have_content /Printer state \(state: "#{HOME_STATE}", substate: "#{DOWNLOAD_FAILED_SUBSTATE}"\) invalid/i
      end

      scenario 'user loads print file when printer is not in ready state' do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file

        write_get_status_command_response(state: PRINTING_STATE, substate: NO_SUBSTATE)

        click_button 'Load'

        expect(page).to have_content /Printer state \(state: "#{PRINTING_STATE}", substate: "#{NO_SUBSTATE}"\) invalid/i
      end


      scenario 'print engine does not return response to get status command' do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file 
        
        click_button 'Load'

        expect(page).to have_content /Did not receive response from printer:/i
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

    scenario 'print file is not a .tar.gz file' do
      visit '/print_file_uploads/new'
      attach_file 'Select print file to load', resource('smith-0.0.2-valid.tar')
      
      click_button 'Load'
      
      expect(page).to have_content /Please select a .tar.gz file/i
    end
   
  end
end
