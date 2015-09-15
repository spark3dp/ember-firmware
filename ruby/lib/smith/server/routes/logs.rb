#  File: logs.rb
#  Sinatra routes for log archive downloads
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
