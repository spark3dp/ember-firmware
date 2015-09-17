#  File: log_connection.rb
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

module LogConnection

  class LogSubscription
    def initialize(callback, connection)
      @callback, @connection = callback, connection
    end

    def call(entries)
      @callback.call(entries)
    end

    def cancel
      @connection.cancel_subscription(self)
    end
  end

  def initialize(copy_to_stdout)
    @subscriptions = []
    @copy_to_stdout = copy_to_stdout
    @entries = []
  end

  def add_subscription(&callback)
    subscription = LogSubscription.new(callback, self)
    @subscriptions.push(subscription)
    subscription
  end

  def cancel_subscription(subscription)
    @subscriptions.delete(subscription).inspect
  end

  def entries
    @entries
  end

  def receive_data(data)
    # Split data into array of log entries
    entries = data.split("\n")

    # Store the entries logged
    @entries.push(*entries)

    # Print log entries if enabled
    puts entries if @copy_to_stdout

    # Listeners just get called with whatever is read from the log io
    # Order is not important as it is with expectations
    @subscriptions.each { |l| l.call(entries) }
  end

end
