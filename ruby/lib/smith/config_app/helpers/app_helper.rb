module Smith
  module ConfigApp
    module AppHelper
      def h(text)
	Rack::Utils.escape_html(text)
      end

      def flatten_hash(hash = params, ancestor_names = [])
	flat_hash = {}
	hash.each do |k, v|
	  names = Array.new(ancestor_names)
	  names << k
	  if v.is_a?(Hash)
	    flat_hash.merge!(flatten_hash(v, names))
	  else
	    key = flat_hash_key(names)
	    key += '[]' if v.is_a?(Array)
	    if v.is_a?(String)
	      flat_hash[key] = v
	    else
	      flat_hash[key] = flatten_hash(v)
	    end
	  end
	end
	
	flat_hash
      end
      
      def flat_hash_key(names)
	names = Array.new(names)
	name = names.shift.to_s.dup 
	names.each do |n|
	  name << "[#{n}]"
	end
	name
      end
      
      def hash_as_hidden_fields(hash = params)
	hidden_fields = []
	flatten_hash(hash).each do |name, value|
	  value = [value] if !value.is_a?(Array)
	  value.each do |v|
	    hidden_fields << %Q(<input type="hidden" name="#{name}" value="#{v.to_s}" />)
	  end
	end
	
	hidden_fields.join("\n")
      end
    end
  end
end
