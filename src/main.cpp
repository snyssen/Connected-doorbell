/*************************************************************
  Premiers essais d'utilisation de blynk avec écriture dans un fichier de logs sur carte SD.
  Le projet utilise un Atmega328 comme µC principal, et un ESP8266 comme shield wifi.

  Copyright 2018 Nyssen Simon
 *************************************************************/

#include <Arduino.h>
//#include <ESP8266_Lib.h>
//#include <BlynkSimpleShieldEsp8266.h>
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
  // LED
const uint8_t led = 2;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "121980e911c747f8802b79d8271f451d";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Condroz-C15-FO";
char pass[] = "condrozc152017";

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

//ESP8266 wifi(&Serial);

File logFile;

/*************************************************************

  DEFINITION DES FONCTIONS

  *************************************************************/

  bool SendCmd(String cmd, int timeout)
  {
    int i=0;
    while(1)
    {
      if (logFile) {
        logFile.print("Sending CMD = ");
        logFile.println(cmd);
      }
      Serial.println(cmd);
      while(Serial.available())
      {
        if(Serial.find("OK"))
          return true;
      }
      delay(timeout);
      if(i>5)
        break;
      i++;
    }
    return false;
  }

/*************************************************************

  SETUP & LOOP

  *************************************************************/

void setup()
{
  // Connexion du bouton
	pinMode(button,INPUT);
  // Connexion de la LED
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
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

  logFile.println("Testing Module...");
  if (SendCmd("AT", 100))
    logFile.println("Module responded accordingly !");
  else {
    logFile.println("Module not responding.");
    logFile.println("--- End log ---");
    logFile.close();
    while (1);
  }

  // Récupère la version du firmware
  logFile.println("Querying module firmware version...");
  logFile.print("Sending cmd = ");
  logFile.println("AT+GMR");
  Serial.println("AT+GMR");
  delay(10);
  while (Serial.available()) {
    logFile.print(Serial.readString());
  }
  logFile.println("");

  // ESP8266 en mode Station
  logFile.println("Setting module to Station mode...");
  if (SendCmd("AT+CWMODE=1", 100))
    logFile.println("Set.");
  else {
    logFile.println("Something went wrong...");
    logFile.println("--- End log ---");
    logFile.close();
    while (1);
  }

  // Déconnexion du WiFi
  logFile.println("Disconnecting from WiFi...");
  if (SendCmd("AT+CWQAP", 100))
    logFile.println("Disconnected.");
  else {
    logFile.println("Something went wrong...");
    logFile.println("--- End log ---");
    logFile.close();
    while (1);
  }

  // Connexion au WiFi
  logFile.println("Connecting to WiFi with credentials: ");
  logFile.print("\tSSID = ");
  logFile.println(ssid);
  logFile.print("\tPWD = ");
  logFile.println(pass);
  String cmd = "AT+CWJAP=\"";
  cmd = cmd + ssid + "\",\"" + pass + "\"";
  if (SendCmd(cmd, 7000))
    logFile.println("Connected.");
  else {
    logFile.println("Unable to connect.");
    logFile.println("--- End log ---");
    logFile.close();
    while (1);
  }

  // Récupération de l'adresse IP
  logFile.println("Receiving IP address...");
  logFile.print("Sending cmd = ");
  logFile.println("AT+CIFSR");
  Serial.println("AT+CIFSR");
  delay(10);
  while (Serial.available()) {
    logFile.print(Serial.readString());
  }
  logFile.println("");

  logFile.println("Setup done !\nClosing file...");
  logFile.println("--- End log ---");
  logFile.close();
  // blink LED pour montrer que le setup est terminé
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  // Ouverture des logs post-setup

}

void loop()
{

  logFile = SD.open("run.log", FILE_WRITE);
  while (!logFile);
	logFile.println("--- Begin log ---");
  logFile.println("--- End log ---");
  logFile.close();
  delay(100);
}
