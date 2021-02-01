/**
 * Game of Lights
 * The objective of this game is to get across the board. The player starts at the top and has to make their way across
 * the board while avoiding the moving enemies. You can move up, down, left and right using the buttons.
 * For the first 4 levels, there is one enemy per row that moves back and forth. With every level the enemies move 
 * with increasing speed. When you reach level 5, the enemies multiply - each row has two enemies and they too increase their speed
 * with every level. If at any point you collide with the enemy the game starts over!
 * You will be able to know which level you are on by the digit display.
 * You win the game when you complete level 9 and then the game starts all over. Good Luck!
 **/


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(64, PIN, NEO_GRB + NEO_KHZ800);

const int length = 8; // length of row in pixel strip
const int maxEnemiesPerRow = 2;
int numEnemiesPerRow = 1; // begin with 1 enemy per row

// Buttons
const int down = 0;           // Digital input pin down button is attached to
const int up = 1;         // Digital input pin up button is attached to
const int left = 2;        // Digital input pin left button is attached to
const int right = 3;         // Digital input pin right button is attached to

// Intervals
int onTimeEnemies = 900;
const int onTimePlayer = 150;
int onTimeDigits = 1000;


int enemies [maxEnemiesPerRow][length - 1]; // last row has no enemies
boolean direction [maxEnemiesPerRow][length - 1]; // TRUE for Right, FALSE for Left

int playerRow = 0;
int playerColumn = 0;
int inputVal = 0;
int level = 1;

unsigned long prevMillisEnemies;
unsigned long prevMillisPlayer;
unsigned long prevMillisDigits;

// Digits setup

int latchPin = 9;
int clockPin = 10;
int dataPin = 8;

const int sevenSeg[9] = {
  B00000110, // 1
  B01011011, // 2
  B01001111, // 3
  B01100110, // 4
  B01101101, // 5
  B01111101, // 6
  B00000111, // 7
  B01111111, // 8
  B01101111, // 9
};

void setup() {
  
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  
  pinMode(up,INPUT_PULLUP);     // declare pushbutton as input
  pinMode(down,INPUT_PULLUP);   // declare pushbutton as input
  pinMode(right,INPUT_PULLUP);  // declare pushbutton as input
  pinMode(left,INPUT_PULLUP);   // declare pushbutton as input
  
  strip.begin();
  initGame();
  strip.show();

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

// Initialize game 
void initGame() {
  colorWipe(strip.Color(0, 0, 0), 0);
  initEnemies();
  // Player begins in the middle of the top row
  playerRow = 0;
  playerColumn = 4;
  setPlayer(0,4);
  prevMillisPlayer = 0;
  prevMillisEnemies = 0;     
}

// Reset game back to level 1 - occurs when player collides with enemy or wins the game
void reset() {
  level = 1;
  numEnemiesPerRow = 1;
  onTimeEnemies = 900;
  initGame();
}

void setPlayer(int prevRow,int prevColumn) {
  strip.setPixelColor(prevRow * 8 + prevColumn, strip.Color(0,0,0)); 
  strip.setPixelColor(playerRow * 8 + playerColumn, strip.Color(50,50,50)); 
}

// Initialize enemy positions
void initEnemies() {
  enemies[0][0] = -1;
  enemies[1][0] = -1;
  for( int i = 0; i < numEnemiesPerRow; i++) {
    for(int j = 1;j < length - 1;j++) { // First row is for player
    
      float color = (j % 2 == 0) ? strip.Color(100, 0, 0) : strip.Color(0, 0, 100);
      strip.setPixelColor(j * 8 + (enemies[i][j] = random(0,length)), color); 

      direction[0][j] = true; // initialize one enemy to the right
    }
  }
}

// Move enemies one step
void moveEnemies() {

  int prev = 0;
  for(int i = 0;i < numEnemiesPerRow;i++) {
    for(int j = 1;j < length - 1;j++) { // First row is for player
      prev = enemies[i][j];
      if((direction[i][j] && enemies[i][j] == 7) || !direction[i][j] && enemies[i][j] != 0) {
        enemies[i][j]--;
        direction[i][j] = false;
      } else {
        enemies[i][j]++;
        direction[i][j] = true;
      }

      strip.setPixelColor(j * 8 + prev, strip.Color(0, 0, 0));
      // alternate between two colors for enemies
      uint32_t color = (j % 2 == 0) ? strip.Color(100, 0, 0) : strip.Color(0, 0, 100);
      strip.setPixelColor(j * 8 + enemies[i][j], color); 
      
    }
  }
}

void movePlayer() {
  int prevRow = playerRow;
  int prevColumn = playerColumn;
  
  inputVal = digitalRead(up);
  if(inputVal == LOW && playerRow != 0) 
    playerRow--;
    
  inputVal = digitalRead(down);
  if(inputVal == LOW && playerRow != 7) 
    playerRow++; 

  inputVal = digitalRead(right);
  if(inputVal == LOW && playerColumn != 7) 
    playerColumn++; 

  inputVal = digitalRead(left);
  if(inputVal == LOW && playerColumn != 0) 
    playerColumn--; 

  setPlayer(prevRow,prevColumn);
}


// Fill the dots one after the other with color c
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// When player and enemy collide
boolean collision() {
  for(int i = 0;i < numEnemiesPerRow;i++) {
    if(enemies[i][playerRow] == playerColumn)
      return true;
  }
  return false;
}

void loop() {
  
  unsigned long currentMillis = millis();
  if(currentMillis - prevMillisEnemies >= onTimeEnemies) {
    moveEnemies();
    prevMillisEnemies = currentMillis;
  } 

  if(currentMillis - prevMillisDigits >= onTimeDigits) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 255-sevenSeg[level - 1]);
    digitalWrite(latchPin, HIGH);
    prevMillisDigits = currentMillis;
  }
  
  if(currentMillis - prevMillisPlayer >= onTimePlayer) {
    movePlayer();
    strip.show();
    if(playerRow == 7) {
      if(level == 9) { // The game is won!
        colorWipe(strip.Color(50, 50, 50), 50);
        level = 1;
      } else {
        colorWipe(strip.Color(0, 50, 0), 50);
        level++;
      }
      numEnemiesPerRow = (level < 5) ? 1 : 2; 
      onTimeEnemies = (level == 5) ? 900 : onTimeEnemies - 100;
      initGame();
     }
    prevMillisPlayer = currentMillis;
 }
  if(collision()) {
    colorWipe(strip.Color(50, 0, 0), 50);
    reset();
  }
  
}