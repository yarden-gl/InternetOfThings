/**
 * In this assignment we control led lights through the blynk app. 
 * In the app we've constructed a page with 4 buttons, 2 joysticks and one slider.
 * On the matrix we have 5 led lights - 2 yellow, 2 green and 1 red.
 * You can control the red led light using the slider. 
 * You can turn the yellow and green lights on and off using the their respective buttons or play with their intensity using the joysticks.
 **/
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define WIFI_SSID "Partner.Home-3B46"
#define WIFI_PASSPHRASE "btermt42"

char auth[] = "h4ul5UjiLBpGzDgYIsQ78nizLfcIEjLT";

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, WIFI_SSID, WIFI_PASSPHRASE);
}

void loop() {
  Blynk.run();
}
