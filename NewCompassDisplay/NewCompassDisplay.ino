
#include <SoftwareSerial.h>
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <Bounce2.h>
#define DEBOUNCE 50  // button debouncer, how many ms to debounce, 5+ ms is usually plenty


//pins
SoftwareSerial mySerial(3,2);    // pin 3 = RX  FOR LCD
#define BtnPlusYes A0
#define BtnMinusNo A1
#define BtnOk A2
int BtnPlusYesSTATE = 0;
int BtnMinusNoSTATE = 0;
int BtnOkSTATE = 0;

//pin 10  - ethernet sheild
//pin 11  - ethernet sheild
//pin 12  - ethernet sheild
//pin 13  - ethernet sheild
//pin 14  - ethernet sheild


//set up bounce instances for buttons
Bounce debouncer1 = Bounce();   //BtnPlus
Bounce debouncer2 = Bounce();   //BtnMinusNo
Bounce debouncer3 = Bounce();   //BtnOK

// compass varible
String bearing = 0;          
String pitch  =  0;
String roll  =  0;
String cal = 0;
String SendStatus = 0;
String Oldbearing = 0;
String Oldpitch  =  0;
String Oldroll  =  0;
String Oldcal = 0;
String OldSendStatus = 0;
long  LastRXPacket = 0;                // time last packet was RX'd
long PacketTimeout = 3000;             // Time used before Error due to no packets from compass head unit
String lastScreen = "";                //Used for checking what was last displayed on screen


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0xA0, 0x60};  // arduino ethernet sheild mac (found on sticker)
IPAddress ip(172, 25, 40, 52);                       // local ip address for arduino
unsigned int localPort = 7000;                       // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;






/////////////////////////////////////////////////////////////
void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  mySerial.begin(9600);
  //clear UDP buffer on boot
  ClearPacketBuffer();

  pinMode(BtnPlusYes, INPUT_PULLUP);
  debouncer1.attach(BtnPlusYes);
  debouncer1.interval(DEBOUNCE); // interval in ms

  pinMode(BtnMinusNo, INPUT_PULLUP);
  debouncer2.attach(BtnMinusNo);
  debouncer2.interval(DEBOUNCE); // interval in ms

  pinMode(BtnOk, INPUT_PULLUP);
  debouncer3.attach(BtnOk);
  debouncer3.interval(DEBOUNCE); // interval in ms



  //Start up display message
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
  mySerial.write("                "); // clear display
  mySerial.write("                ");
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
  mySerial.write("Compass Unit");
  delay(1000);
  ClearScreen();
}
//////////////////////////// END /////////////////////////////////




/////////////////////////////////////////////////////////////////
void loop() {
  // if there's data available, read a packet
  String RXData = ReadPacket();    //Check to see if there is packet infomation. Pass back True/False
  GetValFromPacket(RXData);        //Splits RX'd string into each varbile
  ClearPacketBuffer();             //Clear Buffer so new packets dont get curupted

  ButtonCheckUpdate();    // Check state of buttons and save value
  CheckMenuPressed();     // check to see if it should go to menu setup

  if(CheckStatus() == true){       //Check to make sure Packets are not timing out/head unit missing and display appropitioine info on lcd
    PrintValuesToScreen();         //Display Values to display 
    Serial.println(bearing);
    Serial.println(pitch);
    Serial.println(roll);
    Serial.println(cal);
    Serial.println(SendStatus);
  }
  else{
    PrintErrorToScreen();        //Display error message
    Serial.println("NO PACKETS");
  }


  // send a reply to the IP address and port that sent us the packet we received
  //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  //Udp.write(ReplyBuffer);
  //Udp.endPacket();

  delay(20); //allow time for packet to be received 20ms buffer
}
///////////////////////////////// END //////////////////////////////////////





//////////////////// Subs ///////////////////////////////




/////////////Check Still RX packets/////////////////////
boolean CheckStatus(){
  if((millis() - LastRXPacket) < PacketTimeout){      //Check packets been RX'd before a timeout
    return true;
  }
  else{
    return false; 
  }

}






///////////////////split valaues from string from string/////////////////
void  GetValFromPacket(String DataString){
  if(DataString == "NONE"){                        // split string into each varible
  }
  else{
    //Serial.println(packetBuffer);
    String TempString = DataString;
    int firstColon = TempString.indexOf(':');
    //Serial.print("firstColon:");
    //Serial.println(firstColon);
    bearing = TempString.substring(0,firstColon);
    //Serial.print("bearing:");
    //Serial.println(bearing);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of bearing from string
    //Serial.println(TempString);
    firstColon = TempString.indexOf(':');
    //Serial.println(firstColon);
    pitch = TempString.substring(0,firstColon);
    //Serial.print("pitch:");
    //Serial.println(pitch);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of pitch from string
    //Serial.println(TempString);
    firstColon = TempString.indexOf(':');
    //Serial.println(TempString);
    roll = TempString.substring(0,firstColon);
    //Serial.print("roll:");
    //Serial.println(roll);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of roll from string
    //Serial.println("should be cal only");
    //Serial.println(TempString);
    firstColon = TempString.indexOf(':');
    cal = TempString.substring(0,firstColon);
    //Serial.println(firstColon);
    //Serial.print("cal:");
    //Serial.println(cal);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of cal from string
    //Serial.println(TempString);
    firstColon = TempString.indexOf(':');
    //Serial.println(TempString);
    SendStatus = TempString;

  }
}






////////////////////////// get and check packet infomation //////////////////////////////////////
String ReadPacket(){
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    //Serial.print("Received packet of size ");
    //Serial.println(packetSize);
    //Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      //Serial.print(remote[i], DEC);
      if (i < 3) {
        //Serial.print(".");
      }
    }
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    LastRXPacket = millis();
    return packetBuffer;          //If new packet RX'd, return it to update varbiles
  }
  else{
    return "NONE";                //Return None if no packets have been RX'd
  }
}






//////////////////////////////////////////packet buffer clear//////////////////////////////////
void ClearPacketBuffer(){
  for(int i=0;i<UDP_TX_PACKET_MAX_SIZE;i++) packetBuffer[i] = 0;  //clears UDP buffer stop stop random chars

}






///////////////////// Print Error message to screen //////////////////////////////////////////
void PrintErrorToScreen(){
  if(lastScreen != "PrintErrorToScreen"){
    ClearScreen();
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(128);
    mySerial.write("     ERROR!");
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(192);
    mySerial.write("    No Data!    ");
  }

  lastScreen = "PrintErrorToScreen";  //set last screen so it only prints to screen once once

}






///////////////////// Print Compass values to screen ////////////////////////////
void PrintValuesToScreen(){
  //check to see if this is the first time this is being called apon. if so clear screen and display headers
  if(lastScreen != "PrintValuesToScreen"){
    ClearScreen();

    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(128);
    mySerial.write("Bearing = ");

    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(192);
    mySerial.write("T=");

    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(199);
    mySerial.write("R=");
  }
  lastScreen = "PrintValuesToScreen";            //set last screen so it only prints the titles to screen once once


  //Update screen with no compass infomation
  if(Oldbearing != bearing){
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(138);
    mySerial.write("     ");
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(138);
    mySerial.print(bearing);
    mySerial.print((char)223);
  }

  if(Oldpitch != pitch){

    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(194);
    mySerial.write("    ");
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(194);
    mySerial.print(pitch);
    mySerial.print((char)223);
  }
  if(Oldroll != roll){
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(201);
    mySerial.write("     ");
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(201);
    mySerial.print(roll);
    mySerial.print((char)223);
  }
  if(OldSendStatus != SendStatus){
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(207);
    mySerial.print(SendStatus);
  }

  //Update old compas varbiles with latest values
  Oldbearing = bearing;
  Oldpitch = pitch;
  Oldroll = roll;
  Oldcal = cal;
}






//////////////////// clear screen ////////////////////////////////
void ClearScreen(){
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
  mySerial.print("                ");
  mySerial.print("                ");
}





////////////////////// Menu Pressed ///////////////////////////////
void CheckMenuPressed(){
  if(BtnOkSTATE == LOW){
    while(true){
      ButtonCheckUpdate();    // Check state of buttons and save value
      if(BtnOkSTATE == HIGH){
        break;
      }
    }
    int Menuloop = true;
    int MenuSelect = 1;
    ClearScreen();
    while(Menuloop == true){
      //Dispay and select menu;
      switch(MenuSelect){
      case 1:
        if(lastScreen != "bearingAdj"){
          ClearScreen();
          mySerial.write(254); // move cursor to beginning of first line
          mySerial.write(128);
          mySerial.print("Bearing        +");
          mySerial.print("Adjust         -");
          lastScreen = "bearingAdj";
        }
        break;
      case 2:
        if(lastScreen != "TiltAdj"){
          ClearScreen();
          mySerial.write(254); // move cursor to beginning of first line
          mySerial.write(128);
          mySerial.print("Tilt           +");
          mySerial.print("Adjust         -");
          lastScreen = "TiltAdj";
        }
        break;
      case 3:
        if(lastScreen != "rollAdj"){
          ClearScreen();
          mySerial.write(254); // move cursor to beginning of first line
          mySerial.write(128);
          mySerial.print("Roll           +");
          mySerial.print("Adjust         -");
          lastScreen = "rollAdj";
        }  
        break;
      case 4:
        if(lastScreen != "EXIT"){
          ClearScreen();
          mySerial.write(254); // move cursor to beginning of first line
          mySerial.write(128);
          mySerial.print("EXIT           +");
          mySerial.print("               -");
          lastScreen = "EXIT";
        }        
        break;     
      } 



      // check for plus button being pressed once
      ButtonCheckUpdate();    // Check state of buttons and save value
      if(BtnPlusYesSTATE == LOW){
        while(true){
          ButtonCheckUpdate();    // Check state of buttons and save value
          if(BtnPlusYesSTATE == HIGH){
            break;
          }
        }

        MenuSelect = ++MenuSelect;
        if(MenuSelect > 4){
          MenuSelect = 1;
        }
      }


      // check for minus button being pressed once
      if(BtnMinusNoSTATE == LOW){
        while(true){
          ButtonCheckUpdate();    // Check state of buttons and save value
          if(BtnMinusNoSTATE == HIGH){
            break;
          }
        }

        MenuSelect = --MenuSelect;
        if(MenuSelect < 1){
          MenuSelect = 4;
        }
      }  
      Serial.print("am i gettings here");
      //Action Selected menu
      if(BtnOkSTATE == LOW){
        Serial.print("got here");
        while(true){
          ButtonCheckUpdate();    // Check state of buttons and save value
          if(BtnOkSTATE == HIGH){
            break;
          }
        }
        switch(MenuSelect){
        case 1:


          break; 

        case 2:

          break;

        case 3:

          break;

        case 4:
          Menuloop = false;
          break;   
        }
      }


    }  // end while loop        
  }//end if loop
  ClearPacketBuffer();
}







//////////////// calibrate bearing and send update to head //////////////////////////
void BearingCal(){
  while(true){

       ButtonCheckUpdate();
       if(lastScreen != "BearingCal"){
          ClearScreen();
          mySerial.write(254); // move cursor to beginning of first line
          mySerial.write(128);
          mySerial.print("Bearing =      +");
          mySerial.print("Cal Val =      -");
          lastScreen = "BearingCal";
        }// end of if loop
       if(Oldbearing != bearing){
          mySerial.write(254); // move cursor to beginning of first line
          mySerial.write(137);
          mySerial.write("     ");
          mySerial.write(254); // move cursor to beginning of first line
          mySerial.write(137);
          mySerial.print(bearing);
          mySerial.print((char)223);
        }
        
  Oldbearing = bearing;      
  String RXData = ReadPacket();    //Check to see if there is packet infomation. Pass back True/False
  GetValFromPacket(RXData);        //Splits RX'd string into each varbile    
}//end of while loop
}








////////////////////// Check and update button state ////////////////////////
void ButtonCheckUpdate(){
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();
  BtnPlusYesSTATE = debouncer1.read();
  BtnMinusNoSTATE = debouncer2.read();
  BtnOkSTATE = debouncer3.read();

}












