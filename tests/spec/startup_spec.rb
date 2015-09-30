require 'spec_helper'

describe 'firmware' do
  scenario 'startup' do
    start_smith

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
  end
end
