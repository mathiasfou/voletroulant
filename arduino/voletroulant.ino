/*
*****************************************************
*****************************************************
*
*    VOLET ROULANT
*    
*   
*
*****************************************************
*****************************************************
*/
#include <ESP8266WiFi.h>
#include <AceButton.h>
using namespace ace_button;

// Paramètres du wifi
const char* ssid = "";
const char* password = "";

WiFiServer server(80);

// Attribution des PINs
const int BUTTON_Monter = 5;
const int BUTTON_Descendre = 4;
const int RELAY_Monter = 12;
const int RELAY_Descendre = 14;
const int LED = 2;

// Déclaration des boutons
AceButton buttonH;
AceButton buttonB;
void handleEvent(AceButton*, uint8_t, uint8_t);

// déclaration des variables
unsigned long lastAction = 0;
unsigned long StopDelay = 45000;
unsigned long RefusDelay = 900;
int actif = LOW;
int inactif = HIGH;
bool serveuractif = false;

// SETUP
void setup() {
  delay(1);
  pinMode(RELAY_Monter, OUTPUT);       // Définition des pins
  pinMode(RELAY_Descendre, OUTPUT);    // des relais en sorties
  pinMode(LED, OUTPUT);                // et de la led..
  digitalWrite(LED, inactif);
  arreter();
  pinMode(BUTTON_Monter, INPUT);       // Définition des pins
  pinMode(BUTTON_Descendre, INPUT);    // des boutons en entrées
  buttonH.init(BUTTON_Monter, LOW);    // Initialisation
  buttonB.init(BUTTON_Descendre, LOW); // des boutons

  ButtonConfig* buttonConfig = buttonH.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);

  // Connection au réseau WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED) {
    delay(4500);
  //}
  
}



void loop() {
  if (WiFi.status() == WL_CONNECTED and !serveuractif){
    server.begin(); // Démarrage du serveur
    serveuractif = true;
    digitalWrite(LED, actif);
  }
  if ((millis() - lastAction) > StopDelay) {arreter();}
  buttonH.check();
  buttonB.check();
  if (WiFi.status() == WL_CONNECTED and serveuractif){
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Wait until the client sends some data
  while (!client.available()) {
    delay(1);
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  client.flush();
  // Match the request
  if (req.indexOf("/monter") != -1) {
    monter();
  } else if (req.indexOf("/descendre") != -1) {
    descendre();
  } else if (req.indexOf("/stop") != -1) {
    arreter();
  } else {
    client.stop();
    return;
  }

  client.flush();
  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n" + String(millis() - lastAction) + "</html>\n";
  // Send the response to the client
  client.print(s);
  delay(1);
}
}

// The event handler for the button.
void handleEvent(AceButton* button , uint8_t eventType,
    uint8_t buttonState) {
    uint8_t pin = button->getPin();
    // Détection du type d'évènement sur les boutons
    switch (eventType) {
    case AceButton::kEventReleased: // Un bouton est relaché
      arreter();
      break;
    case AceButton::kEventPressed: // Un bouton est enfoncé
    // Détection du bouton actionné
      switch (pin) {
        case BUTTON_Descendre: // Bouton pour descendre
          descendre();
          break;
        case BUTTON_Monter: // Bouton pour monter
          monter();
          break;
      }
      break;
  }
}

void monter() {
  if ((millis() - lastAction) > RefusDelay) {
    lastAction = millis();
    digitalWrite(RELAY_Descendre, inactif);
    digitalWrite(RELAY_Monter, actif);
}}
void descendre() {
  if ((millis() - lastAction) > RefusDelay) {
    lastAction = millis();
    digitalWrite(RELAY_Monter, inactif);
    digitalWrite(RELAY_Descendre, actif);
}}
void arreter() {
    digitalWrite(RELAY_Descendre, inactif);
    digitalWrite(RELAY_Monter, inactif);
}
