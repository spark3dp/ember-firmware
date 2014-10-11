require 'server_helper'

module Smith
  describe 'Load print file', :tmp_dir do
    include PrintEngineHelper

    let(:print_file) { resource 'print.tar.gz' }
    let(:stale_print_file) { File.join(print_data_dir, 'old_print.tar.gz') }
    let(:uploaded_print_file) { File.join(print_data_dir, 'print.tar.gz') }

    before do
      create_command_pipe
      create_command_response_pipe
      create_print_data_dir
    end
    
    context 'when communication via command pipe is possible' do

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

      scenario 'user loads print file when printer is not in valid state' do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file

        write_get_status_command_response(state: PRINTING_STATE, substate: NO_SUBSTATE)

        click_button 'Load'

        expect(page).to have_content /Printer state \(state: "#{PRINTING_STATE}", substate: "#{NO_SUBSTATE}"\) invalid/i
      end

    end

    scenario 'user loads print file when communication via command pipe is not possible' do
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
