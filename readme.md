# __Programm zum Auslesen der Brutto- und Nettogewichte fuer die Waage `Rhewa 82basic` per COM-Port__

## Idee

Das Programm `zwfwaage.exe` verbindet sich ueber den COM-Port mit der Waage Rhewa 82basic und loest eine Gewichtserfassung aus. Das gemesse Gewicht wird in eine Ganzzahl (Integer, Komma wird entfernt) umgerechnet und ist der Rueckgabewert des Programms, der von der aufrufenden Baan-Session verarbeitet wird.
Im Quellcode ist bereits vorgesehen, dass die Ausgabe des Geraetes auch als XML abgespeichert werden kann. Bei Bedarf kann eine Implementierung schnell realisiert werden.


## Verwendete Programme, Bibliotheken usw.
* [Boost C++ Libraries](http://www.boost.org/ "Boost C++ Libraries")
* [TinyXML](http://www.grinninglizard.com/tinyxml "TinyXML")
* [serial-port](http://gitorious.org/serial-port "serial-port")

## Nutzunung des Programms

Ein Beispielaufruf des Programms kann wie folgt aussehen:

	zwfwaage.exe

Dieser Aufruf erzeugt im Hauptordner des Programms die Datei `last`, die die Messdaten der Rhewa 82basic beinhaltet.
Je nachdem ob mit Tara gearbeitet wurde koennen die entstehenden Dateien wie folgt aussehen:

### Ausgabe ueber COM-Port



## Notwendige Einstellungen an der Waage

Zur Benutzung des Programms `zwfwaage.exe` muessen folgende Einstellungen an der Konfiguration der Waage vorgenommen werden:

* COM-Port Einstellungen des Programms entsprechend der Geraetekonfiguration unter `G.KONFI` -> `INTERFACE` -> ... einstellen
* Handshake fuer COM-Port-Verbindung auf `none` setzen (im Menue `G.KONFI` -> `INTERFACE` -> `HANDSH`)
* Format des Drucks auf `UNIVER` (Universell) stellen (im Menue `G.KONFI` -> `DRUCK` -> `DRUCK.EI` -> `FORMAT`)
* Anzahl der Datensatzwiederholungen auf `1` setzen (im Menue `G.KONFI` -> `DRUCK` -> `DRUCK.EI` -> `ANZAHL`)
* EDV-Kommunikation des Gereates auf `aktiv` setzen (im Menue `G.KONFI` -> `DRUCK` -> `EDV.KOM` -> `AKTIV`)

## Anforderungen

* Rechner mit verfuegbarem seriellen COM-Port
* Windows XP, Windows Vista oder Windows 7
* Bearbeitung der Projektdatei mit Codeblocks (IDE) und Kompilierung mit der C++ Boost Bibliothek

## Kommandozeilenargumente

Das Programm `zwfwaage.exe` kann mit folgenden Startparametern ausgefuehrt werden:

* `--help, --h` Listet alle zulaessigen Kommandozeilenargumente und ihre Funktionsbeschreibung auf
* `--name, --n` Angabe des Dateinamens, in dem die Daten der Messung gespeichert werden.
* `--console, --c` Startet des Programm im interaktiven Konsolenmodus
* `--p, --p` Angabe des COM-Ports z.B. --p COM1 oder --p /dev/ttyS0, Standard ist COM1
* `--baudrate, --b` Angabe der Uebertragungsgeschwindigkeit des COM-Ports, Standard ist 9600
* `--file, --f` Angabe des Pfades zu einer Konfigurationsdatei

## Interaktiver Konsolenmodus

Durch den Start des Programms im interaktiven Konsolenmodus kann ueber spezifische COM-Port-Befehle eine direkte Kommunikation mit der `Rhewa 82basic` hergestellt werden.
Die Rueckantwort des Geraetes wird direkt in der Konsole ausgegeben.

Momentan stehen folgende Befehle zur Verfuegung:

* `<FP>` Gibt die aktuellen Messwerte der Waage in der Konsole aus
* `exit` Beendigung des Programms

##Konfigurationsoptionen

Zur Konfiguration der COM-Port Verbindungsdaten bietet `zwfwaage.exe` verschiedene Moeglichkeiten:


### Erstellung einer Autokonfigurationsdatei

Die Konfigurationsdatei muss den Namen `config` haben und sich im gleichen Ordner wie die Datei `zwfwaage.exe` befinden. Der Aufbau der Datei sieht dabei folgende Struktur vor:
	
	port:COM3
	baudrate:115200

Die Reihenfolge von Port- und Baudratenangabe kann beliebig vertauscht werden.


### Verwendung des Kommandozeilenarguments `--f`

Mit Hilfe des Kommandozeilenarguments `--f` kann ein individueller Pfad zu einer Konfigurationsdatei angegeben werden (siehe Abschnitt Kommandozeilenargumente).
Die Struktur der Konfigurationsdatei enspricht dabei der Struktur der Autokonfigurationsdatei.

### Verwendung der Kommandozeilenargumente `--p` und `--b`

Mit Hilfe der Kommandozeilenargumente `--p` und `--b` koennen die Verbindungsparameter des Programms direkt beim Aufruf angegeben werden (siehe Abschnitt Kommandozeilenargumente).



### ANMERKUNGEN

Wenn die Kommandozeilenargumente `--p` oder `--b` nicht verwendet werden bezieht das Programm die Verbindungsdaten aus der Konfigurationsdatei. Dabei hat die Konfigurationsdatei, die mit dem Kommandozeilenargument `--f` angegeben wurde Vorrang vor der Autokonfigurationsdatei im Hauptornder des Programms.
Existiert keine Autokonfigurationsdatei greifen die Standardeinstellungen (`COM1` mit einer Baudrate von `9600`)


## Kompilierung

Das `zwf.exe` Repository beinhaltet vorkompilierte Versionen der Anwendungen im bin-Ordner. Dort kann zwischen einer Debug und Release-Version entschieden werden.