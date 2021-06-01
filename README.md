# EmbeddedC
Embedded Project
Required:
-esp8266
-HTU2x sensor
-HW-740 sensor
Proiectul are la bază o placă de dezvoltare,ESP8266,
(microcontroller) un senzor de temperatura si umiditate HTU21D, si un
senzor de miscare. Comunicarea dintre ESP8266 si senzorul de mișcare
se face printr-un simplu GPIO (General-purpose input/output) in cazul de
fata pinul D7 de pe esp8266 (GPIO13), alimentare la un pin 3.3v si
GND. Senzorul de temperatura si umiditate se conecteaza prin
intermediul interfeței de comunicare I2C. In cazul de fata ESP8266
dispune de o singura conexiune de tip I2C. Senzorul de temperatura se
conecteaza prin intermediul pinului D1 (GPIO05, Serial Clock Line) si D2
(GPIO04, Serial Data Line), alimentat cu sursa de 3.3v si GND.
