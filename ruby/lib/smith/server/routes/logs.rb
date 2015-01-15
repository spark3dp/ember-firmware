require 'smith/logs'

module Smith
  module Server
    class Application < Sinatra::Base
      
      get '/logs' do
        headers['Content-Disposition'] = "attachment;filename=smith-#{Smith::VERSION}_logs_#{Time.now.strftime('%m-%d-%Y')}_#{Printer.serial_number}.tar.gz"
        # Returns a contents of gzipped tar archive that becomes the body of the response
        Logs.get_archive
      end

    end
  end
end
