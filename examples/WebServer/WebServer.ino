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


// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);


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

    // start the server
    server.begin();

}

void loop()
{
// listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
        Serial.println("new client");
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");  // the connection will be closed after completion of the response
                    client.println("Refresh: 5");  // refresh the page automatically every 5 sec
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");
                    client.println("<h5>Hello World<h5>");
                    client.println("<h4>Hello World<h4>");
                    client.println("<h3>Hello World<h3>");
                    client.println("<h2>Hello World<h2>");
                    client.println("<h1>Hello World<h1>");
                    client.println("</html>");
                    break;
                }
                if (c == '\n') {
                    // you're starting a new line
                    currentLineIsBlank = true;
                } else if (c != '\r') {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disconnected");
    }
}