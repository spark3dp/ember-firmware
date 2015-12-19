require 'rmagick'

module Tests
  class Projector

    def initialize(image_path)
      @image_path = image_path
    end

    class ConditionNotMet < StandardError; end

    def showing_black?
      timeout_seconds = 5
      start_time_seconds = Time.now.to_f

      begin
        if File.file?(@image_path)
          image = Magick::Image.read(@image_path).first
          black = Magick::Image.new(1280, 800) { self.background_color = 'black' }

          difference = black.difference(image)

          image.destroy!
          black.destroy!

          if difference[0] > 0.0
            return false
          else
            return true
          end
        else
          raise ConditionNotMet
        end
      rescue ConditionNotMet
        return false if Time.now.to_f - start_time_seconds >= timeout_seconds
        sleep(0.05)
        retry
      end
    end

  end
end
