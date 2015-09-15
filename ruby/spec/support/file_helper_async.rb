#  File: file_helper_async.rb
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

module FileHelperAsync

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      include FileHelper

      steps = RSpec::EM.async_steps do

        def remove_tmp_dir_async(&callback)
          remove_tmp_dir
          callback.call
        end

        def make_tmp_dir_async(&callback)
          make_tmp_dir
          callback.call
        end

      end

      include steps
    end
  end

end
