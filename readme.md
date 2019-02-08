# Mein printzerow...
Was macht man mit einem Raspberry Zero W...? Man baut sich daraus einen
Printserver (plus ein paar Zusatzfunktionen, da das Ding ja sowieso ständig
an ist)!


## Printserver
Szenario: man besitzt einen Drucker, der noch nicht netzwerkfähig ist und
sieht nicht ein, dass man sich deshalb ein neuen Drucker kaufen soll. Mit 
einem kleinen Raspberry Zero W kann man dies ja nachrüsten!

Als Druck-Server wird CUPS verwendet. Die Installation gestaltet sich 
recht entspannt:
```
> sudo apt-get install cups cups-bsd printer-driver-foo2zjs-common printer-driver-foo2zjs
```

Mit dem Tool cupsctl sollte man folgendes einstellen:
* die lokalen, am Server angeschlossenen, Drucker im Netzwerk freigeben
* CUPS über das Netzwerk administrierbar machen
* die automatische Druckervermittlung aktivieren
```
> sudo cupsctl --share-printers  --remote-admin --remote-printers
```

Ein User auf dem Server sollte CUPS administrieren dürfen (z.B. Drucker 
hinzufügen):
```
> sudo usermod -aG lpadmin <user>
```

Danach kann CUPS z.B. über einen Webbrowser administriert werden. 
URL: https://dein_printzerow:631. Wie man Drucker einrichtet und 
administriert, kann der CUPS-Dokumentation entnommen werden bzw. die 
Web-Oberfläche ist auch recht selbsterklärend...

Ich besitze einen HP Laserjet 1018, es muss die Firmware aus dem Internet 
geholt und installiert werden, damit diese automatisch beim Einschalten 
des Druckers zu selbigen kopiert wird:
```
> sudo getweb 1018
...
```
Das Tool getweb ist Bestandteil des Paketes printer-driver-foo2zjs. Als 
Parameter wird die Typ-Nummer des Druckers angegeben. Das Tool richtet 
dann alles notwendige automatisch ein.

Danach empfiehlt sich ein Reboot des Systems bzw. ein Restart von CUPS.


## Zusatzfunktionalitäten
Als reiner Printserver langweilt sich mein "printzerow", deshalb macht er 
u.a. noch ein paar Sachen nebenbei.

### GUI
Im Unterverzeichnis printzerow_gui (dieses GIT-Repositories) ist eine 
kleine Bedienoberfläche zu finden, welche diverse (ich gebe zu, sehr 
individuelle) Funktionalitäten auf einem OLED-Display mit ein paar Tasten 
zur Verfügung stellt. Das Ganze soll nur eine Anregung für eigene 
Implementierungen sein! 
--> siehe Quelltext...:-)...!

### Temperatur, Luftfeuchtigkeit und Luftdruck messen
Da das Ding (der Printserver) sowie ständig an ist, kann er auch zyklisch 
die Werte eines angeschlossenen BME280-Sensors ermitteln und weiterleiten.
In meinem Fall erfolgt die Weiterleitung an einen MQTT-Broker.
--> siehe Quelltext im Unterverzeichnis bme280...:-)...!


---------
Have fun!



Uwe Berger; 2019
