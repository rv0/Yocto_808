void loop()
{

    Check_Menu_Inst();

    switch (selected_mode) {

    case PATTERN_MIDI_MASTER:
    case PATTERN_DIN_SLAVE:
    case PATTERN_MIDI_SLAVE:

        // Check if the selected mode changed.
        if (!play && old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();

            switch (selected_mode) {
            case PATTERN_MIDI_MASTER:
                Set_Sync_Mode(MASTER);
                Init_Tap_Tempo();
                break;

            case PATTERN_DIN_SLAVE:
                Set_Sync_Mode(DIN_SLAVE);
                break;

            case PATTERN_MIDI_SLAVE:
                Set_Sync_Mode(MIDI_SLAVE);
                MIDI.setHandleClock(Handle_Clock);    // Callback Clock MIDI
                MIDI.setHandleStart(Handle_Start);    // Callback Start MIDI
                MIDI.setHandleContinue(Handle_Start); // Callback Continue MIDI
                MIDI.setHandleStop(Handle_Stop);      // Callback Stop MIDI
                break;
            }

            mute_mode = 0;
            muted_instruments = 0;
        }

        // When we switch between modes while playing we don't wanna lose the sync mode.
        if (first_play_flag) {
            sync_fallback = sync_mode;
        }

        if (sync_mode == MASTER) {
            Check_BPM();
            Read_Tap_Tempo();
        }
        else if (sync_mode == MIDI_SLAVE) {
            MIDI.read();
        }

        Mode_Pattern(); // Button readout SR1
        Check_Edit_Button_Pattern(); // Button readout SR2
        Check_Roll_Scale(); // Encoder readout

        Verticalize_Pattern(); // Read the changed pattern if needed.

        // Check if we have advanced 1PPQN.
        if (clock_counter > 0) {
            clock_counter--; // Decrement clock_counter. Important!
            Sequencer_Tick();
            Update_Pattern_EEprom();
            Update_Pattern_Led();
        }

        break;
    case PATTERN_EDIT:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;

            if (!play) {
                Set_Sync_Mode(sync_fallback);
            }
        }

        if (sync_mode == MASTER) {
            Check_BPM();
        }
        else if (sync_mode == MIDI_SLAVE) {
            MIDI.read();
        }

        Check_Edit_Button_Pattern_Edit(); // Button readout SR2
        Mode_Pattern(); // Button readout SR1

        Verticalize_Pattern(); // Read the changed pattern if needed.

        // Check if we have advanced 1PPQN.
        if (clock_counter > 0) {
            clock_counter--; // Decrement clock_counter. Important!
            Sequencer_Tick();
            Update_Pattern_EEprom();
            Update_Pattern_Led();
        }

        break;

    case INIT_EEPROM:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(MIDI_SLAVE);

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi (send a MIDI stop)
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }
        }

        Check_Edit_Button_Setup();
        Play_Version();
        Initialize_EEprom();//initialise les 256 patterns (init. the 256 patterns)
        break;
    
    case CLEAR_PATTERN:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(MIDI_SLAVE);//mode master synchro
            // flag that we have just reentered the "clear pattern" mode
            first_time_clear_pattern = 1; //flag que l'on vient de rentrée dans le mode clear pattern

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi (send a MIDI stop)
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }
        }

        Check_Edit_Button_Setup();

        if (!button_shift) {
            SR.Led_Step_Write(1 << selected_pattern);
            PORTC &= ~(B11111100);//clear les edits leds dans ce mode
        }
        else if (button_shift) {
            SR.Led_Step_Write(1 << pattern_bank);
            PORTC |= B11111100;//clear les edits leds dans ce mode
        }

        Clear_Pattern();
        break;
    case COPY_PATTERN:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(MIDI_SLAVE);
            first_time_copy_pattern = 1; //flag que l'on vient de rentrée dans le mode copy pattern

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi (send a MIDI stop)
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }
        }

        Check_Edit_Button_Setup();

        if (!button_shift) {
            SR.Led_Step_Write(1 << selected_pattern);
            PORTC &= ~(B11111100);//clear les edits leds dans ce mode
        }
        else if (button_shift) {
            SR.Led_Step_Write(1 << pattern_bank);
            PORTC |= B11111100;//clear les edits leds dans ce mode
        }

        Copy_Pattern();
        break;
    case PASTE_PATTERN:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(MIDI_SLAVE);//mode master synchro
            first_time_paste_pattern = 1; //flag que l'on vient de rentrée dans le mode paste pattern

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }
        }

        Check_Edit_Button_Setup();

        if (!button_shift) {
            SR.Led_Step_Write(1 << selected_pattern);
            PORTC &= ~(B11111100);//clear les edits leds dans ce mode
        }
        else if (button_shift) {
            SR.Led_Step_Write(1 << pattern_bank);
            PORTC |= B11111100;//clear les edits leds dans ce mode
        }

        Paste_Pattern();
        break;

    case SONG_EDIT:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(MASTER);//mode master synchro

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi (send a MIDI stop)
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }

            //Init_Tap_Tempo();
        }

        Check_BPM();
        // Timer1.initialize(timer_time); // set a timer of length in microseconds
        Check_Edit_Button_Song();
        //Read_Tap_Tempo();
        Mode_Song_Edit();
        Update_Pattern_EEprom();
        Update_Song_EEprom();
        Update_Song_Led();
        break;
    case SONG_MIDI_MASTER:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(MASTER);//mode master synchro

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi (send a MIDI stop)
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }

            Init_Tap_Tempo();

        }

        Check_BPM();
        Check_Edit_Button_Song();
        Read_Tap_Tempo();
        Update_Song_EEprom();
        Update_Pattern_EEprom();
        Mode_Song_Play();
        Update_Song_Led();

        break;
    case SONG_DIN_SLAVE:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(DIN_SLAVE);

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi (send a MIDI stop)
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }
        }

        //Check_BPM();
        Check_Edit_Button_Song();
        Mode_Song_Play();
        Update_Pattern_EEprom();
        Update_Song_EEprom();
        Update_Song_Led();
        break;
    case SONG_MIDI_SLAVE:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            Disconnect_Callback();
            Set_Sync_Mode(MIDI_SLAVE);

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }

            MIDI.setHandleClock(Handle_Clock);// Callback Clock MIDI
            MIDI.setHandleStart(Handle_Start);// Callback Start MIDI
            MIDI.setHandleContinue(Handle_Start);// Callback Stop MIDI
            MIDI.setHandleStop(Handle_Stop);// Callback Stop MIDI
        }

        //Check_BPM();
        MIDI.read();
        Check_Edit_Button_Song();
        Mode_Song_Play();
        Update_Pattern_EEprom();
        Update_Song_EEprom();
        Update_Song_Led();
        break;

    case MIDI_PLAY:

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
            PORTC &= ~(B11111100);//clear les edits leds dans ce mode

            Disconnect_Callback();
            Set_Sync_Mode(MIDI_SLAVE);

            MIDI.setHandleNoteOn(Handle_NoteOn);  // Callback NoteON
            MIDI.setInputChannel(selected_channel + 1); //initialise le channel midi

            if (play) {
                play = 0;
                MIDI_Send(0xfc);//envoi un stop midi
                Set_Dinsync_Run_Low();
                button_play_count = 0;
            }

            //initialise les leds suivant le channel selectionner et les sortie des instru a 0
            Load_Midi_Channel();
            temp_step_led = (1 << selected_channel);
            MIDI.setInputChannel(selected_channel + 1);
            SR.ShiftOut_Update(temp_step_led, 0);

            for (int ct = 0; ct < 12; ct++) {
                noteOnOff[ct] = 0;
            }

            Load_Midi_Note();
        }

        // Check user interface
        Check_Edit_Button_Setup();
        Check_Midi_Channel();

        if (selected_channel_changed) {
            MIDI.setInputChannel(selected_channel + 1);
            SR.ShiftOut_Update(temp_step_led, 0);
            Save_Midi_Channel();
            selected_channel_changed = 0;
        }

        Save_Midi_Note();//sauve le note midi si elles ont changé

        if (midi_led_flash_count > 0) {
            --midi_led_flash_count;

            if (midi_led_flash_count == 0) {
                PORTC &= ~MIDI_ACTIVITY_LED;
            }
        }

        if (midi_trig_pulse_count > 0) {
            --midi_trig_pulse_count;

            if (midi_trig_pulse_count == 0) {
                Reset_Trig_Out();
                SR.ShiftOut_Update(temp_step_led, 0);
            }
        }

        // Check for MIDI data
        inst_midi_buffer = 0;
        MIDI.read();

        if (inst_midi_buffer == 0) {
            // Nothing we can do with the MIDI data, go to next loop.
            return;
        }

        // The order of what follows is important.
        //
        // 1. Set pins HIGH for the instrument(s) we want to trigger.
        // 2. Send a 10 microsecond pulse to the TRIG_CPU input of the Accent circuit
        // 3. The TRIG_CPU pulse triggers a monostable  in the Accent circuit which creates a 1
        //    millisecond (inverse) pulse.
        // 4. After the 1 ms Accent pulse is done, return the instrument trigger pins to LOW.
        //
        // It is OK if step 4 is 'late' but if it is too short it affects the
        // envelopes of the instruments. The instrument trigger must remain high
        // until the monostable pulse inside the Accent circuit is done.
        //
        SR.ShiftOut_Update(temp_step_led, inst_midi_buffer);

        Set_CPU_Trig_High();
        delayMicroseconds(10);
        Set_CPU_Trig_Low();

        Send_Trig_Out_Midi();

        // This number is tuned to the speed of the main loop. It needs to be
        // more than 1ms to give Accent monostable enough time to create its 1ms
        // pulse.
        midi_trig_pulse_count = 7;


        break;

    case EEPROM_DUMP:
        Check_Edit_Button_Setup();

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
        }

        if (button_play) {
            Dump_EEprom();
        }

        break;
    case EEPROM_RECEIVE:
        Check_Edit_Button_Setup();

        if (old_selected_mode != selected_mode) {
            old_selected_mode = selected_mode;
        }

        while (MIDI.read()) {
            if (MIDI.getType() >= 0xf0) { // SysEX
                Receive_EEprom(MIDI.getSysExArray(), MIDI.getSysExArrayLength());
            }
        }

        SR.Led_Step_Write(0); // Disable the LEDs.
        break;
    }

}
