require 'spec_helper'

require 'support/settings'
require 'support/smith'
require 'support/definitions'
require 'support/motor_controller'

module Tests

  describe 'firmware' do
    let(:smith) { Smith.new }
    let!(:motor_controller) {
      MotorController.new(
        tmp_dir(MOTOR_CONTROLLER_I2C_WRITE_PIPE),
        tmp_dir(MOTOR_CONTROLLER_I2C_READ_PIPE),
        tmp_dir(MOTOR_CONTROLLER_INTERRUPT_READ_PIPE)
      )
    }
    let(:settings) { Settings.new("#{ROOT_DIR}#{SETTINGS_SUB_DIR}#{SETTINGS_FILE}") }
    after(:each) { smith.stop }

    scenario 'startup' do

      smith.start(tmp_dir_path)

=begin
      expect(front_panel).to display(
        'Ready.
        Load your prepped
        print file via
        network or USB.'
      )
=end

      expect(motor_controller).to be_enabled

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:r)
      expect(movement.action).to                        eq(MC_HOME)
      expect(movement.parameter).to                     eq(UNITS_PER_REVOLUTION)
      expect(movement.settings.step_angle).to           eq(settings.get(R_STEP_ANGLE))
      expect(movement.settings.units_per_revolution).to eq(settings.get(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR)
      expect(movement.settings.microstepping_factor).to eq(settings.get(MICRO_STEPS_MODE))
      expect(movement.settings.max_jerk).to             eq(settings.get(R_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(R_HOMING_SPEED) * R_SPEED_FACTOR)

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:r)
      expect(movement.action).to                        eq(MC_MOVE)
      expect(movement.parameter).to                     eq(settings.get(R_HOMING_ANGLE) / R_SCALE_FACTOR)
      expect(movement.settings.step_angle).to           eq(settings.get(R_STEP_ANGLE))
      expect(movement.settings.units_per_revolution).to eq(settings.get(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR)
      expect(movement.settings.microstepping_factor).to eq(settings.get(MICRO_STEPS_MODE))
      expect(movement.settings.max_jerk).to             eq(settings.get(R_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(R_HOMING_SPEED) * R_SPEED_FACTOR)

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:z)
      expect(movement.action).to                        eq(MC_HOME)
      expect(movement.parameter).to                     eq(-2 * settings.get(Z_START_PRINT_POSITION))
      expect(movement.settings.step_angle).to           eq(settings.get(Z_STEP_ANGLE))
      expect(movement.settings.units_per_revolution).to eq(settings.get(Z_MICRONS_PER_REV))
      expect(movement.settings.microstepping_factor).to eq(settings.get(MICRO_STEPS_MODE))
      expect(movement.settings.max_jerk).to             eq(settings.get(Z_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(Z_HOMING_SPEED) * Z_SPEED_FACTOR)

      motor_controller.respond_to_interrupt_request

      expect(motor_controller).to be_disabled

      #expect(projector).to show_black
    end
  end

end
