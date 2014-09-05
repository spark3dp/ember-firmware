require 'rubygems/package'

# Create custom tar writer class with additional method allowing mtime header value to be specified
module Smith
  class TarWriter < Gem::Package::TarWriter
    def add_file_mtime(name, mode, mtime) # :yields: io
      check_closed

      raise Gem::Package::NonSeekableIO unless @io.respond_to? :pos=

      name, prefix = split_name name

      init_pos = @io.pos
      @io.write "\0" * 512 # placeholder for the header

      yield RestrictedStream.new(@io) if block_given?

      size = @io.pos - init_pos - 512

      remainder = (512 - (size % 512)) % 512
      @io.write "\0" * remainder

      final_pos = @io.pos
      @io.pos = init_pos

      header = Gem::Package::TarHeader.new :name => name, :mode => mode,
                                           :size => size, :prefix => prefix,
                                           :mtime => mtime

      @io.write header
      @io.pos = final_pos

      self
    end
  end
end
