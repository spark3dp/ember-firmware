module Smith
  module ConfigApp
    module AppHelper

      def h(text)
        Rack::Utils.escape_html(text)
      end

      def hidden_fields(object)
        fields = []
        object.marshal_dump.each_pair do |k, v|
          fields << %Q(<input type="hidden" name="wireless_network[#{k}]" value="#{v}" />)
        end 
        fields.join("\n")
      end

    end
  end
end
