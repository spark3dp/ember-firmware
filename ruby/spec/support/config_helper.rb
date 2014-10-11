module ConfigHelper

  def wait_for_wireless_config
    # Find the thread performing the configuration and wait for it to complete 
    config_thread = Thread.list.select { |thread| thread[:name] == :config }.first
    config_thread.join if config_thread
  end

  def stub_iwlist_scan(file_name)
    allow(Smith::Config::WirelessInterface).to receive(:site_survey).and_return(File.read(File.join(Smith.root, 'spec/resource', file_name)))
  end

end
