
#include <SoftwareSerial.h>
#include <SPI.h>           // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>   // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <Bounce2.h>
#define DEBOUNCE 50        // button debouncer, how many ms to debounce, 5+ ms is usually plenty


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
String bearing = "0";          
String tilt  =  "0";
String roll  =  "0";
String bearingCalVal = "0";
String tiltCalVal  =  "0";
String rollCalVal  =  "0";
String SendStatus = "0";
String Oldbearing = "0";
String Oldtilt  =  "0";
String Oldroll  =  "0";
String OldBearingCal = "0";
String OldSendStatus = "0";
long  LastRXPacket = 0;                // time last packet was RX'd
long PacketTimeout = 3000;             // Time used before Error due to no packets from compass head unit
String lastScreen = "";                //Used for checking what was last displayed on screen

String RXData;    // Saves RX data to be passed onto string spliting



// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0xA0, 0x60};         // arduino ethernet sheild mac (found on sticker)
IPAddress ip(172, 25, 40, 52);                 // local ip address for arduino


IPAddress remote_ip(172, 25, 40, 50);          // Address of target machine
unsigned int localPort = 7000;                       // local port to listen on
unsigned int remote_port = 7000;               // Port to send to

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];     //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";           // a string to send back

EthernetUDP Udp;                               // An EthernetUDP instance to let us send and receive packets over UDP


///////////////////////// END /////////////////////////////////////////






/////////////////////////////SETUP/////////////////////////////////////
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

  delay(1000);      //allow screen to settle down


  //Start up display message
  ClearScreen();
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
  mySerial.write("Compass Unit");
  delay(1000);
  ClearScreen();


}
//////////////////////////// END ///////////////////////////////////////




///////////////////////////////////////////////////////////////////////
void loop() {
  // if there's data available, read a packet
  RXData = ReadPacket();    //Check to see if there is packet infomation. Pass back True/False
  GetValFromPacket(RXData);        //Splits RX'd string into each varbile
  ClearPacketBuffer();             //Clear Buffer so new packets dont get curupted

  ButtonCheckUpdate();    // Check state of buttons and save value
  CheckMenuPressed();     // check to see if it should go to menu setup

  if(CheckStatus() == true){       //Check to make sure Packets are not timing out/head unit missing and display appropitioine info on lcd
    PrintValuesToScreen();         //Display Values to display 
    //Serial.println(bearing);
    //Serial.println(tilt);
    //Serial.println(roll);
    //Serial.println(bearingCal);
    //Serial.println(SendStatus);
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
///////////////////////////////// END //////////////////////////////////





//////////////////// SUBS /////////////////////////////////////////////
/////////////Check Still RX packets////////////////////////////////////
boolean CheckStatus(){
  if((millis() - LastRXPacket) < PacketTimeout){      //Check packets been RX'd before a timeout
    return true;
  }
  else{
    return false; 
  }

}
///////////////////////// END /////////////////////////////////////////






///////////////////split valaues from string from string////////////////
void  GetValFromPacket(String DataString){
  if(DataString == "NONE"){                        // split string into each varible
  }
  else{
    //Serial.println(packetBuffer);
    String TempString = DataString;
    int firstColon = TempString.indexOf(':');
    bearing = TempString.substring(0,firstColon);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of bearing from string
    firstColon = TempString.indexOf(':');
    tilt = TempString.substring(0,firstColon);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of tilt from string
    firstColon = TempString.indexOf(':');
    roll = TempString.substring(0,firstColon);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of roll from string
    firstColon = TempString.indexOf(':');
    bearingCalVal = TempString.substring(0,firstColon);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of roll from string
    firstColon = TempString.indexOf(':');
    tiltCalVal = TempString.substring(0,firstColon);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of tiltCal from string
    firstColon = TempString.indexOf(':');
    rollCalVal = TempString.substring(0,firstColon);
    TempString = TempString.substring(firstColon+1, TempString.length());   //cut of tiltCal from string
    firstColon = TempString.indexOf(':');
    SendStatus = TempString;

  }
}
///////////////////////// END /////////////////////////////////////////






////////////////////////// get and check packet infomation ////////////
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
///////////////////////// END /////////////////////////////////////////






//////////////////////////////////////////packet buffer clear//////////
void ClearPacketBuffer(){
  for(int i=0;i<UDP_TX_PACKET_MAX_SIZE;i++) packetBuffer[i] = 0;  //clears UDP buffer stop stop random chars

}
///////////////////////// END /////////////////////////////////////////






///////////////////// Print Error message to screen ///////////////////
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
///////////////////////// END /////////////////////////////////////////






///////////////////// Print Compass values to screen ///////////////////
void PrintValuesToScreen(){
  if(lastScreen != "PrintValuesToScreen"){          //check to see if this is the first time this is being called apon. if so clear screen and display headers
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
    //Print bearing to make sure screen show right
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(138);
    mySerial.print(bearing);
    mySerial.print((char)223);
    //print tilt once to make sure screen shows right
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(194);
    mySerial.print(tilt);
    mySerial.print((char)223);
    //print roll once to make sure screens shows right
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(201);
    mySerial.print(roll);
    mySerial.print((char)223);
    lastScreen = "PrintValuesToScreen";            //set last screen so it only prints the titles to screen once once
  }

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

  if(Oldtilt != tilt){
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(194);
    mySerial.write("    ");
    mySerial.write(254); // move cursor to beginning of first line
    mySerial.write(194);
    mySerial.print(tilt);
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

  //Update old compass varbiles with latest values so values only print to screen if they change
  Oldbearing = bearing;
  Oldtilt = tilt;
  Oldroll = roll;
  OldBearingCal = bearingCalVal;
}
///////////////////////// END /////////////////////////////////////////








//////////////////// clear screen ////////////////////////////////////
void ClearScreen(){
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
  mySerial.print("                ");
  mySerial.print("                ");
  delay(10);
}
///////////////////////// END /////////////////////////////////////////






////////////////////// Menu Pressed ///////////////////////////////////
void CheckMenuPressed(){
  if(BtnOkSTATE == LOW){      //check for button to be pressed and released before changing value
    while(true){
      ButtonCheckUpdate();    // Check state of buttons and save value
      if(BtnOkSTATE == HIGH){
        break;
      }
    }
    int Menuloop = true;      // Menu will exits when false
    int MenuSelect = 1;       // Menu currently selected
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


      // check for +/- button being pressed once
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
      //Action Selected menu
      if(BtnOkSTATE == LOW){
        while(true){
          ButtonCheckUpdate();    // Check state of buttons and save value
          if(BtnOkSTATE == HIGH){
            break;
          }
        }
        switch(MenuSelect){
        case 1:
          BearingCal();
          break; 
        case 2:
          tiltCal();
          break;
        case 3:
          rollCal();
          break;
        case 4:
          Menuloop = false;
          ClearScreen();
          ClearPacketBuffer();
          break;   
        } // end of switch
      }
    }       
  }
}
///////////////////////// END /////////////////////////////////////////







//////////////// calibrate bearing and send update to head /////////////
void BearingCal(){
  String oldbearingCalVal = "0";
  String oldBearingVal = "0";
  while(true){
    ButtonCheckUpdate();
    RXData = ReadPacket();    //Check to see if there is packet infomation. Pass back True/False
    GetValFromPacket(RXData);        //Splits RX'd string into each varbile
    if(lastScreen != "BearingCal"){
      ClearScreen();
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(128);
      mySerial.print("Bearing =      +");
      mySerial.print("Cal =          -");
      lastScreen = "BearingCal";
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(138);
      mySerial.print(bearing);    
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.print(bearingCalVal);  
    }// end of if loop

    //Update screen with new RX cal val
    if(bearingCalVal != oldbearingCalVal){
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.write("    ");
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.print(bearingCalVal);
      oldbearingCalVal = bearingCalVal;
    }
    if(bearing != oldBearingVal){
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(138);
      mySerial.write("    ");
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(138);
      mySerial.print(bearing);
      oldBearingVal = bearing;
    }


    //Button +/- press detection
    if(BtnPlusYesSTATE == LOW){
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnPlusYesSTATE == HIGH){
          SendCompassVals( "b", "+");
          break;
        }
      }
    }

    if(BtnMinusNoSTATE == LOW){
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnMinusNoSTATE == HIGH){
          SendCompassVals( "b", "-");
          break;
        }
      }
    }

    // exit back to menu if ok pressed
    if(BtnOkSTATE == LOW){                // exit cal adjust
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnOkSTATE == HIGH){
          break;
        }// end of if
      }//end if while;
      break;  
    }
  }
}
///////////////////////// END /////////////////////////////////////////








//////////////// calibrate tilt and send update to head ////////////////
void tiltCal(){
  String oldTiltCalVal = "0";
  String oldTiltVal = "0";
  while(true){
    ButtonCheckUpdate();
    RXData = ReadPacket();    //Check to see if there is packet infomation. Pass back True/False
    GetValFromPacket(RXData);        //Splits RX'd string into each varbile
    if(lastScreen != "tiltCal"){
      ClearScreen();
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(128);
      mySerial.print("Tilt =         +");
      mySerial.print("Cal =          -");
      lastScreen = "tiltCal";
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(135);
      mySerial.print(tilt);    
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.print(tiltCalVal);  
    }// end of if loop

    //Update screen with new RX cal val
    if(tiltCalVal != oldTiltCalVal){
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.write("    ");
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.print(tiltCalVal);
      oldTiltCalVal = tiltCalVal;
    }
    if(tilt != oldTiltVal){
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(135);
      mySerial.write("    ");
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(135);
      mySerial.print(tilt);
      oldTiltVal = tilt;
    }


    //Button +/- press detection
    if(BtnPlusYesSTATE == LOW){
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnPlusYesSTATE == HIGH){
          SendCompassVals( "t", "+");
          break;
        }
      }
    }
    if(BtnMinusNoSTATE == LOW){
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnMinusNoSTATE == HIGH){
          SendCompassVals( "t", "-");
          break;
        }
      }
    }

    //Exit to menu if OK pressed
    if(BtnOkSTATE == LOW){                // exit cal adjust
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnOkSTATE == HIGH){
          break;
        }// end of if
      }//end if while;
      break;  
    }
  }
}
///////////////////////// END /////////////////////////////////////////








//////////////// calibrate Roll and send update to head ////////////////
void rollCal(){
  String oldrollCalVal = "0";
  String oldrollVal = "0";
  while(true){
    ButtonCheckUpdate();
    RXData = ReadPacket();    //Check to see if there is packet infomation. Pass back True/False
    GetValFromPacket(RXData);        //Splits RX'd string into each varbile
    if(lastScreen != "rollCal"){
      ClearScreen();
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(128);
      mySerial.print("Roll =         +");
      mySerial.print("Cal =          -");
      lastScreen = "rollCal";
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(135);
      mySerial.print(roll);    
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.print(rollCalVal);  
    }// end of if loop

    //Update screen with new RX cal val
    if(rollCalVal != oldrollCalVal){
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.write("    ");
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(198);
      mySerial.print(rollCalVal);
      oldrollCalVal = rollCalVal;
    }
    //Update screen with new RX cal val
    if(roll != oldrollVal){
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(135);
      mySerial.write("    ");
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(135);
      mySerial.print(roll);
      oldrollVal = roll;
    }


    //Button +/- press detection
    if(BtnPlusYesSTATE == LOW){
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnPlusYesSTATE == HIGH){
          SendCompassVals( "r", "+");
          break;
        }
      }
    }
    if(BtnMinusNoSTATE == LOW){
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnMinusNoSTATE == HIGH){
          SendCompassVals( "r", "-");
          break;
        }
      }
    }

    // exit back to menu
    if(BtnOkSTATE == LOW){                // exit cal adjust
      while(true){
        ButtonCheckUpdate();    // Check state of buttons and save value
        if(BtnOkSTATE == HIGH){
          break;
        }// end of if
      }//end if while;
      break;  
    }
  }
}
///////////////////////// END /////////////////////////////////////////







////////////////////// Check and update button state ///////////////////
void ButtonCheckUpdate(){
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();
  BtnPlusYesSTATE = debouncer1.read();
  BtnMinusNoSTATE = debouncer2.read();
  BtnOkSTATE = debouncer3.read();
}
///////////////////////// END /////////////////////////////////////////







////////////// Send new Cal Val ///////////////////////////////////////
void SendCompassVals(String Type, String Adjustment){
  Udp.beginPacket(remote_ip, remote_port);
  Udp.print(Type);
  Udp.print(":");
  Udp.print(Adjustment);
  Udp.endPacket();
}
///////////////////////// END /////////////////////////////////////////




