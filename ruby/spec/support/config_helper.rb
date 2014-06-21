module ConfigHelper

  def self.included(including_class)
    including_class.extend ClassMethods
  end

  def wait_for_wireless_config
    # Find the thread performing the configuration and wait for it to complete 
    running = Thread.list.select { |t| t.status == 'run' }
    if running.length > 1
      running.reject { |thread| thread == Thread.main }.first.join
    end
  end

  def stub_iwlist_scan(file_name)
    allow(Smith::Config::WirelessInterface).to receive(:site_survey).and_return(File.read(File.join(Smith.root, 'spec/resource', file_name)))
  end

  module ClassMethods
  end

end
