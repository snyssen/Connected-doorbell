/*************************************************************
  Premiers essais d'utilisation de blynk avec écriture dans un fichier de logs sur carte SD.
  Le projet utilise un Atmega328 comme µC principal, et un ESP8266 comme shield wifi.

  Copyright 2018 Nyssen Simon
 *************************************************************/

#include <Arduino.h>

#include <SPI.h>
#include <SD.h>

/*************************************************************

  DEFINITION DES VARIABLES

  *************************************************************/
// Définition des pins
	// bouton
const uint8_t button = 8;
	// SD
const uint8_t chipSelect = 10;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "121980e911c747f8802b79d8271f451d";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Nyssen's family";
char pass[] = "bioleux4122";

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

//ESP8266 wifi(&Serial);

File logFile;

/*************************************************************

  DEFINITION DES FONCTIONS

  *************************************************************/

// Test si le module répond à la commande AT
bool testModule()
{
  int timeout = 0;
  logFile.println("Testing Module...");
  logFile.print("Sending cmd = ");
  logFile.println("AT");
  Serial.println("AT");
  while (1) {
    if (timeout >= 20)
      return false;
    if (Serial.find("OK"))
      return true;
    delay(10);
    timeout++;
  }
}

/*************************************************************

  SETUP & LOOP

  *************************************************************/

void setup()
{
  // Connexion du bouton
	pinMode(button,INPUT);
  // Connexion de la carte SD
	pinMode(chipSelect, OUTPUT);
  if (!SD.begin()) {
		while(1);
	}
  logFile = SD.open("setup.log", FILE_WRITE);
  while (!logFile);
	logFile.println("--- Begin log ---");

  // Set ESP8266 baud rate
  logFile.print("Connecting to ESP8266 with baudrate = ");
  logFile.println(ESP8266_BAUD);
  Serial.begin(ESP8266_BAUD);
  delay (10);
  if (Serial)
    logFile.println("Connected.");
  else {
    logFile.println("Impossible to connect.");
    logFile.println("--- End log ---");
    logFile.close();
    while (1);
  }
  delay(10);


  if (testModule())
    logFile.println("Module responded accordingly !");
  else {
    logFile.println("Module not responding.");
    logFile.println("--- End log ---");
    logFile.close();
    while (1);
  }

  logFile.println("Querying module firmware version...");
  logFile.print("Sending cmd = ");
  logFile.println("AT+GMR");
  Serial.println("AT+GMR");
  delay(10);
  while (Serial.available()) {
    logFile.print(Serial.read());
  }

  logFile.println("Connecting to Blynk server...");
  //Blynk.begin(auth, wifi, ssid, pass);
  logFile.println("Setup done !\nClosing file...");
  logFile.println("--- End log ---");
  logFile.close();
}

void loop()
{
  //Blynk.run();
}
