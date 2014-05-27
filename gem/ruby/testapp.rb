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

   input = open("/tmp/StatusToWebPipe", "r+") # the r+ means we don't block
   #   puts input.gets # will block if there's nothing in the pipe

   start = request.env['HTTP_LAST_EVENT_ID'] ? request.env['HTTP_LAST_EVENT_ID'].to_i+1 : 0

   content_type "text/event-stream"

   stream do |out|
     # out << "data: #{params[:cmd]}\n\n"
     start.upto(11) do |i|
       out << "data: #{input.gets}\n"
     end
     out << "data: \n\n"
   end
  end

end

