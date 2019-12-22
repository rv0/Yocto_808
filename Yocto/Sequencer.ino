void Sequencer_Tick()
{
    // Actions to take when playing the first time
    if (first_play) {
        // Reset all counters.
        first_play = step_count = clock24_count = clock96_count = pattern_count = tempo_led_count = 0;

        // Send sync start.
        if (sync_mode != MIDI_SLAVE) {
            MIDI_Send(0xfa); // Send Midi Start.
        }
        if (sync_mode != DIN_SLAVE) {
            Set_Dinsync_Run_High();
        }
    }

    // Increment counters
    if (sync_mode == MASTER) { // For now, we have master running at 96PPQN, but we don't use it yet.
        if (clock96_count == 96) {
            clock96_count = 0;
        }
        if (clock96_count % 4 == 0) {
            clock24_count++;
            ticked = 1;
        }
        clock96_count++;
    }
    else {
        if (clock24_count == 24) {
            clock24_count = 0;
        }
        clock24_count++;
        ticked = 1;
    }

    // We have a clock tick.
    if (ticked) { // Temp, we want to have everything 24PPQN for now.
        ticked = 0;

        // Send clock to slaves/
        if (sync_mode != DIN_SLAVE) {
            Set_Dinsync_Clock_High(); // TODO: Magic
        }
        if (sync_mode != MIDI_SLAVE) {
            MIDI_Send(0xf8); // TODO: Magic
        }


        Set_CPU_Trig_Low();
        Reset_Trig_Out();

        Update_Pattern_EEprom(); // Is the another pattern selected?
        Update_Pattern_Led(); // Update the LEDs

        tempo_led_count++;

        // Sequencer is playing.
        if (play) {

            // Update the flashing of the LEDs.
            if (tempo_led_count >= 12) {
                tempo_led_count = 0;
                tempo_led_flag_block = !tempo_led_flag_block;
            }

            // We alternate the flag of the tempo LED.
            if (clock24_count >= 3) {
                tempo_led_flag = false;
            }
            else {
                tempo_led_flag = true;
            }

            // Are we at the start of a step? (clock24_count goes from 1 till 24).
            if ((clock24_count % (pattern_scale[pattern_buffer] / 4)) == 1) {
                step_count++;

                if (step_count >= nbr_step[pattern_buffer]) { // The pattern has ended.
                    step_count = 0; // Reset the step counter.
                    end_measure_flag = true; // The measure has ended.
                    //------------------------------------------------ WTF is this???????????????
                    if (load_pattern_ok_flag) { // If the pattern loaded correctly.
                        pattern_buffer = !pattern_buffer; //on switche entre les deux pattern present dans le buffer au debut de la mesure
                        load_pattern_ok_flag = 0; //reinitialise le flag de load pattern
                    }
                }
                else if (step_count == (nbr_step[pattern_buffer] - 1)) {
                    middle_mesure_flag = true;

                    //MODE PATTERN PLAY--------------------------------------------------------------------------------------------------------
                    if (PATTERN_PLAY_MODE) {
                        pattern_count++; // Count when a block of patterns is selected.
                        if (pattern_count > nbr_pattern_block) { //on reset le comteur quand il est superieur au nombre de pattern a lire dans le block
                            pattern_count = 0;
                        }
                        if (nbr_pattern_block_changed_A) {
                            nbr_pattern_block_changed_A = 0;
                            pattern_count = 0;
                        }
                    }

                    //MODE SONG PLAY--------------------------------------------------------------------------------------------------------
                    else if (SONG_PLAY_MODE) {
                        pattern_count++; // Count for advancing the pattern.
                        if (pattern_count >= total_pattern_song[song_buffer]) { //on reset le comteur quand il est superieur au nombre de pattern dans le song
                            pattern_count = 0;
                        }
                    }
                }

                SR.ShiftOut_Update(temp_step_led, ((inst_step_buffer[step_count][pattern_buffer]) & (~inst_mute) | inst_roll));
                Send_Trig_Out((inst_step_buffer[step_count][pattern_buffer]) & (~inst_mute) | inst_roll);
            }
        }

        // Sequencer is stopped.
        else if (!play) {

            // Update the flashing of the LEDs.
            if (tempo_led_count >= 12) {
                tempo_led_count = 0;
                tempo_led_flag = !tempo_led_flag;
            }

            if (first_stop) {
                first_stop = false;
                step_count = 0;
                if (sync_mode != MIDI_SLAVE) {
                    MIDI_Send(0xfc); //Midi Stop
                }
                if (sync_mode != DIN_SLAVE) {
                    Set_Dinsync_Run_Low();
                }
            }

            // Initialize pattern counter if needed.
            if (PATTERN_PLAY_MODE || (SONG_PLAY_MODE && (button_reset))) {
                pattern_count = 0;
            }
        }

        // Roll mode.
        if (roll_mode && (clock24_count % roll_scale[scale_type][roll_pointer] == 0) && inst_roll > 0) { // @TODO
            SR.ShiftOut_Update(temp_step_led, inst_roll);
            Send_Trig_Out(inst_roll);
        }
        else {
            SR.ShiftOut_Update(temp_step_led, 0x00);
        }

        // Finally, make the sounds play.
        Set_CPU_Trig_High();
    }

}