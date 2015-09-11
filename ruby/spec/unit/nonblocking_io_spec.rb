#  File: nonblocking_io_spec.rb
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

require 'common_helper'
require 'smith/nonblocking_io'

module Smith
  describe NonblockingIO do

    context 'when reading lines from reading end of IO pipe in non-blocking fashion' do

      let(:pipe) { IO.pipe }
      let(:pipe_read_io) { pipe[0] }
      let(:pipe_write_io) { pipe[1] }
      
      subject(:io) { described_class.new(pipe_read_io) }

      context 'when bytes separated by new-line character are available for reading' do

        before { pipe_write_io.write("line1\\nstillline1\nline2\nline3\n") }

        it 'returns array with elements corresponding to lines in byte sequence' do
          expect(io.readlines_nonblock).to eq(["line1\\nstillline1\n", "line2\n", "line3\n"])
        end

      end

      context 'when writing end of pipe is closed' do

        before { pipe_write_io.close }

        it 'returns empty array' do
          expect(io.readlines_nonblock).to eq([])
        end

      end

      context 'when no bytes are available to be read' do

        it 'returns empty array' do
          expect(io.readlines_nonblock).to eq([])
        end

      end

    end

  end
end
