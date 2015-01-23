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

  def test_firmware_upgrade_package_url
    "#{url}/test_firmware_upgrade_package"
  end

  def test_print_file_url
    "#{url}/test_print_file"
  end

  def invalid_url
    "#{url}/bad"
  end

  def redirect_url
    "#{url}/redirect"
  end
end
