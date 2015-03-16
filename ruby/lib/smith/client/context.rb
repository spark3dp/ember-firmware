require 'ostruct'
class Context < OpenStruct

  def get_binding
    binding
  end

end