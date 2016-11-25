/*
  Web Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>

#include "webpage.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

// We set up our steppers
int DIR1 = 2;
int STEP1 = 3;
int STEPS_PER_REV = 400;

// State variables
int doRotation = 0;
int doStep = 0;
int doPos = 0;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  //setup stepper
  pinMode(DIR1, OUTPUT);
  pinMode(STEP1, OUTPUT);

  digitalWrite(DIR1, HIGH);

}

void sendMainPage(EthernetClient client) {
   // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
//  client.println("<!DOCTYPE HTML>");
//  client.println("<html>");
//  // add a meta refresh tag, so the browser pulls again every 5 seconds:
//  client.println("<meta http-equiv=\"refresh\" content=\"5\">");
//  // output the value of each analog input pin
//  for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
//    int sensorReading = analogRead(analogChannel);
//    client.print("analog input ");
//    client.print(analogChannel);
//    client.print(" is ");
//    client.print(sensorReading);
//    client.println("<br />");       
//  }
//  client.println("</html>");

client.write(mainpage_html);
}

void sendBlankPage(EthernetClient client) {
   // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
}

void handleRot(EthernetClient client, String arguments) {
  Serial.println(arguments);
  int rot = arguments.toInt();
  doRotation = rot;
  sendBlankPage(client);
}

void handleStep(EthernetClient client, String arguments) {
  Serial.println(arguments);
  int st = arguments.toInt();
  doStep = st;
  sendBlankPage(client);
}

void handlePos(EthernetClient client, String arguments) {
  Serial.println(arguments);
  int pos = arguments.toInt();
  doPos = pos;
  sendBlankPage(client);
}

enum STATE {
  WAITING,
  EAT,
  DATA,
  DISCARD,
  ERR
};

int len_left = 0;

void handleHTTP() {
  // listen for incoming clients
  EthernetClient client = server.available();
  STATE state = WAITING;
  String command;

  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        switch (state) {
          case DISCARD:
            break;
          case WAITING:
            if (c == 'G') {
              state = EAT;
              len_left = 2;
            } else {
              state = ERR;
            }
            break;
          case EAT:
            if (len_left > 0) {
              len_left--;  
            } else {
              state = DATA;
            }
            break;
          case DATA:
            command = client.readStringUntil(' ');
            state = DISCARD;
            break;
        }
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    if (client.connected()) {
      Serial.print("Received command: ");
      Serial.println(command);

      if (command.equals("")) {
        sendMainPage(client);
      } else if (command.equals("favicon.ico")) {
        sendBlankPage(client);
      } else if (command.startsWith("rot?")) {
        String arguments = command.substring(4);
        handleRot(client, arguments);
      } else if (command.startsWith("pos?")) {
        String arguments = command.substring(4);
        handlePos(client, arguments);
      } else if (command.startsWith("step?")) {
        String arguments = command.substring(5);
        handleStep(client, arguments);
      }

    }
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
  
}

void handleSteppers() {
    int i = STEPS_PER_REV * doRotation + doStep;
    doRotation = 0;
    doStep = 0;

    while (i > 0) {
      digitalWrite(STEP1, HIGH);
      delayMicroseconds(100);
      digitalWrite(STEP1, LOW);
      delayMicroseconds(100);
      i--;
    }

}

void loop() {
  handleHTTP();
  handleSteppers();
}

