module ConfigHelper

  def stub_iwlist_scan(file_name)
    allow(Smith::Config::WirelessInterface).to receive(:site_survey).and_return(File.read(File.join(Smith.root, 'spec/resource', file_name)))
  end

end
