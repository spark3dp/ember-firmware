require 'httpclient'
require 'open3'

class DummyServer
  def http_client
    @http_client ||= HTTPClient.new
  end

  def post(endpoint, *args, &block)
    response = http_client.post("#{url}#{endpoint}", *args, &block)
    raise StandardError, 'post request to dummy server unsuccessful' unless response.status_code == 200
    response
  end

  def get(endpoint, *args, &block)
    response = http_client.get("#{url}#{endpoint}", *args, &block)
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
    get('/__identify__')
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
    Timeout::timeout(2) do
      while !responsive? do
        sleep 0.05
      end
    end

    #puts "Started dummy server in #{(Time.now - start_time) * 1000} ms"

  rescue Timeout::Error
    abort 'Timeout attempting to start dummy server'
  end

  def stop
    if @wait_thr && @wait_thr.alive?
      Process.kill('INT', @wait_thr.pid)
      puts 'Timeout attempting to stop dummy server' unless @wait_thr.join(5)
    end
  end
end
