require 'open4'

module Smith
  module Config
    module System

      module_function

      def link_beat?(interface)
        puts "[INFO] Checking for link beat on #{interface}:"
        puts %x(ifplugstatus #{interface})
        print "\n"
        $?.exitstatus == 2
      end

      def nt_hash(string)
        out = %x(echo -n #{string} | iconv -t utf16le | openssl md4)
        if $?.exitstatus == 0
          out.match(/\A\(stdin\)=\s(.*)\n/m)[0]
        end
      end

      def execute(command)
        stderr_str = nil
        puts "[INFO] Executing #{command}:"
        status = Open4.popen4(command) do |pid, stdin, stdout, stderr|
          stdout_str = stdout.read
          stderr_str = stderr.read
          puts stdout_str unless stdout_str.empty?
        end
        print "\n"
        fail "#{command}: #{stderr_str}" unless status.success?
      end

    end
  end
end
