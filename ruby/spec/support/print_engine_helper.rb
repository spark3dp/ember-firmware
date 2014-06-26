module PrintEngineHelper

  def printer_status(vars)
    {
      :PrinterStatus =>
      {
        :State => vars[:state],
        :Change => 'entering',
        :IsError => 'false',
        :ErrorCode => '29',
        :Error => "Can't get setting: ModelExposureSec: Illegal seek",
        :Layer => '0',
        :TotalLayers => '0',
        :SecondsLeft => '6421',
        :JobName => 'slice',
        :Temperature => '0'
      }
    }
  end

end
