ENV['WATCH_DIR']    ||= '/boot/uboot/print_jobs'
ENV['COMMAND_PIPE'] ||= '/tmp/cmd_pipe'

require 'listen'

module Smith
  class Watcher

    def start
      @listener = Listen.to(watch_dir) do
        job_file = Dir[File.join(watch_dir, '*.gz')].first
        write_command("CMD #{job_file}")
      end
      @listener.start
    end

    def stop
      if @listener
        @listener.stop
        @listener = nil
      end
    end

    def write_command(command)
      File.open(command_pipe, 'w+') { |file| file.print(command) }
    end

    def watch_dir
      ENV['WATCH_DIR']
    end

    def command_pipe
      ENV['COMMAND_PIPE']
    end

  end
end
