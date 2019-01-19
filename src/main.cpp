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

  // Lecture sur le port série, écriture dans le fichier de log
  String ReadResponse() {
    String res = "";
    while (Serial.available()) {
      res += Serial.readString();
    }
    return res;
  }

  // Envoie d'une commande, attend que l'ESP réponde avec OK
  // Renvoie faux si le timeout est atteint sans avoir reçu "OK"
  bool SendCmd(String cmd, unsigned int timeout)
  {
    Serial.println(cmd); // Envoi de la commande
    String res;
    unsigned long StartTime = millis();
    int IsOK;
    while ((IsOK = res.indexOf("OK")) < 0 && (millis() - StartTime) <= timeout) {
      res = ReadResponse();
      if (res.length() > 1) // On évite d'écrire des lignes blanches dans les logs
        logFile.println(res);
      if (res.indexOf("busy") > 0) { // La commande précédente n'a pas encore fini
        delay(500);
        Serial.println(cmd); // Renvoi de la commande
        StartTime = millis(); // reset timeout
      }
    }
    logFile.println(""); // Saut d'une ligne
    if (IsOK < 0) // on est sorti de la boucle par timeout et non par OK
      return false;
    return true;
  }

  // boucle infinie qui fait clignoter la LED en cas d'erreur fatale
  void FatalError() {
    while(1) {
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }
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
    FatalError();
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
    FatalError();
  }
  delay(10);

  // Vérification que le module répond à "AT" => PEUT DEMANDER PLUSIEURS ESSAIS
  logFile.println("Testing Module...");
  for (size_t i = 0; i < 6; i++) {
    if (SendCmd("AT", 500)) {
      logFile.println("Module responded accordingly !");
      break;
    }
    if (i >= 5) {
      logFile.println("Module not responding.");
      logFile.println("--- End log ---");
      logFile.close();
      FatalError();
    }
  }

  // ESP8266 en mode Station
  logFile.println("Setting module to Station mode...");
  if (SendCmd("AT+CWMODE=1", 300))
    logFile.println("Set.");
  else {
    logFile.println("Something went wrong...");
    logFile.println("--- End log ---");
    logFile.close();
    FatalError();
  }

  // Déconnexion du WiFi
  logFile.println("Disconnecting from WiFi...");
  if (SendCmd("AT+CWQAP", 1000))
    logFile.println("Disconnected.");
  else {
    logFile.println("Something went wrong...");
    logFile.println("--- End log ---");
    logFile.close();
    FatalError();
  }

  // Connexion au WiFi => CETTE COMMANDE DEMANDE BEAUCOUP DE TEMPS
  logFile.println("Connecting to WiFi with credentials: ");
  logFile.print("\tSSID = ");
  logFile.println(ssid);
  logFile.print("\tPWD = ");
  logFile.println(pass);
  String cmd = "AT+CWJAP=\"";
  cmd = cmd + ssid + "\",\"" + pass + "\"";
  if (SendCmd(cmd, 10000))
    logFile.println("Connected.");
  else {
    logFile.println("Unable to connect.");
    logFile.println("--- End log ---");
    logFile.close();
    FatalError();
  }

  // Récupération de l'adresse IP => PEUT PRENDRE BCP DE TEMPS
  logFile.println("Receiving IP address...");
  if (SendCmd("AT+CIFSR", 10000))
    logFile.println("IP looks good !");
    else {
      logFile.println("Unable to get IP address from DHCP server.");
      logFile.println("--- End log ---");
      logFile.close();
      FatalError();
    }

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
