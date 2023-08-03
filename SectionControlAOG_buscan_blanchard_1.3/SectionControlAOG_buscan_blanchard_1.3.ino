/* adapte depuis le code de
    V2.50 - 07/04/2023 - Daniel Desmartins
   Connected to the Relay Port in AgOpenGPS

lecture du bus can  sur un pulve  blanchard Kuhn  equipe d'un genius  REB3 afin de colorier la carte
   bric bric  23/05/2023
   correction  31/07/23
*/




#include <SPI.h>
#include <mcp2515.h>  //   https://github.com/autowp/arduino-mcp2515

struct can_frame canMsg;
MCP2515 mcp2515(10);


int boomA = 0;
int boomB = 0;

uint8_t  mainByte = 1;  // on donne la valeur  au section automatique = 1  ou   manuel  = 0  ou rien  2  pas d'interet

//Variables:
const uint8_t loopTime = 100; //10hz
uint32_t lastTime = loopTime;
uint32_t currentTime = loopTime;

//Comm checks
uint8_t watchdogTimer = 12;      //make sure we are talking to AOG
uint8_t serialResetTimer = 0;   //if serial buffer is getting full, empty it

//Parsing PGN
bool isPGNFound = false, isHeaderFound = false;
uint8_t pgn = 0, dataLength = 0;
int16_t tempHeader = 0;

//hello from AgIO
uint8_t helloFromMachine[] = { 128, 129, 123, 123, 5, 0, 0, 0, 0, 0, 71 };
bool helloUDP = false;
//show life in AgIO
uint8_t helloAgIO[] = { 0x80, 0x81, 0x7B, 0xEA, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0x6D };
uint8_t helloCounter = 0;

uint8_t AOG[] = { 0x80, 0x81, 0x7B, 0xEA, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0xCC };

//The variables used for storage
uint8_t relayLo = 0, relayHi = 0;

uint8_t onLo = 0, offLo = 0, onHi = 0, offHi = 0 ;
//End of variables

void setup() {

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();


  Serial.begin(38400);  //set up communication
  while (!Serial) {
    // wait for serial port to connect. Needed for native USB
  }


} //end of setup

void loop() {

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    if (canMsg.can_id == 0x90A85077) { // recherche de la trame  qui porte l'info

      if ( bitRead(canMsg.data[3], 5) ) { // si on pulverise on recupre laes section
        boomA = ~canMsg.data[6];    // on en profite pour inverser les bit
        boomB = ~canMsg.data[7];
      } else {  // sino  on mets a zero
        boomA = ~0;
        boomB = ~0;
      }


    }
  }


  //boomA = random(0,255); // pour simuler
 //boomB = random(0,255);



  currentTime = millis();
  if (currentTime - lastTime >= loopTime) {  //start timed loop
    lastTime = currentTime;


    //avoid overflow of watchdogTimer:
    if (watchdogTimer++ > 250) watchdogTimer = 12;

    //clean out serial buffer to prevent buffer overflow:
    if (serialResetTimer++ > 20) {
      while (Serial.available() > 0) Serial.read();
      serialResetTimer = 0;
    }


   


    //Send to AOG
    AOG[5] = (uint8_t)mainByte;
    AOG[9] = (uint8_t)~boomA;//onLo;
    AOG[10] = (uint8_t)boomA;//offLo;
    AOG[11] = (uint8_t)~boomB;//onHi;
    AOG[12] = (uint8_t)boomB;//offHi;

    //add the checksum
    int16_t CK_A = 0;
    for (uint8_t i = 2; i < sizeof(AOG) - 1; i++)
    {
      CK_A = (CK_A + AOG[i]);
    }
    AOG[sizeof(AOG) - 1] = CK_A;

    Serial.write(AOG, sizeof(AOG));
    Serial.flush();   // flush out buffer
  }
  //}

  // Serial Receive
  //Do we have a match with 0x8081?
  if (Serial.available() > 4 && !isHeaderFound && !isPGNFound)
  {
    uint8_t temp = Serial.read();
    if (tempHeader == 0x80 && temp == 0x81)
    {
      isHeaderFound = true;
      tempHeader = 0;
    }
    else
    {
      tempHeader = temp;     //save for next time
      return;
    }
  }

  //Find Source, PGN, and Length
  if (Serial.available() > 2 && isHeaderFound && !isPGNFound)
  {
    Serial.read(); //The 7F or less
    pgn = Serial.read();
    dataLength = Serial.read();
    isPGNFound = true;


  }

  //The data package
  if (Serial.available() > dataLength && isHeaderFound && isPGNFound)
  {
    if (pgn == 239) // EF Machine Data
    {
      Serial.read();
      Serial.read();
      Serial.read();
      Serial.read();
      Serial.read();   //high,low bytes
      Serial.read();

      relayLo = Serial.read();          // read relay control from AgOpenGPS
      relayHi = Serial.read();

      //Bit 13 CRC
      Serial.read();

      //reset watchdog
      watchdogTimer = 0;

      //Reset serial Watchdog
      serialResetTimer = 0;

      //reset for next pgn sentence
      isHeaderFound = isPGNFound = false;
      pgn = dataLength = 0;


    }
    else if (pgn == 200) // Hello from AgIO
    {
      helloUDP = true;

      Serial.read(); //Version
      Serial.read();

      if (Serial.read())
      {
        relayLo -= 255;
        relayHi -= 255;
        watchdogTimer = 0;
      }

      //crc
      Serial.read();

      helloFromMachine[5] = relayLo;
      helloFromMachine[6] = relayHi;

      Serial.write(helloFromMachine, sizeof(helloFromMachine));

      //reset for next pgn sentence
      isHeaderFound = isPGNFound = false;
      pgn = dataLength = 0;
    }
    else { //reset for next pgn sentence
      isHeaderFound = isPGNFound = false;
      pgn = dataLength = 0;
    }
  }
} //end of main loop
