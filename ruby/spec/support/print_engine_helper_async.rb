#  File: print_engine_helper_async.rb
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

module PrintEngineHelperAsync

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      require 'fcntl'
      require 'fileutils'

      include PrintEngineHelper

      steps = RSpec::EM.async_steps do

        def create_command_pipe_async(&callback)
          create_command_pipe
          callback.call
        end

        def create_printer_status_file_async(&callback)
          create_printer_status_file
          callback.call
        end

        def watch_command_pipe_async(&callback)
          fd = IO.sysopen(command_pipe, Fcntl::O_RDONLY | Fcntl::O_NONBLOCK)
          @command_pipe_io = IO.new(fd, Fcntl::O_RDONLY | Fcntl::O_NONBLOCK)
          @command_pipe_connection = EM.watch(@command_pipe_io, PipeConnection)
          @command_pipe_connection.notify_readable = true
          callback.call
        end

        def close_command_pipe_async(&callback)
          if @command_pipe_connection
            @command_pipe_connection.detach
            fail 'all command pipe expectations not met' unless @command_pipe_connection.all_expectations_met?
          end
          @command_pipe_io.close if @command_pipe_io
          @command_pipe_io = nil
          @command_pipe_connection = nil
          callback.call
        end

        def set_printer_status_async(status, &callback)
          set_printer_status(status)
          callback.call
        end

        def create_print_data_dir_async(&callback)
          create_print_data_dir
          callback.call
        end

      end

      include steps
    end
  end

  # Add a callback to be called when the command pipe receives a command
  # Also returns a deferrable object that succeeds when the command is received for
  # convenience when setting multiple expectations in a single step
  def add_command_pipe_expectation(&block)
    step_method = caller[0].sub(Dir.getwd, '.')
    timer = EM.add_timer($client_expectation_timeout) { raise "Timeout waiting for write to command pipe (expectation added #{step_method})" }
    deferrable = EM::DefaultDeferrable.new
    @command_pipe_connection.add_expectation do |*args|
      EM.cancel_timer(timer)
      block.call(*args) if block
      deferrable.succeed
    end
    deferrable
  end

end
