module Smith
  class SettingsError < StandardError; end
  module SettingsFactory
    module_function

    def build(settings)
      # Check for environment overrides
      settings.each do |key, value|
        if overridden_value = ENV["SMITH_#{key.to_s.upcase}"]
          settings[key] =
            if value.is_a?(String)
              overridden_value
            elsif value.is_a?(Fixnum)
              overridden_value.to_i
            else
              raise(SettingsError, "cannot convert overridden setting value for #{key.to_s.inspect} to #{value.class}")
            end
        end
      end

      settings_class = Struct.new(*settings.keys)
      settings_class.class_eval do
        def method_missing(method_name, *args, &block)
          raise(SettingsError, "setting #{method_name.to_s.inspect} does not exist")
        end
      end
      settings_class.new(*settings.values)
    end

  end
end
