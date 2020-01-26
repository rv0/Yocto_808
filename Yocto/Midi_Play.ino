#define NOTE_ON 0x90 //note on sur canal 1


void Handle_NoteOn(byte channel, byte pitch, byte velocity)
{
    if (velocity == 0) {
        return;
    }

    if (midi_led_flash_count == 0) {
        PORTC |= MIDI_ACTIVITY_LED;
    }

    midi_led_flash_count = 10;

    //on enregistre le numero du noteOn dans l'instrument selectionné
    if (button_shift) {
        inst_midi_note[selected_inst] = pitch;
        inst_midi_note_edited = 1;
    }

    if (pitch == inst_midi_note[0]) {
        bitSet(inst_midi_buffer, 0);
    }

    if (pitch == inst_midi_note[1]) {
        bitSet(inst_midi_buffer, 1);
    }

    if (pitch == inst_midi_note[2]) {
        bitSet(inst_midi_buffer, 2);
    }

    if (pitch == inst_midi_note[3]) {
        bitSet(inst_midi_buffer, 3);
    }

    if (pitch == inst_midi_note[4]) {
        bitSet(inst_midi_buffer, 4);
    }

    if (pitch == inst_midi_note[5]) {
        bitSet(inst_midi_buffer, 5);
    }

    if (pitch == inst_midi_note[6]) {
        bitSet(inst_midi_buffer, 6);
    }

    if (pitch == inst_midi_note[7]) {
        bitSet(inst_midi_buffer, 7);
    }

    if (pitch == inst_midi_note[8]) {
        bitSet(inst_midi_buffer, 8);
    }

    if (pitch == inst_midi_note[9]) {
        bitSet(inst_midi_buffer, 9);
    }

    if (pitch == inst_midi_note[10]) {
        bitSet(inst_midi_buffer, 10);
    }

    if (pitch == inst_midi_note[11]) {
        bitSet(inst_midi_buffer, 11);
    }

    if (pitch == inst_midi_note[12]) {
        bitSet(inst_midi_buffer, 12);
    }

    if (pitch == inst_midi_note[13]) {
        bitSet(inst_midi_buffer, 13);
    }

    if (pitch == inst_midi_note[14]) {
        bitSet(inst_midi_buffer, 14);
    }

    if (pitch == inst_midi_note[15]) {
        bitSet(inst_midi_buffer, 15);
    }

    if (inst_midi_buffer > 0 && velocity >= 100) {
        // Enable accent
        bitSet(inst_midi_buffer, 0);
    }
}


void Handle_NoteOff(byte channel, byte pitch, byte velocity)
{
}


void Handle_Start()
{
    Set_Dinsync_Run_High();
    play = 1;
    first_play = 1;
}


void Handle_Stop()
{
    Set_Dinsync_Run_Low();
    play = 0;
    Set_CPU_Trig_Low();
}

void Handle_Clock()
{
    clock_counter++;
}

void Check_Midi_Channel()
{
    unsigned int reading = SR.Button_Step_Read();   // on lit les boutons

    if (reading != old_step_button_state) {           // si ils ont changer on enregistre le temps
        millis_debounce_step_button = millis();
    }

    if ((millis() - millis_debounce_step_button) >= DEBOUNCE) {
        if (reading != step_button_state) {
            step_button_state = reading;

            if (!button_shift) {
                for (byte i = 0; i < 16; i++) { //loop autant de fois que de bouton step soit 16
                    if (bitRead(step_button_state, i)) {
                        //on retourne la valeur du pattern selectionner
                        selected_channel = i;
                        selected_channel_changed = 1;//flag que le pattern selectionner a change
                        temp_step_led = (1 << selected_channel);
                        break;
                    }
                }
            }
        }
    }

    old_step_button_state = reading;
}


void Disconnect_Callback()
{
    /*
    DDRA |= ~B11111100;// les PIN 39 et 40 sont en sorti pour le commun des rotary switch les autres sont en entree pour l'encoder et les rotary switch
    DDRC |= B11111100;// les 6 bits fort du PORTC sont en sorti pour les 6 led Edit
    DDRB |= B111;// les trois sorties TRIG  CPU,1 et 2
    DDRD |= (1<<6);//sortie trig out
    inst_midi_buffer=0;
    SR.ShiftOut_Update(temp_step_led,inst_midi_buffer);
    Set_CPU_Trig_Low();
    */
    PORTC &= ~(B11111100);//clear les edits leds dans ce mode
    SR.Led_Step_Write(0);//tous les leds Step Off
    MIDI.disconnectCallbackFromType(midi::NoteOn);
    MIDI.disconnectCallbackFromType(midi::Clock);
    MIDI.disconnectCallbackFromType(midi::Start);
    MIDI.disconnectCallbackFromType(midi::Stop);
}
