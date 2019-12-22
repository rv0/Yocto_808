///////////////////////////////////////////////////////////////////////
// Timer qui incremente le compteur a 96PPQN
// en mode master
//////////////////////////////////////////////////////////////////////

void Count_24PPQN()
{
    clock_counter++;
}

///////////////////////////////////////////////////////////////////////
// Timer for 4ms dinsync output
// in mode master and midi sync.
//////////////////////////////////////////////////////////////////////
void Count_Clock()
{
    if (sync_mode == MASTER || sync_mode == MIDI_SLAVE) {
        if (dinsync_first_clock_timeout != 0) {
            dinsync_first_clock_timeout--;
            if (dinsync_clock_timeout == 0) {
                Set_Dinsync_Clock_High();
            }
        }

        if (dinsync_clock_timeout != 0) {
            dinsync_clock_timeout--;
            if (dinsync_clock_timeout == 0) {
                Set_Dinsync_Clock_Low();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////
//  Pin Change interruption
// Une interruption est generer sur les Pin PD4 et PD5
// PD4= Din CLK      PD5= Din Start
// a la sync Master ici on est en 24PPQN et non en 96PPQN
///////////////////////////////////////////////////////////////////////
ISR (PCINT3_vect)
{
    clock_counter++;
}

void MIDI_Send(byte OutByte)
{
    Serial1.write(OutByte);
}
