void Update_Pattern_Led()
{

    if (PATTERN_PLAY_MODE) {

        unsigned int temp_led_block = 0; // Leds that will be on on the next block.
        unsigned int temp_led_pattern = 0; // Leds that will be on the next pattern.

        // Update the pattern part LEDs.
        if (step_count <= 16 || step_count == 255) {
            button_pattern_part = 0;
        }
        else if (step_count > 16) {
            button_pattern_part = 1;
        }

        // Update the scale LED with the pattern's scale.
        switch (pattern_scale[pattern_buffer]) {
        case 12:
            button_scale_count = 0;
            break;

        case 24:
            button_scale_count = 1;
            break;

        case 16:
            button_scale_count = 2;
            break;

        case 32:
            button_scale_count = 3;
            break;
        }

        // Calculate which LEDs are on between the two chosen patterns.
        // 1st case: the first pattern is smaller than the last pattern.
        if (first_selected_pattern < last_selected_pattern) {
            temp_led_block = (((1 << last_selected_pattern) - 1) | (1 << last_selected_pattern));
            temp_led_pattern = ((1 << first_selected_pattern) - 1);
        }
        // 2nd case: the first pattern is bigger than the last pattern
        else if (first_selected_pattern > last_selected_pattern) { //2e cas le premier patern est plsu grand que le dernier
            temp_led_block = ((1 << last_selected_pattern) - 1);
            temp_led_pattern = (((1 << first_selected_pattern) - 1) | (1 << first_selected_pattern));
        }

        // Update LEDs for play mode.
        if (play) {

            // Make the MUTE LED blink while in mute mode.
            if (mute_mode) {
                step_leds = (inst_step_buffer[step_count][pattern_buffer] * tempo_led_flag) | muted_instruments;
                PORTC = (1 * !tempo_led_flag) << 7;
            }
            // Make the ROLL LED blink while in roll mode.
            else if (roll_mode) {
                // Blink the roll led in roll mode.
                step_leds = (inst_step_buffer[step_count][pattern_buffer] * tempo_led_flag) | inst_roll;
                PORTC = (1 * !tempo_led_flag) << 6;
            }
            // Bank selection mode.
            else if (button_shift) {
                // Bank selection.
                step_leds = (1 << pattern_bank ^ tempo_led_flag << (step_count)); // Show the current selected bank.
                PORTC = B11111100; // Light up all non-step leds to show we are in bank selection.
            }
            // Normal mode.
            else {
                // If only a single pattern is selected, blink it and animate running lights.
                if (last_selected_pattern == 255) {
                    step_leds = (1 << selected_pattern ^ tempo_led_flag << (step_count));
                }
                // If multiple patterns are selected, light up their leds too.
                else {
                    step_leds = (((1 * tempo_led_flag_block) << selected_pattern) ^ (tempo_led_flag << ((step_count) % 16)) ^ (temp_led_block ^ temp_led_pattern));
                }
                PORTC = (1 << (button_scale_count + 2)) + (1 << (!button_pattern_part + 6));
            }
        }
        // Sequencer is not playing.
        else {

            // Make the MUTE LED blink while in mute mode.
            if (mute_mode) {
                step_leds = muted_instruments;
                PORTC = (1 * !tempo_led_flag) << 7;
            }
            // Make the ROLL LED blink while in roll mode.
            else if (roll_mode) {
                step_leds = inst_roll;
                PORTC = (1 * !tempo_led_flag) << 6;
            }
            // Bank selection mode.
            else if (button_shift) {
                step_leds = (1 << pattern_bank); // Show the current selected bank.
                PORTC = B11111100; // Light up all non-step leds to show we are in bank selection.
            }
            else {
                // If only a single pattern is selected, blink it.
                if (last_selected_pattern == 255) {
                    step_leds = ((1 * tempo_led_flag << selected_pattern));
                }
                // If multiple patterns are selected, light up their leds too.
                else {
                    step_leds = ((1 * tempo_led_flag << selected_pattern) ^ (temp_led_block ^ temp_led_pattern));
                }
                PORTC = (1 << button_scale_count + 2) + (1 << !button_pattern_part + 6);
            }
        }

    }

    else if (selected_mode == PATTERN_EDIT) {

        // Udate the scale LED with the pattern's scale.
        switch (pattern_scale[pattern_buffer]) {
        case 12:
            button_scale_count = 0;
            break;

        case 24:
            button_scale_count = 1;
            break;

        case 16:
            button_scale_count = 2;
            break;

        case 32:
            button_scale_count = 3;
            break;
        }

        if ((nbr_step[pattern_buffer] > 16) && button_part_switch) {
            if (step_count > 16 && button_pattern_part == 0) {
                button_pattern_part = 1;
            }
            else if (step_count <= 16 && button_pattern_part == 1) {
                button_pattern_part = 0;
            }
        }

        // Update the LEDs from the edited part following the buttons.
        PORTC = (1 << button_scale_count + 2) + (1 << !button_pattern_part + 6);

        if (play) {
            if (button_pattern_part == 0) { //si la parti 1 a 16 est selectionner
                step_leds = (pattern[pattern_buffer][selected_inst][0] ^ tempo_led_flag << (step_count));
            }
            else if (step_count > 16 && button_pattern_part == 1) { //si la parti 17 a 32 est selectionner et que le compteur step est superieur ou egale a 16
                step_leds = (pattern[pattern_buffer][selected_inst][1] ^ tempo_led_flag << (step_count) - 16);
            }
            else if (step_count <= 16 & button_pattern_part == 1) { //si la parti 17 a 32 est selectionner et que le compteur step est inferieur  a 16
                step_leds = (pattern[pattern_buffer][selected_inst][1]);
            }
        }
        else if (!play) {
            if (button_shift) {
                step_leds = (1 << pattern_bank);
                PORTC = B11111100; //allumer toutes leds pour indiquer qu'on selectionne la bank
            }
            else if (!button_shift) {
                step_leds = (1 * tempo_led_flag << (selected_pattern));
            }
        }
    }

}
