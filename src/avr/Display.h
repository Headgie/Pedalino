/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */

#ifdef NOLCD
#define screen_update()
#else
#define LCD_LINE1_PERSISTENCE   1500;

byte m1, m2, m3, m4;
unsigned long endMillis2;


void screen_info(byte b1, byte b2, byte b3, byte b4)
{
  m1 = b1;
  m2 = b2;
  m3 = b3;
  m4 = b4;
  endMillis2 = millis() + LCD_LINE1_PERSISTENCE;
}


char foot_char(byte footswitch)
{
  footswitch = constrain(footswitch, 0, PEDALS - 1);
  if (pedals[footswitch].function != PED_MIDI) return ' ';
  if ((footswitch == lastUsedPedal) ||

      ((pedals[footswitch].mode == PED_MOMENTARY1 ||
        pedals[footswitch].mode == PED_LATCH1) && pedals[footswitch].pedalValue[0] == LOW) ||

      ((pedals[footswitch].mode == PED_MOMENTARY2 ||
        pedals[footswitch].mode == PED_MOMENTARY3 ||
        pedals[footswitch].mode == PED_LATCH2) && pedals[footswitch].pedalValue[0] == LOW) ||
        
        pedals[footswitch].pedalValue[1] == LOW) return bar1[footswitch % 10];
  return ' ';
}


void screen_update(bool force = false) {

  static char screen1[LCD_COLS + 1];
  static char screen2[LCD_COLS + 1];
  static int  analog;
  static byte batteryLevel = 0;

  byte        f, p;

  if (!powersaver) {
    
    char buf[LCD_COLS + 1];
    
    // Line 1
    memset(buf, 0, sizeof(buf));
    if (millis() < endMillis2) {
      switch (m1) {
        case midi::NoteOn:
        case midi::NoteOff:
          sprintf(&buf[strlen(buf)], "Note %3d Ch%2d", m2, m4);
          break;
        case midi::ControlChange:
          sprintf(&buf[strlen(buf)], "CC%3d/%3dCh%2d", m2, m3, m4);
          break;
        case midi::ProgramChange:
          sprintf(&buf[strlen(buf)], "PC%3d    Ch%2d", m2, m4);
          break;
        case midi::PitchBend:
          sprintf(&buf[strlen(buf)], "Pitch%3d Ch%2d", m2, m4);
          break;
      }
    }
    else if ( MidiTimeCode::getMode() == MidiTimeCode::SynchroClockMaster || MidiTimeCode::getMode() == MidiTimeCode::SynchroClockSlave) {
      sprintf(&buf[strlen(buf)], "%3dBPM", bpm);
      for (byte i = 0; i < (LCD_COLS - 9); i++)
        if (MTC.isPlaying())
          buf[6 + i] = (MTC.getBeat() == i) ? '>' : ' ';
        else
          buf[6 + i] = (MTC.getBeat() == i) ? '.' : ' ';
    }
    else if ( MidiTimeCode::getMode() == MidiTimeCode::SynchroMTCMaster || MidiTimeCode::getMode() == MidiTimeCode::SynchroMTCSlave) {
      sprintf(&buf[strlen(buf)], "%02d:%02d:%02d:%02d    ", MTC.getHours(), MTC.getMinutes(), MTC.getSeconds(), MTC.getFrames());
    }
    else {
      for (byte i = 0; i < (LCD_COLS - 3); i++) {
        //buf[i] = foot_char(i);
        buf[i] = ' ';
      }
    }
    if (force || strcmp(screen1, buf) != 0) {     // do not update if not changed
      memset(screen1, 0, sizeof(screen1));
      strncpy(screen1, buf, LCD_COLS);
      lcd.setCursor(0, 0);
      lcd.print(buf);
#ifndef NOBLYNK
      blynkLCD.print(0, 0, buf);
#endif
    }
    
    if (bleConnected) {
      lcd.setCursor(13, 0);
      lcd.write(BLUETOOTHICON);
    }  
    if (wifiConnected || true) {
      lcd.setCursor(14, 0);
      lcd.write(WIFIICON);
    }
    if (powerPlug) {
      lcd.setCursor(15, 0);
      lcd.write(POWERPLUG);
    }
  
    byte newLevel = (millis() % 3500) / 500;
    newLevel = 3;
    if (force || batteryLevel != newLevel) {
      batteryLevel = newLevel;
      lcd.createChar(BATTERYLEVEL, battery[batteryLevel]);      
      lcd.setCursor(15, 0);
      lcd.write(BATTERYLEVEL);
    }

    // Line 2
    memset(buf, 0, sizeof(buf));
    sprintf(&buf[strlen(buf)], "Bank%2d", currentBank + 1);
    if (lastUsedPedal >= 0 && lastUsedPedal < PEDALS) {
      //strncpy(&buf[strlen(buf)], &bar2[0], map(pedals[lastUsedPedal].pedalValue[0], 0, MIDI_RESOLUTION - 1, 0, 10));
      //strncpy(&buf[strlen(buf)], "          ", 10 - map(pedals[lastUsedPedal].pedalValue[0], 0, MIDI_RESOLUTION - 1, 0, 10));
      f = map(pedals[lastUsedPedal].pedalValue[0], 0, MIDI_RESOLUTION - 1, 0, 50);
      p = f % 5;
      f = f / 5;
      strncpy(&buf[strlen(buf)], &bar2[0], f);
    }
    if (force || strcmp(screen2, buf) != 0 || analog != pedals[lastUsedPedal].pedalValue[0]) {     // do not update if not changed
      memset(screen2, 0, sizeof(screen2));
      strncpy(screen2, buf, LCD_COLS);
      analog = pedals[lastUsedPedal].pedalValue[0];
      lcd.setCursor(0, 1);
      lcd.print(buf);
      if (p > 0) lcd.write((byte)(p - 1));
      for (byte i = 0; i < 10 - f ; i++)
        lcd.print(" ");
      // replace unprintable chars
      for (byte i = 0; i < LCD_COLS; i++)
        buf[i] = (buf[i] == -1) ? '#' : buf[i];
#ifndef NOBLYNK
      blynkLCD.print(0, 1, buf);
#endif
    }
    
    if (selectBank) {
      lcd.setCursor(5, 1);
      lcd.cursor();
    }
    else
      lcd.noCursor();
  }
}

#endif  // NOLCD

