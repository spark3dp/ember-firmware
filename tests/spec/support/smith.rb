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
        log = File.open('smith.out', 'w')
        IO.copy_stream(@stdout, log)
        IO.copy_stream(@stderr, log)
      end
    end

    def stop
      if @wait_thr && @wait_thr.alive?
        Process.kill('INT', @wait_thr.pid)
        puts 'Timeout attempting to stop smith' unless @wait_thr.join(5)
      end
    end

  end
end