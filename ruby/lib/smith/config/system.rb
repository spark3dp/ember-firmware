require 'open4'

module Smith
  module Config
    module System

      module_function

      def link_beat?(interface)
        log "Checking for link beat on #{interface}"
        print "\t"
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
        log "Executing #{command}"
        status = Open4.popen4(command) do |pid, stdin, stdout, stderr|
          stdout_str = stdout.read
          stderr_str = stderr.read
          stdout_str.split("\n").each do |l|
            print("\t")
            puts(l)
          end
        end
        fail "#{command}: #{stderr_str}" unless status.success?
      end

      def log(msg)
        puts "[INFO smith-config #{Time.now}] #{msg}"
      end

    end
  end
end
