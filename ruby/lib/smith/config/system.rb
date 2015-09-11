#  File: system.rb
#  Functionality for interacting with system
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#  
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'open4'
require 'shellwords'

module Smith
  module Config
    module System
      class Error < StandardError; end

      module_function

      def link_beat?(interface)
        puts "Checking for link beat on #{interface}"
        puts %x(ifplugstatus #{Shellwords.shellescape(interface)})
        $?.exitstatus == 2
      end

      def nt_hash(string)
        return nil if string.strip.empty?
        out = %x(echo -n #{Shellwords.shellescape(string)} | iconv -t utf16le | openssl md4)
        out.match(/\A\(stdin\)=\s(.*)\n/m)[1] if $?.exitstatus == 0
      end

      def wpa_psk(ssid, passphrase)
        # wpa_passphrase will block on stdin if no passphrase is provided
        return nil if passphrase.strip.empty? || ssid.strip.empty?
        out = %x(wpa_passphrase #{Shellwords.shellescape(ssid)} #{Shellwords.shellescape(passphrase)})
        out.match(/\spsk=(.*?)\s/)[1] if $?.exitstatus == 0
      end

      # specify command as an array containing the command and the arguments
      # passing the array as a splat to Open4#popen4 causes the method to escape the command
      def execute(command)
        stderr_str = nil
        puts "Executing #{command.join(' ')}"
        status = Open4.popen4(*command) do |pid, stdin, stdout, stderr|
          stdout_str = stdout.read
          stderr_str = stderr.read
          puts stdout_str unless stdout_str.empty?
        end
        raise(Error, "error executing #{command.join(' ').inspect}: #{stderr_str}") unless status.success?
      end

    end
  end
end
