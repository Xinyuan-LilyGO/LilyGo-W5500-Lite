#include    <Arduino.h>
#include    <SPI.h>
#include    <Ethernet.h>
#include    <SPI.h>
#include    <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include    "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#define     ETH_RST         4
#define     ETH_CS          5
#define     ETH_SCLK       18
#define     ETH_MISO       23
#define     ETH_MOSI       19
#define     OLED_SCL       22
#define     OLED_SDA       21


uint8_t mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, OLED_SDA, OLED_SCL);


// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "www.bing.com";    // name address for Google (using DNS)

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement


void ethernetReset(const uint8_t resetPin)
{
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, HIGH);
    delay(250);
    digitalWrite(resetPin, LOW);
    delay(50);
    digitalWrite(resetPin, HIGH);
    delay(350);
}


void setup()
{
    Serial.begin(115200);
    Serial.println("ESP32 W5500 Start");

    display.init();
    display.flipScreenVertically();

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    display.drawString(0, 0, "ESP32 W5500 Start");
    // display.display();

    SPI.begin(ETH_SCLK, ETH_MISO, ETH_MOSI);

    ethernetReset(ETH_RST);
    Ethernet.init(ETH_CS);

    Serial.println("Starting ETH connection...");
    display.drawString(0, 12, "Starting ETH connection...");
    display.display();


    if (Ethernet.begin(mac) == 0) {

        display.drawString(0, 24, "Failed to configure Eth using DHCP");
        Serial.println("Failed to configure Eth using DHCP");
        display.display();

        if (Ethernet.hardwareStatus() == EthernetNoHardware) {

            display.drawString(0, 34, "Eth shield was not found.");
            Serial.println("Eth shield was not found.");
            display.display();

        } else if (Ethernet.linkStatus() == LinkOFF) {

            display.drawString(0, 34, "Eth cable is not connected.");
            Serial.println("Eth cable is not connected.");
            display.display();

        }
        // no point in carrying on, so do nothing forevermore:
        while (true) {
            delay(1);
        }
    }


    Serial.print("Ethernet IP is: ");
    Serial.println(Ethernet.localIP());

    display.drawString(0, 24, "Ethernet IP is: ");
    display.drawString(0, 34, Ethernet.localIP().toString());
    display.display();


    // give the Ethernet shield a second to initialize:
    delay(1000);
    Serial.print("connecting to ");
    Serial.print(server);
    Serial.println("...");

    // if you get a connection, report back via serial:
    if (client.connect(server, 80)) {
        Serial.print("connected to ");
        Serial.println(client.remoteIP());
        // Make a HTTP request:
        client.println("GET / HTTP/1.1");
        client.println("Host: www.bing.com");
        client.println("Connection: close");
        client.println();
    } else {
        // if you didn't get a connection to the server:
        Serial.println("connection failed");
    }
    beginMicros = micros();

}

void loop()
{
// if there are incoming bytes available
    // from the server, read them and print them:
    int len = client.available();
    if (len > 0) {
        byte buffer[80];
        if (len > 80) len = 80;
        client.read(buffer, len);
        if (printWebData) {
            Serial.write(buffer, len); // show in the serial monitor (slows some boards)
        }
        byteCount = byteCount + len;
    }

    // if the server's disconnected, stop the client:
    if (!client.connected()) {
        endMicros = micros();
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
        Serial.print("Received ");
        Serial.print(byteCount);
        Serial.print(" bytes in ");
        float seconds = (float)(endMicros - beginMicros) / 1000000.0;
        Serial.print(seconds, 4);
        float rate = (float)byteCount / seconds / 1000.0;
        Serial.print(", rate = ");
        Serial.print(rate);
        Serial.print(" kbytes/second");
        Serial.println();

        // do nothing forevermore:
        while (true) {
            delay(1);
        }
    }
}