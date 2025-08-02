//*-----------------------------------------------------------------------------*//

// C++ code
// PEK DING EN , MCIN_PROJECT_2

//*-----------------------------------------------------------------------------*//
// GLOBAL VARIABLES

// WAITS
int WAIT_TIME = 1500; // 1.5 SECONDS
int ROTATING_WAIT_TIME = 500; // 0.5 SECONDS
int GAME_ROTATING_WAIT_TIME = 100; // 0.1 SECONDs
int COUNT_DOWN_WAIT_TIME = 1000; // 1 SECOND

// BOOLEANS
bool simplePattern = true;
bool bombGame = false;
bool COUNT_DOWN_ACTIVE = false;
unsigned long COUNT_DOWN_START = 0;

// SHIFT
int SHIFT_SPACE_RIGHT = 2;
int SHIFT_SPACE_LEFT = 1;

// GAME VARIABLE
int pressedAmount = 0;
int COUNT_DOWN = 0;

//*-----------------------------------------------------------------------------*//
// SETUP

void setup()
{
  DDRD = 0xFC;
  DDRB = 0x07; // WITH BUZZER

  // LIST OF INPUTS
  /*
    PB1 = 0x01 = PINB
    PB2 = 0x02 = PINB
  */
  #define BZR 0x04 // BUZZER
  // IF I CAN USE PINMODE AND DIGITALWRITE, I CAN MAKE SOUND MORE APPEALING
}

//*-----------------------------------------------------------------------------*//
// FUNCTIONS //

// ALTERNATE PATTERN
void AlternateBlinkPattern()
{
  if (!simplePattern) return;

  const byte ledPatterns[] = {                       // ENABLES ME TO DRAW A PATTERN (DNA PATTERN)
    0b10000100,
    0b01001000,
    0b00110000,
    0b01111000,
    0b11111100,
    0b01111000,
    0b00110000, 
    0b01001000, 
    0b10000100, 
    0b01001000, 
    0b00110000,
    0b01111000,
    0b11111100,
    0b01111000,
    0b00110000, 
    0b01001000,
    0b10000100
  };

    for (int i = 0; i < sizeof(ledPatterns); i++)       // USE FOR LOOP TO EXECUTE THE PATTERN
    {
      PORTD = ledPatterns[i];
      delay(ROTATING_WAIT_TIME);                        // PAUSE FOR AWHILE IF NOT WILL WILL BREAK THE PATTERN
    }
}

unsigned char pat = 0x01;
void RotateLeft()
{
  if (!simplePattern) return;
  
  // SHIFTING
  PORTD = pat;
  delay(ROTATING_WAIT_TIME);
  pat = pat << SHIFT_SPACE_LEFT;

  if (pat == 0)
  {
    pat = 0x01;
  }
}

void EnableBombGame()
{
  // PRESS LEFT_PB
  if (PINB & 0x02)
  {
    // TURNS EVERYTHING ON TO SHOW GAME START
    simplePattern = false;
    PORTD = 0xFC;
    for (int i = 0; i < 3; i++)
    {
      // 3 PLANTING BOMB BEEPS
      PORTB |= BZR;        // ON
      delay(50);           // BEEP DURATION
      PORTB &= ~BZR;       // OFF
      delay(75);           // WAIT TIME BEFORE NEXT BEEP
    }

    // WAIT
    delay(WAIT_TIME);

    // TURNS EVERYTHING OFF
    PORTD = 0x00;
    
    // ENABLE BOMB GAME
    bombGame = true;

    // START COUNTDOWN
    COUNT_DOWN = 25; // INIT COUNT_DOWN
    COUNT_DOWN_START = millis();
    COUNT_DOWN_ACTIVE = true;
  }
}

void BombGame()
{
  static unsigned long oldShiftTime = 0;
  unsigned long currentShiftTime = millis();

  if (!bombGame || !COUNT_DOWN_ACTIVE) return;

  // Shift LED pattern every 100ms (or whatever speed you want)
  if (currentShiftTime - oldShiftTime >= GAME_ROTATING_WAIT_TIME) // IF DELTATIME(TIME PASSED) >= GAME_ROTATING_WAIT_TIME
  {
    // RESET DELTATIME(TIME PASSED)
    oldShiftTime = currentShiftTime;

    // SHOW PATTERN
    PORTD = pat;

    // BEEP AND COUNTDOWN ONLY WHEN FIRST LED(0x04) IS LIT
    if (pat == 0x04)
    {
      Serial.println(COUNT_DOWN);
      PORTB |= BZR;   // Beep ON
      delay(50);      // BEEP DURATION
      PORTB &= ~BZR;  // Beep OFF

      COUNT_DOWN--;   // COUNTDOWN

      if (COUNT_DOWN > 20) {GAME_ROTATING_WAIT_TIME = 100;}
      else if (COUNT_DOWN > 15) {GAME_ROTATING_WAIT_TIME = 75;}
      else if (COUNT_DOWN > 10) {GAME_ROTATING_WAIT_TIME = 50;}
      else if (COUNT_DOWN > 5) {GAME_ROTATING_WAIT_TIME = 25;}
      else {GAME_ROTATING_WAIT_TIME = 15;}

      // LOSE CONDITION
      if (COUNT_DOWN <= 0)
      {
        for (int i = 0; i < 5; i++)
        {
          // 3 PLANTING BOMB BEEPS
          PORTB |= BZR;        // ON
          delay(50);           // BEEP DURATION
          PORTB &= ~BZR;       // OFF
          delay(50);           // WAIT TIME BEFORE NEXT BEEP
        }

        COUNT_DOWN_ACTIVE = false;
        PORTD = 0xFC;  // All LEDS ON

        PORTB |= BZR;   // Beep ON
        delay(1000);
        PORTB &= ~BZR;  // Beep OFF

        PORTD &= 0x00;  // All LEDS OFF

        pressedAmount = 0;
        return;
      }
    }

    // IN-GAME SHIFTING
    pat = pat << SHIFT_SPACE_LEFT;
    if (pat == 0)
    {
      pat = 0x01;
    }
  }

  // DEFUSE CHECK
  if (pat == 0x80 && (PINB & 0x01) && (pressedAmount == 2))
  {
    // DEFUSED!
    bombGame = false;
    COUNT_DOWN_ACTIVE = false;
    PORTD = 0x00;
    pressedAmount = 0;
    return;
  }
}

void DefuseSINGLEPress()
{
  static bool LASTBUTTON_STATE = false;                                       // THIS STORES THE PREVIOUS STATE
  bool CURRENTBUTTON_STATE = PINB & 0x01;
  if (bombGame && CURRENTBUTTON_STATE && !LASTBUTTON_STATE && pat == 0x80)     // CONDITIONS TO ADD PRESSED_AMOUNT
  {
    pressedAmount++;
  }
  LASTBUTTON_STATE = CURRENTBUTTON_STATE;                                      // THIS UPDATES THE BUTTON STATE FOR THE NEXT LOOP
}

//*-----------------------------------------------------------------------------*//

void loop()
{
  // Only when LEFT_PB is pressed THEN simplePattern is false //
  EnableBombGame();
  BombGame();
  DefuseSINGLEPress();

  // Only when RIGHT_PB is pressed AND simplePattern is true //
  // PRESS RIGHT_PB
  if (PINB & 0x01)
  {
  	AlternateBlinkPattern();
  }
  else
  {
    RotateLeft();
  }
}

//*-----------------------------------------------------------------------------*//