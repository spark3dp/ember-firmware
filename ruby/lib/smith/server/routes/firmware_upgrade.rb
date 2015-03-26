module Smith
  module Server
    class Application < Sinatra::Base

      helpers do

        def validate_upgrade_package
          if @upgrade_package.nil?
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
