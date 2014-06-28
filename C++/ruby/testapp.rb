require 'sinatra'

set :server, :thin
set :bind, '0.0.0.0'

get '/' do
  send_file "index.html"
end

get '/command' do
  # puts params.inspect
  # puts request.body.read.inspect
  # puts "Last-Event-Id: #{request.env['HTTP_LAST_EVENT_ID']}"

  output = open("/tmp/CommandPipe", "w+") # w+ means don't block
  output.puts "#{params[:cmd]}\n"
  output.flush

  if params[:cmd] == 'GetStatus'

   input = open("/tmp/CommandResponsePipe", "r+") # the r+ means we don't block

   start = request.env['HTTP_LAST_EVENT_ID'] ? request.env['HTTP_LAST_EVENT_ID'].to_i+1 : 0

   content_type "text/event-stream"

   stream do |out|
    line = input.gets
    out << "data: #{line}\n"

    # for debug
    puts 'status:'
    puts line
    puts 

   end
  end

end

