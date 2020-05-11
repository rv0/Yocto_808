void Sequencer_Tick()
{
    // Actions to take when playing the first time
    if (first_play_flag) {
        // Reset all counters.
        first_play_flag = step_count = clock24_count = clock96_count = pattern_count = 0;

        // Send sync start.
        if (sync_mode != DIN_SLAVE) {
            Set_Dinsync_Run_High();
        }

        if (sync_mode != MIDI_SLAVE) {
            MIDI_Send(0xfa); // Send Midi Start.
        }
    }

    // Keep clock24 within bounds.
    if (clock24_count == 24) {
        clock24_count = 0;
    }

    if (sync_mode == MASTER) {
        // Keep clock96 within bounds.
        if (clock96_count == 96) {
            clock96_count = 0;
        }

        // Derive 24ppqn from 96ppqn. // TODO: Future code should upsample clock when slaved.
        if (clock96_count % 4 == 0) {
            ticked = true;
        }
    }
    else {
        ticked = true;
    }
 
    // We have a clock tick.
    if (ticked) {
        ticked = false;

        // Send clock to slaves
        if (sync_mode != DIN_SLAVE) {
            Set_Dinsync_Clock_High(); // TODO: Magic 2ms delay on first play.
        }

        if (sync_mode != MIDI_SLAVE) {
            MIDI_Send(0xf8);
        }

        // We alternate the flag of the tempo LED.
        if (clock24_count >= 12) {
            tempo_led_flag = false;
        }
        else {
            tempo_led_flag = true;
        }

        Set_CPU_Trig_Low();
        Reset_Trig_Out();
        Update_Pattern_EEprom(); // Is the another pattern selected?

        // Sequencer is playing.
        if (play) {

            // Are we at the start of a step?
            if (clock24_count % (pattern_scale[pattern_buffer]/4) == 0) {
                //step_changed = 1;

                if (step_count >= nbr_step[pattern_buffer]) { // The pattern has ended.
                    step_count = 0 ; // Reset the step counter.
                    end_measure_flag = true; // The measure has ended.

                    //------------------------------------------------ WTF is this???????????????
                    if (load_pattern_ok_flag) { // If the pattern loaded correctly.
                        pattern_buffer = !pattern_buffer; //on switche entre les deux pattern present dans le buffer au debut de la mesure
                        load_pattern_ok_flag = false; //reinitialise le flag de load pattern
                    }
                }
                /*
                else if (step_count == (nbr_step[pattern_buffer])) {
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
                */
                
                SR.ShiftOut_Update(step_leds, ((inst_step_buffer[step_count][pattern_buffer]) & (~muted_instruments) | inst_roll));
                Send_Trig_Out((inst_step_buffer[step_count][pattern_buffer]) & (~muted_instruments) | inst_roll);

                // Increment counters last.
                step_count++;

            }
            else {
                //step_changed = 0;
            }
        }

        // Sequencer is stopped.
        else {

            if (first_stop_flag) {
                first_stop_flag = false;
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

        // Roll works regardless of play or stop.
        if (roll_mode && (clock24_count % roll_scale[scale_type][roll_pointer] == 1) && inst_roll > 0) { // @TODO
            SR.ShiftOut_Update(step_leds, inst_roll);
            Send_Trig_Out(inst_roll);
        }
        else {
            SR.ShiftOut_Update(step_leds, 0x00);
        }

        // Finally, make the sounds play.
        Set_CPU_Trig_High();

        // Lastly, increment the counters.
        clock24_count++;
    }

    if (sync_mode == MASTER) { // Always increment the 96ppqn counter.
        clock96_count++;
    }
}