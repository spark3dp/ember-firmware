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

    scenario 'start print' do

      #Note: valid print data must be available for this test to pass!
      smith.start(tmp_dir_path)

      expect(front_panel).to have_led_ring_brightnesses(Array.new(FrontPanel::LED_COUNT, 0))
      expect(front_panel).to have_led_ring_animation_sequence(0)

      expect(front_panel).to show_text({ text: 'Homing the build',   x_position: 64, y_position: 32, alignment: :center, color: 0xFFFF, size: 1 },
                                       { text: 'head. Please wait.', x_position: 64, y_position: 48, alignment: :center, color: 0xFFFF, size: 1 })

      expect(motor_controller).to be_enabled

      expect_motor_controller_to_go_home

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

      expect_motor_controller_to_go_home

      expect_motor_controller_to_go_to_start_position

      #moving to start position screen
      expect(front_panel).to show_text({ text: 'Starting to print',            x_position: 64,  y_position: 32,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: settings.get(JOB_NAME_SETTING), x_position: 64,  y_position: 48,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Skip calibration?',            x_position: 64,  y_position: 80,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Yes',                          x_position: 127, y_position: 112, alignment: :right,  color: 0xFFFF, size: 1},
                                       { text: 'Cancel',                       x_position: 0,   y_position: 96,  alignment: :left,   color: 0xFFFF, size: 1},
                                       { text: 'print',                        x_position: 0,   y_position: 112, alignment: :left,   color: 0xFFFF, size: 1})

      expect(front_panel).to have_led_ring_brightnesses(Array.new(FrontPanel::LED_COUNT, 0))
      expect(front_panel).to have_led_ring_animation_sequence(0)

      motor_controller.respond_to_interrupt_request

      #calibration screen
      expect(front_panel).to show_text({ text: 'Complete calibration', x_position: 64,  y_position: 16,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'procedure.',           x_position: 64,  y_position: 32,  alignment: :center, color: 0xFFFF, size: 1},
                                       { text: 'Done',                 x_position: 127, y_position: 112, alignment: :right,  color: 0xFFFF, size: 1},
                                       { text: 'Cancel',               x_position: 0,   y_position: 96,  alignment: :left,   color: 0xFFFF, size: 1},
                                       { text: 'print',                x_position: 0,   y_position: 112, alignment: :left,   color: 0xFFFF, size: 1})

      expect(front_panel).to have_led_ring_brightnesses(Array.new(FrontPanel::LED_COUNT, 0))
      expect(front_panel).to have_led_ring_animation_sequence(0)

    end
  end

end
