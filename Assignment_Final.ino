#define BLYNK_PRINT Serial

// Libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <DHT.h>

#define WIFI_SSID "Insert network name here"
#define WIFI_PASSPHRASE "Insert password here"

char auth[] = "Insert Blynk authentication here"; // Blynk authentication

/*************************** Adafruit Setup ************************************/

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Insert username here"
#define AIO_KEY         "Insert key here"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[] = AIO_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] = AIO_USERNAME;
const char MQTT_PASSWORD[] = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

// Setup feeds for temperature & humidity
const char TEMPERATURE_FEED[] = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

const char HUMIDITY_FEED[] = AIO_USERNAME "/feeds/humidity";
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);

const char MEALS_FEED[] = AIO_USERNAME "/feeds/meals";
Adafruit_MQTT_Publish meals = Adafruit_MQTT_Publish(&mqtt, MEALS_FEED);
/***************************************************************************/

// Constants
#define SPEAKER_PIN D1
#define PIEZO_PIN A0  
#define LED_PIN D3
#define TRIGGER_PIN  D7           // Ultrasonic sensor
#define ECHO_PIN     D6           // Ultrasonic sensor
#define THRESHOLD 10              // of Piezo
#define DHTPIN D4
#define DHTTYPE DHT22
#define PIN_TEMPERATURE V1
#define PIN_HUMIDITY V2

// Notes
#define NOTE_A3  220
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_C5  523
#define NOTE_D5  587
#define REST      0


// Variables
DHT dht(DHTPIN, DHTTYPE, 15);
int onTimeDetection = 100;
int onTimeMQTT = 900;
int onTimeActivate = 900000;       // 15 minutes
unsigned long prevMillisDetection;
unsigned long prevMillisMQTT;
unsigned long initialMillis;
boolean moveBowl = false;         // If true, bowl needs to be moved
boolean piezoActivated = true;
boolean detected = false;
int val = 0;                      // Variable for reading input pin status

// Functions
void connect();
boolean petDetection(int currentMillis);
void mqttPingPublish();
int ultrasonicMeasure(int TRIGGER_PIN, int ECHO_PIN);
void playMusic();

int melody[] = {
  
  // Zelda's Lullaby - The Legend of Zelda Ocarina of Time. 
  // Score available at https://musescore.com/user/12754451/scores/2762776
  
  NOTE_E4,2, NOTE_G4,4,
  NOTE_D4,2, NOTE_C4,8, NOTE_D4,8, 
  NOTE_E4,2, NOTE_G4,4,
  NOTE_D4,-2,
  NOTE_E4,2, NOTE_G4,4,
  NOTE_D5,2, NOTE_C5,4,
  NOTE_G4,2, NOTE_F4,8, NOTE_E4,8, 
  NOTE_D4,-2,
  NOTE_E4,2, NOTE_G4,4,
  NOTE_D4,2, NOTE_C4,8, NOTE_D4,8, 
  NOTE_E4,2, NOTE_G4,4,
  NOTE_D4,-2,
  NOTE_E4,2, NOTE_G4,4,

  NOTE_D5,2, NOTE_C5,4,
  NOTE_G4,2, NOTE_F4,8, NOTE_E4,8, 
  NOTE_F4,8, NOTE_E4,8, NOTE_C4,2,
  NOTE_F4,2, NOTE_E4,8, NOTE_D4,8, 
  NOTE_E4,8, NOTE_D4,8, NOTE_A3,2,
  NOTE_G4,2, NOTE_F4,8, NOTE_E4,8, 
  NOTE_F4,8, NOTE_E4,8, NOTE_C4,4, NOTE_F4,4,
  NOTE_C5,-2, 
  
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int tempo = 120; // Tempo of song
int wholenote = (60000 * 4) / tempo; // Calculates the duration of a whole note in ms
int divider = 0, noteDuration = 0;


/*************************** Sketch Code ************************************/


void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, WIFI_SSID, WIFI_PASSPHRASE);
  connect();         // Connect to Adafruit IO
  dht.begin();       // Initiate sensor
  
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);          
  pinMode(TRIGGER_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT); 
}

  
void loop() {
  
  Blynk.run();
  
  unsigned long currentMillis = millis();

  // Publishing temperature, humidity, and pet eating status to MQTT 
  if(currentMillis - prevMillisMQTT >= onTimeMQTT) {
    mqttPingPublish();
    prevMillisMQTT = currentMillis;
  }
    
  currentMillis = millis();

  // Detecting if pet is near
  if(currentMillis - prevMillisDetection >= onTimeDetection && piezoActivated) {
    initialMillis = petDetection(currentMillis) ? currentMillis : initialMillis;
    prevMillisDetection = currentMillis;
  }

  currentMillis = millis();

  // Activate piezo 15 minutes after pet detected
  if(!piezoActivated && (currentMillis - initialMillis >= onTimeActivate)) {
    piezoActivated = true;         // Ready for next pet detection
    detected = !detected;
  }

  // Alert for moving bowl
  if(moveBowl) { 
    digitalWrite(LED_PIN, HIGH); // Turn LED on
    delay(1000);
  }
}


/**
 * This function is for detecting when your pet approaches their food bowl.
 */
boolean petDetection(int currentMillis) {

   val = analogRead(PIEZO_PIN);

  int distance = ultrasonicMeasure(TRIGGER_PIN, ECHO_PIN);
    
  if(val > THRESHOLD && (distance <= 50 && distance >= 0)) { // Piezo was pressed and pet is near
    Serial.println("Pet Detected");
    detected = true;
    piezoActivated = false;       // Pet identified, reactivate piezo after 15 minutes
    playMusic();
    Blynk.notify("Your pet is eating!");
    
    return true;
  }
  return false;
}


/**
 * Ping mqtt to keep the connection. The function is also responsible for
 * publishing the temperature, humidity, and whether you pet is eating.
 */
void mqttPingPublish() {
  
  if (! mqtt.ping(3)) {       // Ping Adafruit IO to remain connected
    if (! mqtt.connected())   // Reconnect to Adafruit IO
      connect();
  }
 
  int humidity_data = (int)dht.readHumidity();          // Grab the current state of the sensor  
  int temperature_data = (int)dht.readTemperature();    // Grab the current state of the sensor
  int meals_data = (int)detected;    

  if(humidity_data > 50 || temperature_data > 25)
    moveBowl = true;
  else
    moveBowl = false;

  if (! meals.publish(meals_data))                      // Publish data
    Serial.println("Failed to publish meals");
  else
    Serial.println("Meals published");
  
  if (! temperature.publish(temperature_data))          // Publish data
    Serial.println("Failed to publish temperature");
  else
    Serial.println("Temperature published");

  if (! humidity.publish(humidity_data))                // Publish data
    Serial.println("Failed to publish humidity");
  else
    Serial.println("Humidity published");
}


/**
 * This function is for grabbing and calculating the 
 * current state of the ultrasonic sensor.
 */
int ultrasonicMeasure(int TRIGGER_PIN, int ECHO_PIN) {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, LOW);
  
  int duration = pulseIn(ECHO_PIN, HIGH);           // Measure the pulse input in echo pin
  return (duration / 2) / 29.1;
}


/** 
 * Play music through Blynk
 */
BLYNK_WRITE(V0) {
  yield();
   val = param.asInt();
   Serial.print("V0 value is: ");
   Serial.println(val);
   if(val == 1) {   
    playMusic();
   }
}


/**
 * This function sends the current temperature
 * to Blynk when Arduino receives a pull.
 */
BLYNK_READ(PIN_TEMPERATURE)
{
  // This command sends the current state of the sensor 
  Blynk.virtualWrite(PIN_TEMPERATURE, (int)dht.readTemperature());
}


/**
 * This function sends the current humidity
 * to Blynk when Arduino receives a pull.
 */
BLYNK_READ(PIN_HUMIDITY)
{ 
  // This command sends the current state of the sensor
  Blynk.virtualWrite(PIN_HUMIDITY, (int)dht.readHumidity());
}


/**
 * Connect to Adafruit IO via MQTT
 */
void connect() {
  Serial.print("Connecting to Adafruit IO... ");
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println("Wrong protocol"); break;
      case 2: Serial.println("ID rejected"); break;
      case 3: Serial.println("Server unavail"); break;
      case 4: Serial.println("Bad user/pass"); break;
      case 5: Serial.println("Not authed"); break;
      case 6: Serial.println("Failed to subscribe"); break;
      default: Serial.println("Connection failed"); break;
    }
    if (ret >= 0) {
      mqtt.disconnect();
    }
    Serial.println("Retrying connection...");
    delay(5000);
  }
  Serial.println("Adafruit IO Connected!");
}


/**
 * Plays music for pet by iterating over notes of the melody
 */
void playMusic() {
  Serial.println("PLAYING MUSIC");
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    divider = melody[thisNote + 1]; // Calculates the duration of each note
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // Increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(SPEAKER_PIN, melody[thisNote], noteDuration*0.9);

    // Wait for the note duration before playing the next note.
    delay(noteDuration);
    
    // stop the waveform generation before the next note.
    noTone(SPEAKER_PIN);
  }
}
