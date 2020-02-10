// Some helpers for readability.
#define BUTTON_PLAY    1
#define BUTTON_SHIFT   2

#define BUTTONS_SHIFT_PLAY 3

#define BUTTON_SCALE   4
#define BUTTON_NEXT    4

#define BUTTON_1732    8 // Button 17-32.
#define BUTTON_ROLL    8
#define BUTTON_END     8

#define BUTTON_0116    16 // Button 01-16.
#define BUTTON_MUTE    16
#define BUTTON_RESET   16

#define BUTTONS_0116_1732  24

#define BUTTON_ENC         32

#define BUTTONS_ENC_SHIFT  34 // Encoder button + shift.


void Check_Slave_Transport_State() {
    if (sync_mode != MASTER) {
        // check status play in DIN_SYNC mode
        boolean din_start_state = PIND & (1 << 5);

        if (din_start_state != old_din_start_state) {
            if (din_start_state) { // DinSync Run signal is HIGH.
                first_play_flag = 1; // Indicate first play.
                play = 1;

            }
            else if (!din_start_state) { // DinSync Run signal is LOW.
                play = 0;
                first_stop_flag = 1;
            }

            old_din_start_state = din_start_state;
        }
    }
}

void Check_Edit_Button_Pattern_Edit()
{
    Check_Slave_Transport_State(); // Decide on play/stop flags if slaved.

    // Read the state of the buttons.
    byte reading = SR.Button_SR_Read(2);

    if (reading != old_edit_button_state) {
        millis_debounce_edit_button = millis();
    }

    if ((millis() - millis_debounce_edit_button) >= DEBOUNCE) {
        if (reading != edit_button_state) {
            edit_button_state = reading;

            switch (edit_button_state) {
            case BUTTON_PLAY:
                if (sync_mode == MASTER) {
                    button_play_count++;

                    if (button_play_count == 1) {
                        play = 1;
                        // allow the first step to play
                        first_play_flag = 1; //permet de jouer le premier pas (cf interrupt)
                    }
                    else if (button_play_count == 2) {
                        play = 0;
                        button_play_count = 0;
                        first_stop_flag = 1;
                    }
                }
                break;

            case BUTTON_SCALE:
                pattern_scale_changed_flag = true;
                button_scale_count++;

                if (button_scale_count == 4) {
                    button_scale_count = 0;
                }

                switch (button_scale_count) {
                case 0:
                    pattern_scale[pattern_buffer] = 12; // 1/32 time.
                    break;

                case 1:
                    pattern_scale[pattern_buffer] = 24; // 1/16 time.
                    break;

                case 2:
                    pattern_scale[pattern_buffer] = 16; // 1/16t time.
                    break;

                case 3:
                    pattern_scale[pattern_buffer] = 32; // 1/8t time.
                    break;
                }
                break;

            // check which part is selected - either 1-16 or 17-32
            case BUTTON_1732: // 17..32 button.
                button_pattern_part = 1;
                break;

            case BUTTON_0116: // 1..16 button.
                button_pattern_part = 0;
                break;
            }

            // Both 1..16 and 17..32 buttons are pressed.
            if (edit_button_state == BUTTONS_0116_1732) {
                button_part_switch = !button_part_switch;
            }

            // Shift button.
            //button_shift = (edit_button_state == BUTTON_SHIFT) || (edit_button_state == BUTTONS_ENC_SHIFT);
            // Encoder button.
            //button_encoder = (edit_button_state == BUTTON_ENC) || (edit_button_state == BUTTONS_ENC_SHIFT);
            // Shift button.
            if (edit_button_state == 2 || edit_button_state == 34) {
                button_shift = 1;
            }
            else {
                button_shift = 0;
            }

            // Encoder button.
            if (edit_button_state == 32 || edit_button_state == 34) {
                button_encoder = 1;
            }
            else {
                button_encoder = 0;
            }
            // Check if the pattern part is pressed.
            if ((edit_button_state == BUTTON_1732) || (edit_button_state == BUTTON_0116) || (edit_button_state == BUTTONS_0116_1732)) {
                button_pattern_part_pressed = 1;
            }
            else {
                button_pattern_part_pressed = 0;
            }
        }
    }

    // Save the state of the buttons.
    old_edit_button_state = reading;
}

//=============================================================================
void Check_Edit_Button_Pattern()
{
    Check_Slave_Transport_State(); // Decide on play/stop flags if slaved.

    // Read the state of the buttons.
    byte reading = SR.Button_SR_Read(2);

    if (reading != old_edit_button_state) {
        millis_debounce_edit_button = millis();
    }

    if ((millis() - millis_debounce_edit_button) >= DEBOUNCE) {
        if (reading != edit_button_state) {
            edit_button_state = reading;

            // First check the buttons that permanently change state.
            switch (edit_button_state) {
            case BUTTON_PLAY:
                if (sync_mode == MASTER) {
                    button_play_count++;

                    if (button_play_count == 1) {
                        play = 1;
                        first_play_flag = 1; //permet de jouer le premier pas (cf interrupt)
                    }
                    else if (button_play_count == 2) {
                        play = 0;
                        button_play_count = 0;
                        first_stop_flag = 1;
                    }
                }

                break;

            case BUTTON_SCALE:
                break;

            case BUTTON_ROLL:
                mute_mode = false;
                roll_mode = true;
                break;

            case BUTTON_MUTE:
                if (mute_mode) { // Special case, if already in mute mode, enter solo mode.
                    solo_mode = 0;
                }

                mute_mode = true;
                roll_mode = false;
                break;
            }

            // Next check momentary buttons.
            button_shift = (edit_button_state == BUTTON_SHIFT);
            button_encoder = (edit_button_state == BUTTON_ENC);

            // If shift button is pressed, we exit the mute or roll mode.
            if (button_shift && (mute_mode || roll_mode)) {
                mute_mode = roll_mode = 0;
            }
        }
    }

    // Save the state of the buttons.
    old_edit_button_state = reading;
}

//==================================================================
void Check_Edit_Button_Setup()
{
    // Read the state of the buttons.
    byte reading = SR.Button_SR_Read(2);

    if (reading != old_edit_button_state) {
        millis_debounce_edit_button = millis();
    }

    if ((millis() - millis_debounce_edit_button) >= DEBOUNCE) {
        if (reading != edit_button_state) {
            edit_button_state = reading;

            // Play button.
            button_play = (edit_button_state == BUTTON_PLAY);
            // Shift button.
            button_shift = (edit_button_state == BUTTON_SHIFT);
            // Init button.
            button_init = (edit_button_state == BUTTONS_SHIFT_PLAY);
            // Reset button.
            button_reset = (edit_button_state == BUTTON_RESET);
        }
    }

    // Save the state of the buttons.
    old_edit_button_state = reading;
}

void Check_Edit_Button_Song()
{
    Check_Slave_Transport_State(); // Decide on play/stop flags if slaved.

    // Read the state of the buttons.
    byte reading = SR.Button_SR_Read(2);

    if (reading != old_edit_button_state) {
        millis_debounce_edit_button = millis();
    }

    if ((millis() - millis_debounce_edit_button) >= DEBOUNCE) {
        if (reading != edit_button_state) {
            edit_button_state = reading;

            //---------------------------------------------------------------------------------
            if (selected_mode == SONG_MIDI_MASTER || selected_mode == SONG_EDIT) { //le mode selectionne est il SONG_MIDI_MASTER
                switch (edit_button_state) {
                case BUTTON_PLAY:
                    button_play_count++;

                    if (button_play_count == 1) {
                        play = 1;
                        first_play_flag = 1; //permet de jouer le premier pas (cf interrupt)
                    }
                    else if (button_play_count == 2) {
                        play = 0;
                        button_play_count = 0;
                        first_stop_flag = 1;
                    }

                    break;
                }
            }

            // Shift button.
            button_shift = (edit_button_state == BUTTON_SHIFT);
            // Reset button.
            button_reset = (edit_button_state == BUTTON_RESET);
            // Encoder button.
            button_encoder = (edit_button_state == BUTTON_ENC);

            // Next button.
            if (edit_button_state == BUTTON_NEXT) {
                button_next = 1;
            }
            else {
                button_next = 0;
                first_push_next_flag = false;
            }

            // End button
            if (edit_button_state == BUTTON_END) {
                button_end = 1;
            }
            else {
                button_end = 0;
                first_push_end_flag = false;
            }
        }
    }

    // Save the state of the buttons.
    old_edit_button_state = reading;
}
