#  File: dummy_server.rb
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

require 'httpclient'
require 'open3'
require 'json'

class DummyServer
  attr_reader :printer_id, :auth_token, :registration_code, :registration_url

  def http_client
    @http_client ||= HTTPClient.new
  end

  # Make JSON post request to dummy server
  def post(endpoint, body)
    response = http_client.post("#{url}#{endpoint}", body.to_json, { 'Content-Type' => 'application/json', 'Accept' => 'application/json' })
    raise StandardError, 'post request to dummy server unsuccessful' unless response.status_code == 200
    response
  end

  # Make JSON get request to dummy server
  def get(endpoint, params = {})
    response = http_client.get("#{url}#{endpoint}", params, { 'Content-Type' => 'application/json', 'Accept' => 'application/json' })
    raise StandardError, 'get request to dummy server unsuccessful' unless response.status_code == 200
    response
  end

  def url
    @url ||= "http://localhost:#{port}"
  end

  def port
    @port ||= rand(5000...5999)
  end

  def responsive?
    # The identify endpoint returns the constants used by the server
    # Store values so they can be referenced from the tests
    constants = JSON.parse(get('/__identify__').body, symbolize_names: true)
    @registration_url, @printer_id, @auth_token, @registration_code = constants.values_at(:registration_url, :printer_id, :auth_token, :registration_code)
    true
  rescue Errno::ECONNREFUSED
    false
  end

  def start
    # Set the environment
    ENV['RAILS_ENV'] ||= 'test'

    spec_root = File.expand_path('../..', __FILE__)
    start_time = Time.now
    @stdin, @stdout, @stderr, @wait_thr = Open3.popen3(File.join(spec_root, "dummy_server/bin/rails server -p #{port}"))
 
    # Write the stdout from the dummy server to a file in another thread
    Thread.new do
      Thread.current.abort_on_exception = true
      log = File.open(File.join(spec_root, 'dummy_server.out'), 'w')
      IO.copy_stream(@stdout, log)
      IO.copy_stream(@stderr, log)
    end
   
    # Poll to determine when the server is responsive
    Timeout::timeout(10) do
      while !responsive? do
        sleep 0.05
      end
    end

    #puts "Started dummy server in #{(Time.now - start_time) * 1000} ms"

  rescue Timeout::Error
    abort 'Timeout attempting to start dummy server, check spec/dummy_server.out'
  end

  def stop
    if @wait_thr && @wait_thr.alive?
      Process.kill('INT', @wait_thr.pid)
      puts 'Timeout attempting to stop dummy server' unless @wait_thr.join(5)
    end
  end

  # Convenience methods to avoid duplication of endpoint strings
  def post_registration(params)
    post('/v1/user/printers', params)
  end

  def post_command(params)
    post('/command', params)
  end

  def valid_firmware_upgrade_package_url
    "#{url}/smith-0.0.2-valid.tar"
  end

  def invalid_firmware_upgrade_package_url
    "#{url}/smith-0.0.2-invalid_checksum.tar"
  end

  def test_print_file_url
    "#{url}/test_print_file"
  end

  def invalid_url
    "#{url}/bad"
  end

  def latest_firmware_redirect_url
    "#{url}/latest_firmware"
  end

  def test_upload_url
    "#{url}/test_upload"
  end
end
