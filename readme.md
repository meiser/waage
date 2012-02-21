# __Programm zur Ansteuerung des Fischer FMP100 in einer Baan-Session (Infor ERP)__

## Idee

Das Programm `fmp100.exe` verbindet sich ueber USB mit dem Schichtdickenmessgeraet Fischer FMP100 und wandelt dessen Daten in eine XML-Datei um, die im Anschluss von einer Baan-Session eingelesen und verarbeitet werden kann. Durch Verwendung der Boost-Bibliothek (u.a. Asio, Regex, Thread, Filesystem) ist der Programmcode betriebssystemunabhaengig.

## Nutzung des Programms

Ein Beispielaufruf des Programms kann wie folgt aussehen:

	fmp100.exe --start_time 123

Dieser Aufruf erzeugt im Hauptordner des Programms die Dateien `data/123/123` und `data/123/messwerte.xml`

* Die Datei `123` beinhaltet die gesendeten Daten des Messgeraetes (unveraendert)
* Die XML-Datei `messwerte.xml` beinhaltet eine aufbereitete XML-Version der COM-Port-Daten

## Verwendete Programme, Bibliotheken usw.
* [Boost C++ Libraries](http://www.boost.org/ "Boost C++ Libraries")
* [TinyXML](http://www.grinninglizard.com/tinyxml "TinyXML")
* [serial-port](http://gitorious.org/serial-port "serial-port")

## Anforderungen

* Fischer FMP100 USB Treiber auf Mini CD (ist dem Messgeraet beigelegt)
* Windows XP, Windows Vista oder Windows 7
* Bearbeitung der Projektdatei mit Codeblocks (IDE) und Kompilierung mit der C++ Boost Bibliothek

## Kommandozeilenargumente

Das Programm `fmp100.exe` kann mit folgenden Startparametern ausgefuehrt werden:

* `--help, --h` Listet alle zulaessigen Kommandozeilenargumente und ihre Funktionsbeschreibung auf
* `--start_time, --s` Angabe der Startzeit des Programms als Timestamp. Dieser wird von der Baan-Session erzeugt und an das aufrufende Programm uebergeben. Dieser Parameter ist erforderlich.
* `--console, --c` Startet des Programm im interaktiven Konsolenmodus
* `--input, --i` Angabe des COM-Ports z.B. --i COM1 oder --i /dev/ttyS0, Standard ist COM1
* `--baudrate, --b` Angabe der Uebertragungsgeschwindigkeit des COM-Ports, Standard ist 9600
* `--file, --f` Angabe des Pfades zu einer Konfigurationsdatei

## Interaktiver Konsolenmodus

Durch den Start des Programms im interaktiven Konsolenmodus kann ueber spezifische COM-Port-Befehle eine direkte Kommunikation mit dem Fischer FMP100 hergestellt werden.
Die Rueckantwort des Geraetes wird direkt in der Konsole ausgegeben.

Momentan stehen folgende Befehle zur Verfuegung:

* `VV` Gibt den Namen des Geraetes und die verwendete Firmwareversion aus
* `NAMHEX` Angabe des Geraetenamens in Hexadezimalschreibweise
* `PE` Ausgabe des fuer die Messapplikation konfigurierten Gruppenseparators. Folgende Gruppenseparatoren koennen ueber die COM-Port-Einstellungen des Geraetes eingestellt werden: `GS` (Hex code 0x1d), `*`, `;`, `#`, `:` und `,`
* `SAM` Gibt alle Daten der aktuellen Messapplikation entsprechend der COM-Port-Einstellungen und der Blockergebnisvorlage aus
* `DAT0-DATxxx` Gibt Datum und Uhrzeit der Erstellung eines Messblocks xxx aus. Der erste Block beginnt entsprechend mit DAT0
* Unbekannte bzw. falsche Steuerbefehle liefern als Antwort ein Fragezeichen zurueck (`?`)
* Die Bedeutung folgender Steuerbefehle ist noch nicht bekannt: `SL`, `LSL`, `USL`
* `exit` Beendigung des Programms

##Konfigurationsoptionen

Zur Konfiguration der COM-Port Verbindungsdaten bietet `fmp100.exe` verschiedene Moeglichkeiten:


### Erstellung einer Autokonfigurationsdatei

Die Konfigurationsdatei muss den Namen `config` haben und sich im gleichen Ordner wie die Datei `fmp100.exe` befinden. Der Aufbau der Datei sieht dabei folgende Struktur vor:
	
	port:COM3
	baudrate:115200

Die Reihenfolge von Port- und Baudratenangabe kann beliebig vertauscht werden.


### Verwendung des Kommandozeilenarguments `--f`

Mit Hilfe des Kommandozeilenarguments `--f` kann ein individueller Pfad zu einer Konfigurationsdatei angegeben werden (siehe Abschnitt Kommandozeilenargumente).
Die Struktur der Konfigurationsdatei enspricht dabei der Struktur der Autokonfigurationsdatei.

### Verwendung der Kommandozeilenargumente `--i` und `--b`

Mit Hilfe der Kommandozeilenargumente `--i` und `--b` koennen die Verbindungsparameter des Programms direkt beim Aufruf angegeben werden (siehe Abschnitt Kommandozeilenargumente).



### ANMERKUNGEN

Wenn die Kommandozeilenargumente `--i` oder `--b` nicht verwendet werden bezieht das Programm die Verbindungsdaten aus der Konfigurationsdatei. Dabei hat die Konfigurationsdatei, die mit dem Kommandozeilenargument `--f` angegeben wurde Vorrang vor der Autokonfigurationsdatei im Hauptornder des Programms.
Existiert keine Autokonfigurationsdatei greifen die Standardeinstellungen (`COM1` mit einer Baudrate von `9600`)

## XML-Struktur

Das Programm `fmp100.exe` erzeugt aus der aktuell gewaehlten Messapplikation eine XML-Datei mit den Elementen "application", "block" und "value".

1. Das Element "application" ist das Wurzelelement der XML-Datei und beinhaltet alle relevanten Daten zur Messapplikation (Name der Messanwendung, obere und untere Toleranzgrenze sowie die verwendete Messeinheit)
2. Das Element "block" beinhaltet alle Daten fuer die Beschreibung eines Messblockes (u.a. Auftragsnummer, Blockkommentar, Anzahl der Messwerte und Zeitpunkt der Erstellung zerlegt in Tag, Monat, Jahr, Stunden und Minuten)
3. Das Element "value" beinhaltet den Zahlenwert des Messwertes

Anmerkung: Auftragsnummer und Kommentar sind nicht zwingend erforderlich.

Die folgende Datei zeigt eine Beispielanwendung  "Farbschichtmessung" mit 3 Messbloecken:

	<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
	<application name="Farbschichtmessung" max="2000" min="455" unit="um">
	<block ordernumber="121987654" day="16" month="2" year="2012" hour="16" minute="0" comment="Kommentar 1" amount="16">
	<value>3.59143</value>
        <value>0.978486</value>
        <value>2.68681</value>
        <value>0.0290042</value>
        <value>3.08025</value>
        <value>1.30557</value>
        <value>2.41982</value>
        <value>1.13978</value>
        <value>0.873436</value>
        <value>1.43767</value>
        <value>1.45676</value>
        <value>1.47592</value>
        <value>0.856119</value>
        <value>3.24752</value>
        <value>2.70945</value>
        <value>2.77777</value>
    </block>
    <block ordernumber="" day="17" month="2" year="2012" hour="8" minute="46" comment="Kommentar 2" amount="14">
        <value>2.3154</value>
        <value>2.31013</value>
        <value>1.13798</value>
        <value>1.49795</value>
        <value>1.38427</value>
        <value>1.83822</value>
        <value>0.685929</value>
        <value>0.908234</value>
        <value>0.804493</value>
        <value>0.925715</value>
        <value>1.61164</value>
        <value>2.07399</value>
        <value>2.2901</value>
        <value>2.34477</value>
    </block>
    <block ordernumber="" day="20" month="2" year="2012" hour="7" minute="5" comment="" amount="4">
        <value>3.4179</value>
        <value>1.67061</value>
        <value>1.32427</value>
        <value>3.20199</value>
    </block>
</application>


## Kompilierung

Das `fmp100.exe` Repository beinhaltet vorkompilierte Versionen der Anwendungen im bin-Ordner. Dort kann zwischen einer Debug und Release-Version entschieden werden.
Die verwendete Baan-Session greift dabei auf die Datei `fmp100.exe` der Release-Version zurueck (`fmp100/bin/Release/fmp100.exe`)