require 'mkfifo'
require 'fileutils'

module PrintEngineHelper

  def self.included(including_class)
    including_class.class_exec do
      let(:command_pipe) { tmp_dir 'command_pipe' }
      let(:command_response_pipe) { tmp_dir 'command_response_pipe' }
      let(:print_data_dir) { tmp_dir 'print_data' }

      # Helper methods provided by this module require a temporary directory
      if metadata[:client]
        # Client specs are asynchronous and need the async tmp dir hooks
        metadata[:tmp_dir_async] = true
      else
        # Otherwise just use the synchronous hooks
        metadata[:tmp_dir] = true
      end
    end
  end

  def printer_status(vars)
    {
      Smith::PRINTER_STATUS_KEY =>
      {
          Smith::STATE_PS_KEY => vars[:state],
          Smith::UISUBSTATE_PS_KEY => vars[:substate],
      }
    }
  end

  def create_command_pipe
    File.mkfifo(command_pipe)
    ENV['COMMAND_PIPE'] = command_pipe
  end

  def create_command_response_pipe
    File.mkfifo(command_response_pipe)
    ENV['COMMAND_RESPONSE_PIPE'] = command_response_pipe
  end

  def open_command_pipe
    @command_pipe_io = File.open(command_pipe, 'r+')
  end

  def close_command_pipe
    @command_pipe_io.close
  end

  def next_command_in_command_pipe
    Timeout::timeout(0.1) { @command_pipe_io.gets.sub("\n", '') }
  rescue Timeout::Error
    fail 'timeout waiting to read from command pipe'
  end

  def open_command_response_pipe
    @command_response_pipe_io = File.open(command_response_pipe, 'r+')
  end

  def close_command_response_pipe
    @command_response_pipe_io.close
  end

  def write_get_status_command_response(status_values)
    File.write(@command_response_pipe_io, printer_status(status_values).to_json + "\n")
  end

  def create_print_data_dir
    FileUtils.mkdir(ENV['PRINT_DATA_DIR'] = print_data_dir)
  end

end
