require 'spec_helper'
require 'open3'

class SmithExecutable

  def start
    @stdin, @stdout, @stderr, @wait_thr = Open3.popen3('/usr/local/bin/smith')

    # Write the stdout from the dummy server to a file in another thread
    Thread.new do
      Thread.current.abort_on_exception = true
      log = File.open(File.join(File.expand_path('..', __FILE__), 'smith.out'), 'w')
      puts log.path
      IO.copy_stream(@stdout, log)
      IO.copy_stream(@stderr, log)
    end
  end

  def stop
    if @wait_thr && @wait_thr.alive?
      Process.kill('INT', @wait_thr.pid)
      puts 'Timeout attempting to stop smith' unless @wait_thr.join(5)
    end
  end

end


describe 'firmware' do
  let(:smith) { SmithExecutable.new }

  scenario 'startup' do
    smith.start
=begin
    expect(front_panel).to display(
      'Ready.
      Load your prepped
      print file via
      network or USB.'
    )

    expect(r_axis).to move_home_with_max_travel(settings.get(R_HOMING_ANGLE)).with_settings(
      step_angle: settings.get(R_STEP_ANGLE),
      units_per_rev: settings.get(R_MILLIDEGREES_PER_REV),
      microstepping: settings.get(MICRO_STEPS_MODE),
      jerk: settings.get(R_HOMING_JERK),
      speed: settings.get(R_HOMING_SPEED)
    )

    expect(z_axis).to move_home_with_max_travel(-2 * settings.get(Z_START_PRINT_POSITION)).with_settings(
      step_angle: settings.get(Z_STEP_ANGLE),
      units_per_rev: settings.get(Z_MICRONS_PER_REV),
      microstepping: settings.get(MICRO_STEPS_MODE),
      jerk: settings.get(Z_HOMING_JERK),
      speed: settings.get(Z_HOMING_SPEED)
    )


    expect(r_axis).to be_disabled
    expect(z_axis).to be_disabled
=end
    smith.stop
  end
end
