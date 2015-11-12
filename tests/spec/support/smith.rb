require 'open3'

# Encapsulates execution of main firmware

module Tests
  class Smith

    def start(working_dir)
      Dir.chdir(working_dir) do
        @stdin, @stdout, @stderr, @wait_thr = Open3.popen3('/usr/local/bin/smith')
      end

      # Write the stdout from the dummy server to a file in another thread
      Thread.new do
        Thread.current.abort_on_exception = true
        log = File.open('/root/smith.out', 'w')
        IO.copy_stream(@stdout, log)
        IO.copy_stream(@stderr, log)
      end
    end

    def stop
      if @wait_thr && @wait_thr.alive?
        Process.kill('INT', @wait_thr.pid)
        if @wait_thr.join(5)
          raise 'smith executable exited with non-zero status' unless @wait_thr.value == 0
        else
          puts '~~~~~~ ALERT: Timeout attempting to stop smith via SIGINT, sending SIGKILL ~~~~~~'
          Process.kill('KILL', @wait_thr.pid)
        end
      end
    end

  end
end