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
char ssid[] = "Condroz-C15-FO";
char pass[] = "condrozc152017";
//char ssid[] = "Nyssen's family";
//char pass[] = "bioleux4122";

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

File logFile;

/*************************************************************

  DEFINITION DES FONCTIONS

  *************************************************************/

  // boucle infinie qui fait clignoter la LED en cas d'erreur fatale
  void FatalError() {
    while(1) {
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }
  }

  // Lecutre sur le port série
  String ReadResponse() {
    String res = "";
    while (Serial.available()) {
      res += Serial.readString();
    }
    return res;
  }

  // Permet de vider le buffer tout en gardant une trace de ce qui s'y trouvait
  // => utilisé au démarrage pour se débarasser des messages du démarrage de l'ESP
  void ReadFor(unsigned int timeout) {
    String res;
    unsigned long StartTime = millis();
    while ((millis() - StartTime) <= timeout) {
      res = ReadResponse();
      if (res.length() > 1) // On évite d'écrire des lignes blanches dans les logs
        logFile.println(res);
    }
  }

  // lit le buffer jusqu'à ce qu'on rencontre le string recherché ou que le timeout soit atteint
  bool ReadTil(unsigned int timeout, String ToFind) {
    String res;
    unsigned long StartTime = millis();
    int Found = -1;
    while (Found < 0 && (millis() - StartTime) <= timeout) {
      res = ReadResponse();
      if (res.length() > 1) // On évite d'écrire des lignes blanches dans les logs
        logFile.println(res);
      Found = res.indexOf(ToFind);
    }
    if (Found < 0) {
      logFile.println("timeout reached !");
      return false;
    }
    return true;
  }

  // Envoie une commande et attend le OK
  bool SendCmd(String cmd, unsigned int timeout) {
    logFile.println("Sending -> " + cmd);
    Serial.println(cmd);
    return ReadTil(timeout, "OK");
  }

  // Vérifie si on est connecté au wifi
  void ConnectToWifi() {
    Serial.println("AT+CWJAP?");
    if (ReadTil(5000, ssid)) { // on vérifie si il renvoie le ssid sur lequel on est censé être connecté
      // On est déjà connecté au wifi
      ReadFor(1000); // On vide le buffer
    }
    else {
      // On est pas connecté au wifi
      // set en ESP en station
      SendCmd("AT+CWMODE=1", 5000);
      // Il faudra peut être vider le buffer ici
      // Tentative de connexion au WiFi
      String cmd = "AT+CWJAP=\"";
      cmd = cmd + ssid + "\",\"" + pass + "\"";
      if (!SendCmd(cmd, 20000)) {
        // Impossible de se connecter
        logFile.println("Can't connect to Wifi.");
        logFile.println("--- End log ---");
        logFile.close();
        FatalError();
      }
      // Connexion OK
      ReadFor(1000); // On vide le buffer
    }

    // Vérif IP
    for (uint8_t i = 0; i < 4; i++) {
      Serial.println("AT+CIFSR");
      if (ReadTil(5000, "+CIFSR:STAIP")) {
        // On a une IP
        break;
      }
      if (i >= 3) {
        // On a tjs pas d'IP après 3 essais
        logFile.println("Can't get IP address");
        logFile.println("--- End log ---");
        logFile.close();
        FatalError();
      }
    }
    // Tout est OK
    ReadFor(1000); // On vide le buffer
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
  while(!Serial);
  logFile.println("Serial connected !");

  // On lit les messages de démarrage de l'ESP
  ReadFor(10000);

  // On se connecte au WiFi
  ConnectToWifi();

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

    // Etablissement de la connexion TCP
    SendCmd("AT+CIPMUX=0", 5000); // Pas de multiplexage
    SendCmd("AT+CIPSTART=\"TCP\",\"172.16.100.166\",1337,1", 5000); // dernier arg ("1") précise qu'on veut activer le keep alive

    // Envoi de la requête
    String request = "GET /ring HTTP/1.1\r\n\r\n";
    logFile.println(request);
    // CIPSEND fonctionne en attendant le nombre de bytes indiqués en arg avant d'envoyer le message
    SendCmd("AT+CIPSEND=" + String(request.length()), 1000); // +4 pour tenir compte des deux \r\n de fin de requête
    ReadFor(1000);
    Serial.print(request);
    if (!ReadTil(10000, "SEND OK"))
      logFile.println("Error sending request !");

    // Réception
    ReadTil(30000, "data=");
    ReadFor(5000);

    // Fermeture connexion TCP
    SendCmd("AT+CIPCLOSE", 1000);

    logFile.println("--- End log ---");
    logFile.close();
    delay(100);
    digitalWrite(led, HIGH);
  }
}
