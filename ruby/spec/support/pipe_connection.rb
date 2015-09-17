#  File: pipe_connection.rb
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

require 'smith/printer'

module PipeConnection

  # Connection to named pipe that is notified when the command pipe is readable and reads commands
  # from the pipe.  Each line on the command pipe is a command and requires a corresponding
  # expectation or allowance.  Expectations take a block that is called when a command is received
  # and enforce order while allowances allow a command to be received.  If an expectation or
  # allowance does not exist for a received command, an error is raised.

  def initialize
    # Treat arrays as FIFO queue
    # array[0] is the rear of the queue
    # array[n] is the front of the queue
    @expectation_callbacks = []
    @allowed_commands = []
  end

  def add_expectation(&callback)
    @expectation_callbacks.unshift(callback)
  end

  def allow_command(command)
    @allowed_commands.unshift(command)
  end

  def notify_readable
    @io.readlines.map { |line| line.sub("\n", '') }.each do |command|
      if @allowed_commands.last == command
        # This command is allowed, remove the allowance now that it has been received
        @allowed_commands.pop
      else
        # Otherwise an expectation must be present
        call_next_expectation_callback(command)
      end
    end
  end

  def all_expectations_met?
    @expectation_callbacks.empty?
  end

  private

  def call_next_expectation_callback(*args)
    if callback = @expectation_callbacks.pop
      callback.call(*args)
    else
      fail "command pipe received unexpected command: #{args.first.inspect}"
    end
  end

end
