void Mode_Pattern()
{

    // Read the button states from the shift register.
    unsigned int reading = SR.Button_Step_Read();

    // If the readings have changed, remember current time.
    if (reading != old_step_button_state) {
        millis_debounce_step_button = millis();
    }

    // The current reading has been the same for longer than DEBOUNCE time.
    if ((millis() - millis_debounce_step_button) >= DEBOUNCE) {
        // Check if the button states have changed.
        if (reading != step_button_state) {
            step_button_state = reading;

            // Loop over the 16 step buttons
            for (byte i = 0; i < 16; i++) {
                step_button_just_pressed[i] = false; // Initialize each step pressed value.
                step_button_current_state[i] = bitRead(reading, i); // Assign the new value.

                if (step_button_current_state[i] != step_button_previous_state[i]) {
                    if ((step_button_pressed[i] == LOW) && (step_button_current_state[i] == HIGH)) {
                        step_button_just_pressed[i] = true;
                    }

                    step_button_pressed[i] = step_button_current_state[i];
                }

                step_button_previous_state[i] = step_button_current_state[i];
            }

            //Mode pattern Play
            //==============================================================================================================
            if (PATTERN_PLAY_MODE) {

                //MUTE MODE----------------------------------------------------------------------------------------------------
                if (mute_mode) {
                    for (byte i = 0; i < 16; i++) {
                        if (step_button_just_pressed[i]) {
                            if (solo_mode) {
                                // mute all the instruments except the one pressed with "SOLO EXCLUSIVE"
                                inst_mute = (B11111111 & (B11111111 << 8)) | ~(1 << i); //on mute tous les intrus sauf celui appuyer SOLO EXCLUSIF

                                for (byte ct = 0; ct < 16; ct++) { //on reinitialise les compteur des instrument smuter a 1 pour pouvoir les demuter en reappuyant dessus
                                    step_button_count[ct] = 1;
                                }

                                step_button_count[i] = 0;
                            }
                            else {
                                step_button_count[i]++;
                            }

                            switch (step_button_count[i]) {
                            case 1:
                                bitSet(inst_mute, i);
                                break;

                            case 2:
                                bitClear(inst_mute, i);
                                step_button_count[i] = 0;
                                break;
                            }
                        }
                    }
                }

                //SOLO MODE----------------------------------------------------------------------------------------------------
                /*  if(solo_mode){
                 inst_mute = ~step_button_state;
                 }*/
                //ROLL MODE----------------------------------------------------------------------------------------------------
                else if (roll_mode) {
                    inst_roll = step_button_state;

                    if ((pattern_scale[pattern_buffer] % 16) == 0) {
                        scale_type = 1;    //permet de savoir si le roulement doit etre en ternaire ou binaire suivant la scale du pattern
                    }
                    else {
                        scale_type = 0;    //0 = binaire scale   1= ternaire scale
                    }
                }
                //NORMAL MODE----------------------------------------------------------------------------------------------------
                else {
                    // Bank Select
                    if (button_shift) {
                        for (byte i = 0; i < 16; i++) { // Loop for all step buttons.
                            if (bitRead(step_button_state, i)) {
                                // if the shift button is pressed, we retun to the selected bank
                                // si le bouton shift est appuyer on retourne la bank selectionner selectionner
                                pattern_bank = i;
                                selected_pattern_changed = 1; //flag que le pattern selectionner a change
                                break;
                            }
                        }
                    }
                    // Pattern Select
                    else {

                        // Loop for all step buttons.
                        for (byte i = 0; i < 16; i++) {
                            if (step_button_just_pressed[i]) { // If a button is pressed.
                                pushed_button_step_count++; // Increment the counter of the number of pushed buttons.

                                if (pushed_button_step_count == 1) { // If it is the first pushed button.
                                    first_selected_pattern = i; // We register it as the first selected pattern.
                                    selected_pattern_changed = 1; // Flag that the selected pattern has changed.
                                    last_selected_pattern = 255; // We initialize the second selected pattern.
                                }
                                else if (pushed_button_step_count == 2) { // If a second button is pressed
                                    last_selected_pattern = i; // We register it as the 2nd selected pattern.
                                    pushed_button_step_count = 0; // We initialize the counter of the number of pushed buttons.
                                    break; // Break out of the for loop.
                                }
                            }
                        }

                        // Allows the variable "selected_pattern" to always store the smallest selected pattern.
                        if ((first_selected_pattern > last_selected_pattern) && (last_selected_pattern != 255)) {
                            low_selected_pattern = last_selected_pattern;
                            selected_pattern_changed = 1; // Flag that the selected pattern has changed.
                        }
                        else { //sinon il est egal au premier
                            low_selected_pattern = first_selected_pattern;
                            selected_pattern_changed = 1; // Flag that the selected pattern has changed.
                        }

                        selected_pattern = low_selected_pattern;

                        // If there is no 2nd pattern selected, there is no block.
                        if (last_selected_pattern == 255) { //si il n'y a pas de dernier pattern selectionner
                            nbr_pattern_block = 0; //0 pattern dans le block
                        }
                        else { //sinon
                            nbr_pattern_block = abs(last_selected_pattern - first_selected_pattern); //le difference du premier et du dernier pattern donne le nombre de pattern dans le block
                            nbr_pattern_block_changed = 1; //flag que le block a change sert pour reinitialiser en stop

                            if (play) {
                                nbr_pattern_block_changed_A = 1; //flag que le block a change sert dans l'interuption
                            }
                        }
                    }

                    //pattern_nbr = selected_pattern+(16*pattern_bank);//le numero du pattern est egal au pattern selectionner plus 16 fois la bank soit
                    //255 pattern

                }
            }

            //Mode Pattern Edit
            //==============================================================================================================
            else if (selected_mode == PATTERN_EDIT) {
                if (play) { //si seq en run
                    if (button_pattern_part_pressed) { // If we are pressing the pattern part 0-16/17-32 buttons.
                        // loop as many times as the step button is 16
                        for (byte i = 0; i < 16; i++) { //loop autant de fois que de bouton step soit 16
                            if (bitRead(step_button_state, i)) {
                                // if the shift button is pressed, we return the value of number of steps
                                // si le bouton shift est appuyer on retourne la valeur du nombre de step
                                nbr_step[pattern_buffer] = (i + (16 * button_pattern_part)) + 1;
                                // the number of steps of the pattern changed
                                nbr_step_changed = 1; //le nombre de pas du pattern a change
                            }
                        }
                    }
                    // if the pattern parts buttons not pressed, we return the value of buttons in the edited pattern
                    else {
                        // flag for if the pattern was changed since last save
                        selected_pattern_edited = 1;
                        // flag for if the pattern was changed and if saved
                        selected_pattern_edited_saved = 1;
                        // the pattern should only change on a button's state change from
                        // "not pressed" to "pressed." other cases must be ignored.
                        pattern[pattern_buffer][selected_inst][button_pattern_part] ^= ((step_button_state ^ last_step_button_state) & step_button_state);
                        last_step_button_state = step_button_state;
                    }

                    // Real-time clearing of steps with the encoder button.
                    if (button_encoder) {
                        if (button_shift) {
                            // Clear the full row of steps for instrument.
                            pattern[pattern_buffer][selected_inst][0] = 0;
                            pattern[pattern_buffer][selected_inst][1] = 0;
                        }
                        else {
                            // Clear only the steps that are playing.
                            if (step_count < 16) {
                                bitClear(pattern[pattern_buffer][selected_inst][0], step_count);
                            }
                            else if (step_count >= 16) {
                                bitClear(pattern[pattern_buffer][selected_inst][1], step_count - 16);
                            }
                        }

                        selected_pattern_edited = 1; //flag que le pattern a ete editer depuis la derniere sauvegarde
                        selected_pattern_edited_saved = 1; //flag que le pattern a ete editer depuis la derniere sauvegarde
                    }

                    // TAP step entry.
                    if (button_shift && (last_button_shift != button_shift)) {

                        // Quantising strategy:
                        // TL;DR: If a tap is "late", we register it on the next step.
                        //
                        // What defines "late"? We have to consider the 4 different scales..
                        // By careful probing of a TR-606 (which also has tap mode) it seems Roland did the following:
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        // Scale  |  PPQN  |  Pulses/Step  |  Amount of pulses not "late"
                        // 1/16   |  24    |  6            |  3
                        // 1/32   |  12    |  3            |  1
                        // 1/8t   |  32    |  8            |  4
                        // 1/16t  |  16    |  4            |  2
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        // The code below "scales" the current ppqn count down to Pulses/step,
                        // And checks if the result is smaller than the amount of pulses not "late".
                        // The maths below for pulses not "late" seem to work for all cases.
                        // There most likely is a more elegant way.
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        // Check if we can place the note on the current step:
                        if (clock24_count % (pattern_scale[pattern_buffer] / 4) < (int)(pattern_scale[pattern_buffer] / 8)) {
                            if (step_count < 16) {
                                bitSet(pattern[pattern_buffer][selected_inst][0], step_count);
                            }
                            else {
                                bitSet(pattern[pattern_buffer][selected_inst][1], step_count - 16);
                            }
                        }
                        else { // We need to push the note to the next step.
                            if (step_count < 15) {
                                bitSet(pattern[pattern_buffer][selected_inst][0], step_count + 1);
                            }
                            else if (step_count + 1 == nbr_step[pattern_buffer]) { // Last step
                                bitSet(pattern[pattern_buffer][selected_inst][0], 0);
                            }
                            else {
                                bitSet(pattern[pattern_buffer][selected_inst][1], step_count - 15);
                            }
                        }

                        // Some magic to play the actual sound.
                        // Seems logical not to  play the actual sound in the case it is pushed
                        // to the next step, as it will play properly already by default once the step starts.
                        // However in practice this usually does not work. Double triggers are better than no
                        // triggers at all, so we go for the possible double trigger for now:
                        SR.Inst_Send(1 << selected_inst);
                        /*
                        Set_CPU_Trig_High();
                        delayMicroseconds(10);
                        Set_CPU_Trig_Low();
                        */
                        // Make sure the change is saved.
                        selected_pattern_edited = 1;
                        selected_pattern_edited_saved = 1;
                    }

                    last_button_shift = button_shift; // Store prev shift value.



                }
                else if (!play) { //si pas en play
                    if (button_shift) { //si bouton shift appuyer
                        for (byte i = 0; i < 16; i++) { //loop autant de fois que de bouton step soit 16
                            if (bitRead(step_button_state, i)) {
                                // if the shift button is pressed, we return the chosen bank
                                // si le bouton shift est appuyer on retourne la bank selectionner selectionner
                                pattern_bank = i;
                                break;
                            }
                        }
                    }
                    else {
                        for (byte i = 0; i < 16; i++) { //loop autant de fois que de bouton step soit 16
                            if (bitRead(step_button_state, i)) {
                                // we return the value of the chosen pattern
                                //on retourne la valeur du pattern selectionner
                                selected_pattern = i;
                                break;
                            }
                        }
                    }

                    //pattern_nbr = selected_pattern+(16*pattern_bank);//le numero du pattern est egal au pattern selectionner plus 16 fois la bank soit
                    //255 pattern
                    //Serial.println(pattern_nbr,DEC);
                }
            }

            // Pattern Chain selection
            // function that makes the chosen pattern advance when a chain (block) is selected.
            // a block is chosen and we have advanced in the measure
            if ((nbr_pattern_block != 0) && (middle_mesure_flag)) { //un block est se'lectionner et on a avnacer dans la mesure
                // the selected pattern equals the first pattern + the count pattern that incremented in the interrup
                selected_pattern = low_selected_pattern + pattern_count; //le pattern selectionner egal le premier pattern + le coount pattern qui incrementer dans l'interuption
                selected_pattern_changed = 1; //flag que le pattern selectionner a change permet de verticalize
                middle_mesure_flag = 0; //reset flag que la mesure a avancé
            }

            if (!play) {
                nbr_pattern_block_changed_A = 0; //flag que le block a change

                if (nbr_pattern_block_changed) {
                    nbr_pattern_block_changed = 0;
                    middle_mesure_flag = 0;
                    selected_pattern = low_selected_pattern;
                    selected_pattern_changed = 1; //flag que le pattern selectionner a change
                }
            }

            pattern_nbr = selected_pattern + (16 * pattern_bank); //le numero du pattern est egal au pattern selectionner plus 16 fois la bank soit 255 pattern

        }
    }
    // if we have released (unpushed) all the buttons we initialize the counter of the number of pushed buttons.
    else if (step_button_state == 0) {
        pushed_button_step_count = 0;
    }

    //Unmute all
    if (button_encoder && PATTERN_PLAY_MODE && mute_mode) {
        inst_mute = 0;

        for (byte i = 0; i < 16; i++) {
            step_button_count[i] = 0;
        }
    }

    old_step_button_state = reading;
}
