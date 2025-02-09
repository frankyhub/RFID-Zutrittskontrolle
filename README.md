<a name="oben"></a>

<div align="center">

|[:skull:ISSUE](https://github.com/frankyhub/RFID-Zutrittskontrolle/issues?q=is%3Aissue)|[:speech_balloon: Forum /Discussion](https://github.com/frankyhub/RFID-Zutrittskontrolle/discussions)|[:grey_question:WiKi](https://github.com/frankyhub/RFID-Zutrittskontrolle/wiki)||
|--|--|--|--|
| | | | |
|![Static Badge](https://img.shields.io/badge/RepoNr.:-%20104-blue)|<a href="https://github.com/frankyhub/RFID-Zutrittskontrolle/issues">![GitHub issues](https://img.shields.io/github/issues/frankyhub/RFID-Zutrittskontrolle)![GitHub closed issues](https://img.shields.io/github/issues-closed/frankyhub/RFID-Zutrittskontrolle)|<a href="https://github.com/frankyhub/RFID-Zutrittskontrolle/discussions">![GitHub Discussions](https://img.shields.io/github/discussions/frankyhub/RFID-Zutrittskontrolle)|<a href="https://github.com/frankyhub/RFID-Zutrittskontrolle/releases">![GitHub release (with filter)](https://img.shields.io/github/v/release/frankyhub/RFID-Zutrittskontrolle)|
|![GitHub Created At](https://img.shields.io/github/created-at/frankyhub/RFID-Zutrittskontrolle)| <a href="https://github.com/frankyhub/RFID-Zutrittskontrolle/pulse" alt="Activity"><img src="https://img.shields.io/github/commit-activity/m/badges/shields" />| <a href="https://github.com/frankyhub/RFID-Zutrittskontrolle/graphs/traffic"><img alt="ViewCount" src="https://views.whatilearened.today/views/github/frankyhub/github-clone-count-badge.svg">  |<a href="https://github.com/frankyhub?tab=stars"> ![GitHub User's stars](https://img.shields.io/github/stars/frankyhub)|
</div>

# RFID-Zutrittskontrolle


## Story
Dieses Projekt umfasst eine Zutrittskontrolle mit einem RFID-Benutzerverwaltungssystem auf einem Webserver. Der ESP32 ist mit einem MFRC522 RFID-Lesegerät und einem microSD-Karten Modul verbunden. Auf der SD-Karte werden alle Benutzerdaten gespeichert. Der WEB-Server umfasst ein Zutritts-Protokoll in dem alle Zutritte mit Datum, Uhrzeit, UID (UID = Unique Identification), Name und Status (User oder Admin) gespeichert sind. Nicht erfasste RFID-Karten haben den Status "unbekannt".
Zutrittsberechtigte Personen erfasst man auf der WEB-Seite "Mitglieder". Auf dieser Seite speichert man die UID, den Namen und den Status der zutrittsberechtigten Person. In der Mitglieder-Verwaltung sind alle zutrittsberechtigten Personen gelistet. Auf dieser Seite besteht auch die Möglichkeit zutrittsberechtigte Personen wieder zu löschen.
Wird Zugang gewährt, geht der GPIO 22 auf HIGH und erteilt die Freigabe (z.B. Relais), der GPIO 4 zeigt einen unbekannten User an. Ein Zutritt wird nicht erteilt.

---

![Bild](pic/Protokoll.png)

![Bild](pic/Verwaltung.png)

![Bild](pic/Mitglieder.png)

---

## Hardware

| Anzahl | Bezeichnung | 
| -------- | -------- | 
| 1        | ESP32 DOIT DEVKIT V1 Board       | 
| 1        | MFRC522 RFID Reader/Writer        | 
| 1        | MicroSD Card Module        | 
| 1        | MicroSD Card 16GB       | 
| 2        | 5mm LED   (Relais)     | 
| 2        | 220 Ohm    |
| -------- | -------- | 

---

## MicroSD - ESP32

| MicroSD | 	ESP32| 
| -------- | -------- | 
|3V3	 |               3.3V|
|CS	  |              GPIO 15|
|MOSI	 |             GPIO 23|
|CLK	 |               GPIO 18|
|MISO	 |             GPIO 19|
|GND	|                GND|
| -------- | -------- | 

---

## RFID Reader - ESP32

| RFID Reader	 | 	ESP32| 
| -------- | -------- | 
|SDA	  |      GPIO 5	  |       
|SCK	  |      GPIO 18	  |          
|MOSI	  |    GPIO 23	    |       
|MISO	  |    GPIO 19	   |         
|IRQ	  |     NC	|     
|GND	  |      GND	|
|RST	  |      GPIO 21	 |          
|3.3V	  |    3.3V	    |           
| -------- | -------- | 

---

<div style="position:absolute; left:2cm; ">   
<ol class="breadcrumb" style="border-top: 2px solid black;border-bottom:2px solid black; height: 45px; width: 900px;"> <p align="center"><a href="#oben">nach oben</a></p></ol>
</div>  

---

