require 'open4'

module Smith
  module Config
    module System
      class Error < StandardError; end

      module_function

      def link_beat?(interface)
        puts "Checking for link beat on #{interface}"
        puts %x(ifplugstatus #{interface})
        $?.exitstatus == 2
      end

      def nt_hash(string)
        return nil if string.strip.empty?
        out = %x(echo -n #{string} | iconv -t utf16le | openssl md4)
        out.match(/\A\(stdin\)=\s(.*)\n/m)[1] if $?.exitstatus == 0
      end

      def wpa_psk(ssid, passphrase)
        # wpa_passphrase will block on stdin if no passphrase is provided
        return nil if passphrase.strip.empty? || ssid.strip.empty?
        out = %x(wpa_passphrase #{ssid} #{passphrase})
        out.match(/\spsk=(.*?)\s/)[1] if $?.exitstatus == 0
      end

      def execute(command)
        stderr_str = nil
        puts "Executing #{command}"
        status = Open4.popen4(command) do |pid, stdin, stdout, stderr|
          stdout_str = stdout.read
          stderr_str = stderr.read
          puts stdout_str unless stdout_str.empty?
        end
        raise(Error, "error executing #{command.inspect}: #{stderr_str}") unless status.success?
      end

    end
  end
end
