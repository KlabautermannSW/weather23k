# weather23k

## Entstehung, Ziel
Lange habe ich meine [Wetterseite](http://ur9.de/) mit einem selbst geschriebenen Programm auf Basis
der [OPEN2300-Bibliothek](http://lavrsen.dk/foswiki/bin/view/Open2300/WebHome) von Kenneth Lavrsen
gefüttert.

Allerdings war mir die Übersetzungszeit auf einem [Raspberry Pi](http://www.raspberrypi.org/) einfach zu lang, weil
in den Quelltexten sehr viele Sachen enthalten sind, die ich nicht brauche. Zudem gibt
es die Treiber sowohl für Linux als auch für Windows, letztere brauche ich aber nicht
(mehr).

Also entstand die Idee, diese Bibliothek als Grundlage für eine eigene Entwicklung zu
nehmen. Hierbei habe ich Wert auf klare Struktur und möglichst schnellen Code gelegt. Daraus
sind einige Module entstanden, die folgende Kriterien erfüllen :

- Code nur für Linux
- Strukturierung des Projekts in einzelnen Verzeichnissen
- Zusammengehörende Funktionen und Daten befinden sich im gleichen Modul (Source- und Include-Datei)
- Modulinterne Funktionen und Daten sind statisch deklariert
- Die API der einzelnen Module enthält nur die extern notwendigen Funktions- und Typdeklarationen
- Innerhalb eines Moduls gibt es keine Funktion, die hier nur einmal aufgerufen wird

Aus diesen Überlegungen ist das Programm "weather23k" entstanden. Es ist keine allgemein
verwendbare Bibliothek, sondern ein komplettes Programm, um 
- die Daten aus einer WS2300-kompatiblen Wetterstation auszulesen
- diese Daten zu loggen (lokal und auf einem FTP-Server)
- ausgewählte Daten für eine Webseite zur Verfügung zu stellen
- mit einfacher Debug-Möglichkeit

Noch ist das Programm in Entwicklung, erfüllt aber bereits die genannten Anforderungen.

## Hinweise zur Sicherheit
Wenn das Programm auf einem Computer betrieben wird, der von ausserhalb des lokalen Netzwerks
erreichbar ist, sollte folgendes beachtet werden :
- der Quellcode sollte nicht auf dem selben Computer gespeichert sein
- die folgenden Funktionen sollten mit einem eigenen Ver-/Entschlüsselungs-Algorythmus
implementiert sein
    - encode( void )
    - decode( char * p_in, char * p_password )

## Idea and Goal
I feeded my [weather webpage](http://ur9.de/) with a self developed program based on the
[OPEN2300 library](http://lavrsen.dk/foswiki/bin/view/Open2300/WebHome) implemented by
Kenneth Lavrsen for a long time.

When workinh on the program I felt the compiling time on a [Raspberry Pi](http://www.raspberrypi.org/)
was to long because in the source there are lots of gadgets that I never needed. Additionally the drivers included
in the library are for Linux as for Winows. The last I do not need any more.

So I decided to use the [OPEN2300 library](http://lavrsen.dk/foswiki/bin/view/Open2300/WebHome)
as a base for my own development. I emphasized well structured and fast as possible code for
my project. That resulted in some modules that fulfill the following criteria :
- code is implemented for Linux only
- the project has a precisely defined structure using separate directories
- functions that belong together are collected in the same module (source and include file)
- functions and data used exclusive in one module are declared as static there
- the API contains external needed functions and type declarations only
- there is no function inside a module that is called only once

From this thoughts the program "weather23k" came into existence. It is not a
generally usable library but a complete application to
- read data for a WS2300 compatible weather station
- log these data (locally and on a ftp server)
- provide selected data to a web page
- support simple debug features

The program is in development state yet but it fulfills most of the requirements just now.

## Security hints
If you run the program "weather23k" on a computer that is accessible from
outside your local network you should beware of the following :
- do NOT store the source code on the same computer
- implement the functions
    - encode( void )
    - decode( char * p_in, char * p_password )

with your own encoder/decoder algorithm so there will be no plain text
password in the configuration file

## More to know
To make and use this application you must have installed libcurl and
libcurl-devel on your system.

For a files list see doc/filestree.txt
