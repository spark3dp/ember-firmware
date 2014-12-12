# In memory (non-file backed) state implementation

require 'ostruct'

class InMemoryState < OpenStruct

  def save
  end

  def get_binding
    binding
  end

  def update(hash)
    hash.each do |key, value|
      send("#{key}=", value)
    end
  end

end
