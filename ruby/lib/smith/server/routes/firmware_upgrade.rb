#  File: firmware_upgrade.rb
#  Sinatra routes for upgrading firmware
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

module Smith
  module Server
    class Application < Sinatra::Base

      helpers do

        def validate_upgrade_package
          unless is_file_upload?(@upgrade_package)
            flash.now[:error] = 'Please select a firmware upgrade package'
            halt erb :firmware_upgrade
          end

          if @upgrade_package[:filename] !~ /\A.+?\.tar\z/i
            flash.now[:error] = 'Upgrade package must be a tar archive'
            halt erb :firmware_upgrade
          end
        end

        def upgrade_firmware
          Config::Firmware.upgrade(@upgrade_package[:tempfile].path)
        rescue Config::Firmware::UpgradeError => e
          flash.now[:error] = "Unable to complete firmware upgrade (#{e.message})"
          halt erb :firmware_upgrade
        ensure
          @upgrade_package[:tempfile].close!
        end

      end

      get '/firmware_upgrade' do
        erb :firmware_upgrade
      end

      post '/firmware_upgrade' do
        @upgrade_package = params[:upgrade_package]

        validate_upgrade_package
        upgrade_firmware

        flash[:success] = 'Firmware upgraded successfully, please reboot printer'
        redirect to '/firmware_upgrade'
      end

    end
  end
end
