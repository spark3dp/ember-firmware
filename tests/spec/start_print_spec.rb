require 'spec_helper'

require 'support/settings'
require 'support/smith'
require 'support/definitions'
require 'support/motor_controller'
require 'support/front_panel'
require 'support/projector'

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

    let!(:front_panel) {
      FrontPanel.new(
        tmp_dir(FRONT_PANEL_I2C_WRITE_PIPE),
        tmp_dir(FRONT_PANEL_I2C_READ_PIPE),
        tmp_dir(FRONT_PANEL_INTERRUPT_READ_PIPE)
      )
    }

    let(:projector) { Projector.new(tmp_dir(FRAME_BUFFER_IMAGE)) }
    let(:settings) { Settings.new("#{ROOT_DIR}#{SETTINGS_FILE}") }

    #before(:each) { front_panel.enable_logging }
    after(:each) { smith.stop }

    scenario 'start_print' do

      #Note: valid print data must be available for this test to pass!
      smith.start(tmp_dir_path)

      expect(front_panel).to have_led_ring_brightnesses(Array.new(FrontPanel::LED_COUNT, 0))
      expect(front_panel).to have_led_ring_animation_sequence(0)

      expect(front_panel).to show_text({ text: 'Homing the build',   x_position: 64, y_position: 32, alignment: :center, color: 0xFFFF, size: 1 },
                                       { text: 'head. Please wait.', x_position: 64, y_position: 48, alignment: :center, color: 0xFFFF, size: 1 })

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

      expect(front_panel).to show_text({ text: 'Ready.',            x_position: 64,  y_position: 16,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Load your prepped', x_position: 64,  y_position: 32,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'print file via',    x_position: 64,  y_position: 48,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'network or USB.',   x_position: 64,  y_position: 64,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Clear',             x_position: 0,   y_position: 96,  alignment: :left,   color: 0xFFFF, size: 1},
                                       { text: 'file',              x_position: 0,   y_position: 112, alignment: :left,   color: 0xFFFF, size: 1},
                                       { text: 'Reprint',           x_position: 127, y_position: 96,  alignment: :right,  color: 0xFFFF, size: 1},
                                       { text: 'file',              x_position: 127, y_position: 112, alignment: :right,  color: 0xFFFF, size: 1})

      expect(front_panel).to have_led_ring_brightnesses(Array.new(FrontPanel::LED_COUNT, 0))
      expect(front_panel).to have_led_ring_animation_sequence(0)

      expect(projector).to be_showing_black

      front_panel.button_action(BTN2_PRESS)

      # home again first
      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:r)
      expect(movement.action).to                        eq(MC_HOME)
      expect(movement.parameter).to                     eq(UNITS_PER_REVOLUTION)
      expect(movement.settings.units_per_revolution).to eq(settings.get(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR)
      expect(movement.settings.max_jerk).to             eq(settings.get(R_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(R_HOMING_SPEED) * R_SPEED_FACTOR)

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:r)
      expect(movement.action).to                        eq(MC_MOVE)
      expect(movement.parameter).to                     eq(settings.get(R_HOMING_ANGLE) / R_SCALE_FACTOR)
      expect(movement.settings.units_per_revolution).to eq(settings.get(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR)
      expect(movement.settings.max_jerk).to             eq(settings.get(R_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(R_HOMING_SPEED) * R_SPEED_FACTOR)

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:z)
      expect(movement.action).to                        eq(MC_HOME)
      expect(movement.parameter).to                     eq(-2 * settings.get(Z_START_PRINT_POSITION))
      expect(movement.settings.units_per_revolution).to eq(settings.get(Z_MICRONS_PER_REV))
      expect(movement.settings.max_jerk).to             eq(settings.get(Z_HOMING_JERK))
      expect(movement.settings.speed).to                eq(settings.get(Z_HOMING_SPEED) * Z_SPEED_FACTOR)

      # move to start position
      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:r)
      expect(movement.action).to                        eq(MC_MOVE)
      expect(movement.parameter).to                     eq(settings.get(R_START_PRINT_ANGLE) / R_SCALE_FACTOR)
      expect(movement.settings.units_per_revolution).to eq(settings.get(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR)
      expect(movement.settings.max_jerk).to             eq(settings.get(R_START_PRINT_JERK))
      expect(movement.settings.speed).to                eq(settings.get(R_START_PRINT_SPEED) * R_SPEED_FACTOR)

      movement = motor_controller.next_movement
      expect(movement.axis).to                          eq(:z)
      expect(movement.action).to                        eq(MC_MOVE)
      expect(movement.parameter).to                     eq(settings.get(Z_START_PRINT_POSITION))
      expect(movement.settings.units_per_revolution).to eq(settings.get(Z_MICRONS_PER_REV))
      expect(movement.settings.max_jerk).to             eq(settings.get(Z_START_PRINT_JERK))
      expect(movement.settings.speed).to                eq(settings.get(Z_START_PRINT_SPEED) * Z_SPEED_FACTOR)

      #moving to start position screen
      expect(front_panel).to show_text({ text: 'Starting to print',            x_position: 64,  y_position: 32,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: settings.get(JOB_NAME_SETTING), x_position: 64,  y_position: 48,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Skip calibration?',            x_position: 64,  y_position: 80,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Yes',                          x_position: 127, y_position: 112, alignment: :right,  color: 0xFFFF, size: 1})

      expect(front_panel).to have_led_ring_brightnesses(Array.new(FrontPanel::LED_COUNT, 0))
      expect(front_panel).to have_led_ring_animation_sequence(0)

      motor_controller.respond_to_interrupt_request

      #calibration screen
      expect(front_panel).to show_text({ text: 'Complete calibration', x_position: 64,  y_position: 16,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'procedure.',            x_position: 64,  y_position: 32,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Done',                 x_position: 127, y_position: 112, alignment: :right,  color: 0xFFFF, size: 1})

      expect(front_panel).to have_led_ring_brightnesses(Array.new(FrontPanel::LED_COUNT, 0))
      expect(front_panel).to have_led_ring_animation_sequence(0)

    end
  end

end
