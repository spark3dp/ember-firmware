#  File: dummy_server_helper.rb
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

module DummyServerHelper

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'

      steps = RSpec::EM.async_steps do

        def subscribe_to_test_channel_async(&callback)
          # @http_request_callbacks is a hash with each key/value pair corresponding
          # to an endpoint and an array of expectations callbacks set for that endpoint
          # Treat arrays as FIFO queue
          # array[0] is the rear of the queue
          # array[n] is the front of the queue
          @http_request_callbacks = Hash.new { |hash, key| hash[key] = [] }

          @subscription = Faye::Client.new("#{dummy_server.url}/faye").subscribe('/test') do |raw_payload|
            payload = JSON.parse(raw_payload, symbolize_names: true)

            # If there is a callback to call for this endpoint, call it with the request params
            if callback = @http_request_callbacks[payload[:request_endpoint]].pop
              callback.call(payload[:request_params])
            end
          end

          @subscription.callback { callback.call }
          @subscription.errback { raise 'error subscribing to test channel' }
        end

        def unsubscribe_from_test_channel_async(&callback)
          @subscription.cancel
          # Let the cancellation get processed
          EM.next_tick { callback.call }
        end

      end

      include steps
    end
  end

  def dummy_server
    $dummy_server
  end

  # Add a callback to be called when the client makes a request to the specified endpoint
  # Also returns a deferrable object that succeeds when the specified request is made for
  # convenience when setting multiple expectations in a single step
  def add_http_request_expectation(endpoint, &callback)
    step_method = caller[0].sub(Dir.getwd, '.')
    timer = EM.add_timer($client_expectation_timeout) { raise "Timeout waiting for expected HTTP request (expectation added #{step_method})" }
    deferrable = EM::DefaultDeferrable.new
    # Callback arrays are keyed by endpoint without the domain name
    @http_request_callbacks[endpoint.sub(dummy_server.url, '')].unshift(proc do |*args|
      EM.cancel_timer(timer)
      callback.call(*args) if callback
      deferrable.succeed
    end)
    deferrable
  end

end
