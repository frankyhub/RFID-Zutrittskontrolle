
/*************************************************************************************************
                                      PROGRAMMINFO
**************************************************************************************************
  Funktion: RFID Zutrittskontrolle
**************************************************************************************************
  Version: 09.02.2025
**************************************************************************************************
  Board: DOIT ESP32 DEVKIT V1

**************************************************************************************************
  C++ Arduino IDE V2.3.4
**************************************************************************************************
  Einstellungen:
  https://dl.espressif.com/dl/package_esp32_index.json
  http://dan.drown.org/stm32duino/package_STM32duino_index.json
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json

  Librarys
  ESPASyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
  AsyncTCP: https://github.com/me-no-dev/asynctcp

MFRC522     RFID Reader	        ESP32	Description
SDA	        GPIO 5	            SPI signal input,   I2C data line, or UART data input
SCK	        GPIO 18	            SPI clock
MOSI	      GPIO 23	            SPI data input
MISO	      GPIO 19	            SPI master-in-slave-out, I2C serial clock, or UART serial output
IRQ	        Don’t connect	      Interrupt pin; signals the microcontroller when an RFID tag is nearby
GND	        GND	
RST	        GPIO 21	             LOW signal to put the module in power-down mode; send a HIGH signal to reset the module
3.3V	      3.3V	               Power supply (2.5-3.3V)


MicroSD card module	ESP32
3V3	                3.3V
CS	                GPIO 15
MOSI	              GPIO 23
CLK	                GPIO 18
MISO	              GPIO 19
GND	                GND

LED —  GPIO 22 (mit 220 Ohm) Freigabe, Zugang gewährt
Buzzer — GPIO 4 (mit 220 Ohm) Unbekannter User, kein Zugang


**************************************************************************************************/
#include <Arduino.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
//#include <MFRC522DriverI2C.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <time.h>
#include <WiFi.h>

// L
MFRC522DriverPinSimple ss_pin(5);

MFRC522DriverSPI driver{ss_pin}; // SPI driver
//MFRC522DriverI2C driver{};     // I2C driver
MFRC522 mfrc522{driver};         // MFRC522 Instance

// WLAN Zugangsdaten
const char* ssid = "R2-D2";
const char* password = "xxx";

long timezone = 0;
byte daysavetime = 1;

// Erstellen eines AsyncWebServer-Objekts an Port 80
AsyncWebServer server(80);

const char* PARAM_INPUT_1 = "uid";
const char* PARAM_INPUT_2 = "person";
const char* PARAM_INPUT_3 = "role";
const char* PARAM_INPUT_4 = "delete";
const char* PARAM_INPUT_5 = "delete-user";

String inputMessage;
String inputParam;

const int ledPin = 22; //Freigabe, Zugang gewährt
const int buzzerPin = 4; //Unbekannter User, kein Zugang

// Auf die SD-Karte schreiben
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Datei schreiben: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Fehler beim Öffnen der Datei");
    return;
  }
  if(file.print(message)) {
    Serial.println("Datei geschrieben");
  } else {
    Serial.println("Fehler beim Schreiben");
  }
  file.close();
}

// Anhängen von Daten an die SD-Karte
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("An Datei anhängen: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Fehler beim Öffnen der Datei");
    return;
  }

  time_t t = file.getLastWrite();
  struct tm *tmstruct = localtime(&t);

  char bufferDate[50]; // Passe die Puffergröße nach Bedarf an
  snprintf(bufferDate, sizeof(bufferDate), "%d-%02d-%02d", 
          (tmstruct->tm_year) + 1900, 
          (tmstruct->tm_mon) + 1, 
          tmstruct->tm_mday);
  char bufferTime[50]; // Passe die Puffergröße nach Bedarf an
  snprintf(bufferTime, sizeof(bufferTime), "%02d:%02d:%02d", 
          tmstruct->tm_hour, 
          tmstruct->tm_min, 
          tmstruct->tm_sec);
          
  String lastWriteTime = bufferDate;
  String finalString = String(bufferDate) + "," + String(bufferTime) + "," + String(message) + "\n";
  Serial.println(lastWriteTime);
  if(file.print(finalString.c_str())) {
    Serial.println("Daten angehängt");
  } else {
    Serial.println("Fehler beim Schreiben");
  }
  file.close();
}

// Anhängen von Daten an die SD-Karte
void appendUserFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("An Datei anhängen: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Fehler beim Öffnen der Datei");
    return;
  }

  String finalString = String(message) + "\n";

  if(file.print(finalString.c_str())) {
    Serial.println("Daten angehängt");
  } else {
    Serial.println("Fehler beim Schreiben");
  }
  file.close();
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Datei löschen: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("Datei gelöscht");
  } else {
    Serial.println("Löschen fehlgeschlagen");
  }
}

String processor(const String& var){
  return String("HTTP GET-Anfrage an den ESP gesendet (" 
                + inputParam + ") mit Wert: " + inputMessage +
                "<br><a href=\"/\"><button class=\"button button-home\">Zur&uuml;ck zur Startseite</button></a>");
}

void deleteLineFromFile(const char* filename, int lineNumber) {
  File file = SD.open(filename);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei zum Lesen.");
    return;
  }

  // Lese  alle Zeilen außer der zu löschenden Zeile
  String lines = "";
  int currentLine = 0;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (currentLine != lineNumber) {
      lines += line + "\n";
    }
    currentLine++;
  }
  file.close();

  // Alle Zeilen außer der gelöschten zurückschreiben
  file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei.");
    return;
  }

  file.print(lines);
  file.close();
  Serial.println("Zeile erfolgreich gelöscht.");
}

String getRoleFromFile(const char* filename, String uid) {
  File file = SD.open(filename);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei.");
    return "";
  }

  // Überspringen der Kopfzeile
  file.readStringUntil('\n');
  
  // Lese jede Zeile und prüfe sie auf UID
  while (file.available()) {
    String line = file.readStringUntil('\n');
    
    int commaIndex = line.indexOf(',');
    if (commaIndex > 0) {
      String fileUID = line.substring(0, commaIndex);
      String role = line.substring(commaIndex + 1);

      // UID vergleichen
      if (fileUID == uid) {
        file.close();
        role.trim();  // Entferne alle zusätzlichen Leerzeichen oder Zeilenumbrüche
        return role;
      }
    }
  }
  file.close();
  return "";  // Gibt einen leeren String zurück, wenn die UID nicht gefunden wird
}

void initRFIDReader() {
  mfrc522.PCD_Init();    // Init MFRC522
  MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);	// Details zu PCD - MFRC522 Card Reader Details anzeigen.
	Serial.println(F("PICC scannen, um UID zu sehen"));
}

void initLittleFS() {
  if(!LittleFS.begin()){
    Serial.println("Beim Mounten von LittleFS ist ein Fehler aufgetreten");
        return;
  }
}
void initWifi() {
  // WiFi Verbinden
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbinde mit WiFi...");
  }
  // ESP32 Lokale IP-Adresse
  Serial.print("ESP IP Addresse: ");
  Serial.println(WiFi.localIP());
}

void initTime() {
  Serial.println("Initialisierungszeit");
  struct tm tmstruct;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  Serial.printf(
    "Uhrzeit und Datum ist jetzt : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min,
    tmstruct.tm_sec
  );
}

void initSDCard() {
  // CS pin = 15
  if (!SD.begin(15)) {
    Serial.println("Karten-Zugriff fehlgeschlagen");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("Keine SD-Karte angeschlossen");
    return;
  }

  Serial.print("SD Card Typ: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("unbekannt");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Größe der SD-Karte: %lluMB\n", cardSize);

  // Wenn die log.txt Datei nicht vorhanden ist, erstelle  eine Datei auf der SD-Karte und schreibe den Header
  File file = SD.open("/log.txt");
  if(!file) {
    Serial.println("log.txt Datei existiert nicht");
    Serial.println("Erstelle die Datei...");
    writeFile(SD, "/log.txt", "Date,Time,UID,Role\r\n");
  }
  else {
    Serial.println("log.txt Datei existiert bereits.");  
  }
  file.close();

  // Wenn die users.txt Datei nicht vorhanden ist, erstelle eine Datei auf der SD-Karte und schreibe den Header
  file = SD.open("/users.txt");
  if(!file) {
    Serial.println("users.txt Datei existiert nicht");
    Serial.println("Erstelle die Datei...");
    writeFile(SD, "/users.txt", "UID,Role\r\n");
  }
  else {
    Serial.println("users.txt Datei existiert bereits.");  
  }
  file.close();
}

void setup() {
  Serial.begin(115200);  // Initialisieren der seriellen Kommunikation
  while (!Serial);       
  
  initRFIDReader();
  initLittleFS();
  initWifi();
  configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  initTime();
  initSDCard();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Root / Webseite
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/full-log.html");
  });
  // Root/Add-User-Webseite
  server.on("/add-user", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/add-user.html");
  });
  // Root / Webseite "manage-users"
  server.on("/manage-users", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/manage-users.html");
  });

  // Statische Dateien bereitstellen
  server.serveStatic("/", LittleFS, "/");

  // Lädt die log.txt Datei
  server.on("/view-log", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/log.txt", "text/plain", false);
  });
  // Lädt die users.txt Datei
  server.on("/view-users", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/users.txt", "text/plain", false);
  });
  
  //  HTTP GET-Anfragen auf <ESP_IP>/get?input= <inputMessage>empfangen
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
     if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)&& request->hasParam(PARAM_INPUT_3)) {    
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = String(PARAM_INPUT_1);
      inputMessage += " " + request->getParam(PARAM_INPUT_2)->value();
      inputParam += " " + String(PARAM_INPUT_2);
      inputMessage += " " + request->getParam(PARAM_INPUT_3)->value();
      inputParam += " " + String(PARAM_INPUT_3);



     String finalMessageInput = String(request->getParam(PARAM_INPUT_1)->value()) + "," + String(request->getParam(PARAM_INPUT_2)->value())+ "," + String(request->getParam(PARAM_INPUT_3)->value());
      appendUserFile(SD, "/users.txt", finalMessageInput.c_str());
    }
    else if (request->hasParam(PARAM_INPUT_4)) {
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = String(PARAM_INPUT_4);
      if(request->getParam(PARAM_INPUT_4)->value()=="users") {
        deleteFile(SD, "/users.txt");
      }


      else if(request->getParam(PARAM_INPUT_4)->value()=="log") {
        deleteFile(SD, "/log.txt");
      }
    }
    else if (request->hasParam(PARAM_INPUT_5)) {
      inputMessage = request->getParam(PARAM_INPUT_5)->value();
      inputParam = String(PARAM_INPUT_5);
      deleteLineFromFile("/users.txt", inputMessage.toInt());
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send(LittleFS, "/get.html", "text/html", false, processor);
  });

  // Start Server
  server.begin();
}

void loop() {
	// Setze die Schleife zurück, wenn keine neue Karte auf dem Sensor/Lesegerät vorhanden ist. Dies spart den gesamten Prozess im Leerlauf.
	if (!mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Wähle eine der Karten aus.
	if (!mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  // Speichern der UID in einer String-Variablen
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidString += "0"; 
    }
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("Card UID: ");
  Serial.println(uidString);

  String role = getRoleFromFile("/users.txt", uidString);
  if (role != "") {
    Serial.print("Status der UID: ");
    Serial.print(uidString);
    Serial.print(" ist ");
    Serial.println(role);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);

  } else {
    role = "unbekannt";
    Serial.print("UID: ");
    Serial.print(uidString);
    Serial.println(" Nicht gefunden, setze die UID auf unbekannt");
  digitalWrite(buzzerPin, HIGH);
  delay(2500);
  digitalWrite(buzzerPin, LOW);
  }
  String sdMessage = uidString + "," + role;
  appendFile(SD, "/log.txt", sdMessage.c_str());

}
