/*************************************************************
  Sonnette connectée, envoie une requête sur un serveur NodeJS qui prévient avec des sockets les utilisateurs connectés que quelqu'un a sonné à la porte.
  Ecriture dans un fichier de logs sur carte SD.
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
  // LED
const uint8_t led = 2;

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "Condroz-C15-FO";
//char pass[] = "condrozc152017";
char ssid[] = "Nyssen's family";
char pass[] = "bioleux4122";

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

File logFile;

/*************************************************************

  DEFINITION DES FONCTIONS

  *************************************************************/

  // vide le buffer en lisant sans log
  void EmptyBuffer() {
    while (Serial.available()) {
      Serial.read();
    }
  }

  // Envoie d'une commande, attend que l'ESP réponde avec OK
  // Le timeout détermine après combien de temps renvoyer la commande si il n'y a pas de réponse (max 5x)
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
        if(Serial.find("OK")) {
          EmptyBuffer();
          return true;
        }
      }
      delay(timeout);
      if(i>5)
        break;
      i++;
    }
    EmptyBuffer();
    return false;
  }

  // Lecture sur le port série, écriture dans le fichier de log
  String ReadResponse(unsigned int timeout) {
    unsigned long StartTime = millis();
    String res = "";
    while (Serial.available() && (millis() - StartTime) <= timeout) {
      res += Serial.readString();
    }
    return res;
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
  logFile.println(ReadResponse(1000));
  logFile.println(""); // Saut d'une ligne

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
  delay(100);

  // Récupération de l'adresse IP
  logFile.println("Receiving IP address...");
  logFile.print("Sending cmd = ");
  logFile.println("AT+CIFSR");
  String res;
  do {
    Serial.println("AT+CIFSR");
    res = ReadResponse(1000);
    logFile.println(res);
    delay(200);
  } while(res.indexOf("OK") < 0); // Le module répond "busy" si il n'a pas fini avec la commande précédente
  logFile.println("");

  logFile.println("Setup done !\nClosing file...");
  logFile.println("--- End log ---");
  logFile.close();
  // On allume la LED pour montrer que le setup est terminé
  digitalWrite(led, HIGH);
}

void loop()
{
  if (!digitalRead(button)) {
    digitalWrite(led, LOW);
    logFile = SD.open("run.log", FILE_WRITE);
    while (!logFile);
  	logFile.println("--- Begin log ---");
    logFile.println("--- End log ---");
    logFile.close();
    delay(100);
    digitalWrite(led, HIGH);
  }
}
