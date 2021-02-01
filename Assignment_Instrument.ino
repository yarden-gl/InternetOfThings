/* The following program simulates the basics of a drum set - toms, snare, bass, splash and ride.
 * The pizeo simulates the splash (and funnily enough it looks like it too).
 * The buttons simulate the rest of the drums. We defined the tones of each button to be 
 * similar to the frequency of the original individual instrument. 
 * Altogether we assembled our version of the arduino drum set.
 */

const int speakerPin = 9;           // Analog output pin that the larger speaker is attached to

const int buttonPin0 = 0;           // Digital input pin that the button is attached to
const int buttonPin1 = 1;           // Digital input pin that the button is attached to
const int buttonPin2 = 2;           // Digital input pin that the button is attached to
const int buttonPin3 = 3;           // Digital input pin that the button is attached to
const int pizeo = 0;                // Analog input pin that the pizeo is attached to
int val = 0;

void setup() {                      // run once, when the sketch starts
  pinMode(speakerPin, OUTPUT);
  
  pinMode(buttonPin0,INPUT_PULLUP);  // declare pushbutton as input
  pinMode(buttonPin1,INPUT_PULLUP); // declare pushbutton as input
  pinMode(buttonPin2,INPUT_PULLUP); // declare pushbutton as input
  pinMode(buttonPin3,INPUT_PULLUP); // declare pushbutton as input
  pinMode(pizeo,INPUT);             // sets the analog pin as input
}

void loop() {

   noTone(speakerPin);
   
  // Splash sound 
  val = analogRead(pizeo);
  if(val > 122) {
    int frequency = map(val, 122, 199, 200 , 4000);
    frequency = constrain(frequency,200,4000);
    tone(speakerPin,frequency);
    delay(100);
    noTone(speakerPin);
  }
  // Buttons 
  val = digitalRead(buttonPin0);// Toms sound of drums.
  if(val == LOW) {            
    tone(speakerPin, 210, 100);// 210 - frequency of Toms
    delay(300);    
  }
  val = digitalRead(buttonPin1);// Bass sound of drums.
  if(val == LOW) {
    tone(speakerPin, 100, 100);// 100 - frequency of Bass
    delay(300);   
  } 
  val = digitalRead(buttonPin2);// Snare sound of drums.
  if(val == LOW) {
    tone(speakerPin, 450, 100);// 450 - frequency of Snare
    delay(300);   
  } 
  val = digitalRead(buttonPin3);// Ride sound of drums.
  if(val == LOW) {
    tone(speakerPin, 900, 100);// 900 - frequency of Ride
    delay(300);
  }
  else
   noTone(speakerPin);
}