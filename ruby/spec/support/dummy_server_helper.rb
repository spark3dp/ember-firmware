require 'faye'

module DummyServerHelper

  def dummy_server
    @dummy_server ||= DummyServer.new
  end

end
