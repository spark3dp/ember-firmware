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
