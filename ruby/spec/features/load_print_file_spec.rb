require 'spec_helper'

module Smith
  describe 'Load print file', :tmp_dir do
    include FileHelper

    let(:command_pipe) { tmp_dir('command_pipe') }
    let(:print_file) { resource('print.tar.gz') }

    before do
      %x(mkfifo #{command_pipe})
      ENV['COMMAND_PIPE'] = command_pipe
    end
        
    context 'when command pipe is open' do

      before { @output = File.open(command_pipe, 'r+') }
      after { @output.close }

      scenario 'user loads print file' do
        visit '/print_file_uploads/new'
        attach_file 'Select print file to load', print_file 
        
        click_button 'Load'

        expect(page).to have_content /Print file loaded successfully/i
        expect(File.read(tmp_dir('print.tar.gz'))).to eq(File.read(print_file))
        expect(Timeout::timeout(0.1) { @output.gets }).to eq("LoadFile\n")
      end

    end

    scenario 'user loads print file when command pipe is not open' do
      visit '/print_file_uploads/new'
      attach_file 'Select print file to load', print_file 
     
      Timeout::timeout(0.2) { click_button 'Load' } 
    
      expect(page).to have_content /Unable to communicate with print engine/i
    end
    
    scenario 'user loads print file when command pipe does not exist' do
      ENV['COMMAND_PIPE'] = tmp_dir('foo')
      visit '/print_file_uploads/new'
      attach_file 'Select print file to load', print_file 
      
      click_button 'Load'
      
      expect(page).to have_content /Unable to communicate with print engine/i
    end

    scenario 'print file missing on upload' do
      visit '/print_file_uploads/new'
      
      click_button 'Load'
      
      expect(page).to have_content /Please select a print file/i
    end

  end
end
