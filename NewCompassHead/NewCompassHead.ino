#include <Wire.h>
#include <SPI.h>        
#include <Ethernet.h>
#include <EthernetUdp.h>

#define CMPS11_ADDRESS 0x60 //compass modual address
#define ANGLE_8  1           // Register to read 8bit angle from



// Enter a MAC address and IP address for your controller below.

// The IP address will be dependent on your local network:

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(172, 25, 40, 50);
IPAddress remote_ip(172, 25, 40, 52);  // Address of target machine

unsigned int localPort = 7000;      // local port to listen on
unsigned int remote_port = 7000;      // Port to send to

//Compass varibles
unsigned char high_byte, low_byte, angle8;
char pitch, roll;
unsigned int angle16;
char bearingCal = 0;
char pitchCal = 0;
char rollCal = 0;


int SendStatus = 1;

unsigned int Heading;
int Bearing;
unsigned int Calibration = 1;


// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP)
EthernetServer server(80);



void setup() {
   Wire.begin();

  Serial.begin(9600);
  
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);
  server.begin();

  }













void loop() {
  CompassRead();
  //Serial.println(remote_ip);
  
  Udp.beginPacket(remote_ip, remote_port);
  Udp.print(Bearing);
  Udp.print(":");
  Udp.print(pitch, DEC);
  Udp.print(":");
  Udp.print(roll,DEC);
  Udp.print(":");
  Udp.print(bearingCal,DEC);
  Udp.print(":");
  Udp.print(pitchCal,DEC);
  Udp.print(":");
  Udp.print(rollCal,DEC);
  Udp.print(":");


  switch (SendStatus){
    case 1:
    SendStatus = ++SendStatus;
    Udp.write("*");
      break;
    case 2:
    SendStatus = ++SendStatus;
    Udp.write("#");
      break;
    case 3:
    SendStatus = ++SendStatus;
    Udp.write("*");
      break;
    case 4:
    SendStatus = 1;
    Udp.write("#");
      break;  
}



  Serial.println("status:");
Serial.println(SendStatus);

  Udp.endPacket();
  delay(1000);       /// this is blocking, use mili loop


/*
//------------------------------------------------------------
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
            // output the value of each analog input pin
        
            client.print("Compass: ");
            client.print(Bearing);
            client.println("<br />");
            client.print("pitch: ");
            client.print(pitch, DEC);
            client.println("<br />");
            client.print("Roll: ");
            client.print(roll, DEC);
            client.println("<br />");
            client.print("Cal: ");
            client.print(bearingCal, DEC);
            client.println("<br />");   
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
  


*/
}

  

//////////////////////////////
unsigned int CompassRead(){
  Wire.beginTransmission(CMPS11_ADDRESS);  //starts communication with CMPS11
  Wire.write(ANGLE_8);                     //Sends the register we wish to start reading from
  Wire.endTransmission();
 
  // Request 5 bytes from the CMPS11
  // this will give us the 8 bit bearing, 
  // both bytes of the 16 bit bearing, pitch and roll
  Wire.requestFrom(CMPS11_ADDRESS, 5);       
  
  while(Wire.available() < 5);        // Wait for all bytes to come back
  
  angle8 = Wire.read();               // Read back the 5 bytes
  high_byte = Wire.read();
  low_byte = Wire.read();
  pitch = Wire.read();
  roll = Wire.read();
  
  angle16 = high_byte;                 // Calculate 16 bit angle
  angle16 <<= 8;
  angle16 += low_byte;
    
  Serial.print("roll: ");               // Display roll data
  Serial.print(roll, DEC);
  
  Serial.print("    pitch: ");          // Display pitch data
  Serial.print(pitch, DEC);
  
  Serial.print("    angle full: ");     // Display 16 bit angle with decimal place
  Serial.print(angle16 / 10, DEC);
  Bearing = (angle16 / 10);

  delay(100);    
   
}

