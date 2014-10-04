require 'rspec/em'

# Steps that are shared between different contexts
ClientSteps = RSpec::EM.async_steps do

  # Start the client and wait unitl primary registration is complete
  # Does not make any expectations on registration commands to avoid over testing
  def allow_primary_registration(&callback)
    write_get_status_command_response(state: Smith::HOME_STATE)
    expect_get_status_command

    add_command_pipe_allowance do
      # this callback gets called when the CMD_REGISTRATION_CODE command is sent
      # the callback "callback" gets called when CMD_REGISTERED is sent
      dummy_server.post('/v1/user/printers', registration_code: '4321')
    end
    
    add_command_pipe_allowance(&callback)

    start_client
  end

end
