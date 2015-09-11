#  File: print_engine_helper.rb
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

require 'mkfifo'
require 'fileutils'
require 'support/file_helper'

module PrintEngineHelper

  def self.included(including_class)
    including_class.class_exec do
      include FileHelper
      let(:command_pipe) { tmp_dir 'command_pipe' }
      let(:print_data_dir) { tmp_dir 'print_data' }
      let(:printer_status_file) { tmp_dir 'printer_status' }

      # Helper methods provided by this module require a temporary directory
      if metadata[:client]
        # Client specs are asynchronous and need the async tmp dir hooks
        metadata[:tmp_dir_async] = true
      else
        # Otherwise just use the synchronous hooks
        metadata[:tmp_dir] = true
      end
    end
  end

  def create_printer_status_file
    FileUtils.touch(printer_status_file)
    Smith::Settings.printer_status_file = printer_status_file
  end

  def set_printer_status(status_values)
    File.write(printer_status_file, printer_status(status_values).to_json + "\n")
  end

  def printer_status(vars)
    { Smith::STATE_PS_KEY => vars[:state],
      Smith::UISUBSTATE_PS_KEY => vars[:ui_sub_state],
      Smith::SPARK_STATE_PS_KEY => vars[:spark_state],
      Smith::ERROR_CODE_PS_KEY => vars[:error_code],
      Smith::ERROR_MSG_PS_KEY => vars[:error_message],
      Smith::SPARK_JOB_STATE_PS_KEY => vars[:spark_job_state],
      Smith::JOB_ID_PS_KEY => vars[:job_id],
      Smith::TOTAL_LAYERS_PS_KEY => vars[:total_layers],
      Smith::LAYER_PS_KEY => vars[:layer],
      Smith::LOCAL_JOB_UUID_PS_KEY => vars[:spark_local_job_uuid] }
  end

  def create_command_pipe
    File.mkfifo(command_pipe)
    Smith::Settings.command_pipe = command_pipe
  end

  def open_command_pipe
    @command_pipe_io = File.open(command_pipe, 'r+')
  end

  def close_command_pipe
    @command_pipe_io.close
  end

  def next_command_in_command_pipe
    Timeout::timeout($test_named_pipe_timeout) { @command_pipe_io.gets.sub("\n", '') }
  rescue Timeout::Error
    fail 'timeout waiting to read from command pipe, pipe does not contain a line to be read'
  end

  def create_print_data_dir
    FileUtils.mkdir(Smith::Settings.print_data_dir = print_data_dir)
  end

  def keys_to_symbols(hash)
    JSON.parse(hash.to_json, symbolize_names: true)
  end

  def assert_connecting_connected_commands_sent
    expect(next_command_in_command_pipe).to eq(Smith::CMD_SHOW_WIRELESS_CONNECTING)
    expect(next_command_in_command_pipe).to eq(Smith::CMD_SHOW_WIRELESS_CONNECTED)
  end

end
