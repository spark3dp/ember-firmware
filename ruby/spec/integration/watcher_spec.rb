require 'spec_helper'

module Smith
  describe Watcher, :tmp_dir do
   
    let(:target) { double 'target' }
    let(:pipe) { File.join(@tmp_path, 'pipe') }
    let(:watch_dir) { File.join(@tmp_path, 'watch_dir') }
    let(:job_file) { File.join(watch_dir, 'job.tar.gz') }

    subject(:watcher) { described_class.new }
    
    def read_pipe
      contents = nil
      Timeout::timeout 1 do
        file = File.open(pipe)
        contents = file.read
        file.close
      end
    rescue Timeout::Error
      # rescue so the test will fail from unmet assertions rather than a timeout error
      puts 'Pipe read timeout'
    ensure
      return contents
    end

    def start_watcher
      watcher.start
      
      # allow time for Listen to pick up the change
      sleep 0.05
    end

    before do
      %x(mkfifo #{pipe})
      Dir.mkdir(watch_dir)

      ENV['COMMAND_PIPE'] = pipe
      ENV['WATCH_DIR'] = watch_dir
    end

    after do
      watcher.stop
    end

    context 'when watch directory is empty' do
      context 'when print job file is added to watch directory' do
        it 'writes command with path of added print job file to command pipe' do
          start_watcher
          FileUtils.touch(job_file)
          expect(read_pipe).to eq("CMD #{job_file}")
        end 
      end
    
      context 'non print job file is added to watch directory' do
        it 'does not write command' do
        end
      end
    end

    context 'when watch directory contains print job file' do
      scenario 'print job file is added to watch directory'
      scenario 'print job file is removed from watch directory'
    end

    context 'when watch directory contains non print job file' do
      scenario 'print job file is added to watch directory'
    end

  end
end
