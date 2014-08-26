module Smith
  module App
    class Application < Sinatra::Base
      
      get '/logs' do
        send_file Application.log_file, filename: "smith-syslog-#{Time.now.strftime('%m-%d-%Y')}"
      end

    end
  end
end
