require 'rspec/em'
require 'vcr'

VCRSteps = RSpec::EM.async_steps do
  def insert_vcr_cassette(name, &callback)
    # Specify record once option
    # This results in:
    # - Replay previously recorded interactions
    # - Record new interactions if there is no cassette file
    # - Cause an error to be raised for new requests if there is a cassette file
    VCR.insert_cassette(name, record: :once, preserve_exact_body_bytes: true)
    callback.call
  end

  def eject_vcr_cassette(&callback)
    VCR.eject_cassette
    callback.call
  end
end
