
// Section Control code for AgOpenGPS V 4.3.10 (=V17) and V5.x.20 (=V20) for Arduino Nano

// by MTZ8302 see GitHub https://github.com/mtz8302 and Youtube Ma Ha MTZ8302 https://www.youtube.com/channel/UCv44DlUXQJKbQjzaOUssVgw

// can be used as output only = relais for sections
// or for documentation = input only = input to ESP32 -> enables mapping for active (=switch to GND) section in AOG
// or any combination = section control with input switch
// main and pressure switches are (ON)-0-(ON) toggle switches (GND)- 1.75V - (3,3V). So use 2 resistors GND- 1K - (COM on toggle switch) - 1K - 3.3V
// code fits for WEder section control PCB




byte vers_nr = 46;
//char VersionTXT[150] = " - 5. April 2021 by MTZ8302 (Nano Version, V4.3 + V5 ready)";

struct set {
	//User config: ***********************************************************************************
	uint8_t aogVersion = 20;			//V4.3.10 = 17, V4.6 + V5.x.20 = 20

//	uint8_t LEDWiFi_PIN = 13;			// WiFi Status LED 255 = off
//	uint8_t LEDWiFi_ON_Level = 1;		// 1 = HIGH = LED on high, 0 = LOW = LED on low

	uint16_t BaudRate = 38400;              //Baudrate = speed of serial port or USB or Bluetooth. AOG uses 38400 for UART

////the following lines should be configed by the user to fit the programm to the sprayer/ESP32
////GPIOs of the ESP32 (current setting is for the layout shown as example WIKI)
//
//// if only 1 flowrate is used, use left
////Example1: motor valve is controled only by Switch not by AOG, no Flowmeter, : RateControl..Equiped = false; RateSW..Equiped = true; RateControlPWM = false;
////Example2: PWM valve, with flowmeter all controled by AOG:   RateControl..Equiped = true; RateSW..Equiped = true; RateControlPWM = true;	
//	uint8_t RateControlLeftInst = 0;		//1 if Rate control is there, else: 0
//	uint8_t RateSWLeftInst = 1;			    //1 if Rate control Pressure switch is there, else: 0
//	uint8_t RateSWLeft_PIN = A6;			//Rate +/- switch
//	uint8_t RateControlPWM = 0;				//1 if PWM valve, 0 if Motor drive for pressure change		
//
//	uint8_t	FlowDirLeft_PIN = 12;			//Rate-Control Valve/Motor Direktion
//	uint8_t	FlowPWMLeft_PIN = 11;			//Rate-Control Valve PWM/Motor ON/OFF
//	uint8_t	FlowEncALeft_PIN = 255;			//Flowmeter left/1
//
//	uint8_t RateControlRightInst = 0;	    //1 if Rate control is there, else: 0
//	uint8_t RateSWRightInst = 0;			//1 if Rate control Pressure switch is there, else: 0	
//	uint8_t	RateSWRight_PIN = 255;			//Rate +/- switch (ON)-0-(ON) toggle switch (GND)- 1.75V - (3,3V)
//	uint8_t	FlowDirRight_PIN = 255;			//255  = unused Rate-Control Valve Direktion
//	uint8_t	FlowPWMRight_PIN = 255;			//255  = unused Rate-Control Valve PWM
//	uint8_t	FlowEncARight_PIN = 255;		//Flowmeter right/2 
//
//	uint8_t SectNum = 6;					// number of sections
//	uint8_t SectRelaysInst = 1;				//relays for SC output are equiped (0=no output, only documentation)
//	uint8_t SectRelaysON = 1;				//relays spray on 1 or 0 (high or low)
//	uint8_t Relay_PIN[16] = { 10,9,8,7,6,5,255,255,255,255,255,255,255,255,255,255 };  //GPIOs of ESP32 OUT to sections of sprayer HIGH/3.3V = ON
//	uint8_t Relais_MainValve_PIN = 255;		//PIN for Main fluid valve 255 = unused
//	uint8_t SectSWInst = 1;					//1 if section input switches are equiped, else: 0	
//	uint8_t SectSWAutoOrOn = 1;				//Section switches spray/auto on 1 = high = used with pullup, 0 = low = pulldown 
//
//	uint8_t SectSW_PIN[16] = { A0,A1,A2,A3,A4,A5,3,255,255,255,255,255,255,255,255,255 };// section switches to GPIOs of ESP32 GND = section off, open/+3.3V section auto/on
//
//	uint8_t	SectMainSWType = 1;				// 0 = not equiped 1 = (ON)-OFF-(ON) toggle switch or push buttons 2 = connected to hitch level sensor 3 = inverted hitch level sensor
//	uint16_t HitchLevelVal = 500;		    // Value for hitch level: switch AOG section control to Auto if lower than... ESP:2000 nano 500
//	uint8_t	SectMainSW_PIN = A7;			//ESP32 to AOG Main auto toggle switch open=nothing/AOG button GND=OFF +3,3=AOG Auto on	OR connected to hitch level sensor	
//	uint8_t	SectAutoManSW_PIN = 4;			// Main Auto/Manual switch 39:!!no internal pullup!!
//
//	uint8_t DocOnly = 0;					// 0: use as section control, 1: Documentation only = AOG writes the state of the input switches
//


	bool debugmode = false;
	bool debugmodeRelay = false;
	bool debugmodeSwitches = false;
	bool debugmodeDataFromAOG = false;
	bool debugmodeDataToAOG = false;

	// END of user config ****************************************************************************
}; set Set;


bool EEPROM_clear = false;



byte SCToAOG[14] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0}, SCToAOGOld[14] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Sentence up to V4.3 -----------------------------------------------------------------------------	
#define SCDataFromAOGHeaderV17  0xFA
#define SCDataToAOGHeaderV17  0xF9
#define SCDataSentenceToAOGLengthV17  10

// sentences to AOG V4.6 and up -------------------------------------------------------------------	
const byte FromAOGSentenceHeader[3] = { 0x80,0x81,0x7F };
#define FromSCHeader 0x7B
#define SCDataToAOGHeader  0xEA
#define SteerDataFromAOGHeader  0xFE  //take section info from steer packet
#define SCSettingsFromRCHeader  0xFC
#define SCDataSentenceToAOGLength  14

//write incoming Data to sentence array if it fits in
#define SentenceFromAOGMaxLength 14
byte SentenceFromAOG[SentenceFromAOGMaxLength], SentenceFromAOGLength;

//global, as serial/USB may not come at once, so values must stay for next loop
byte incomSentenceDigit = 0,DataToAOGLength;
bool isSteerDataFound = false, isRCSettingFound = false;
bool isSCDataFoundV17 = false;

#define incommingDataArraySize 3
byte incommingBytes[incommingDataArraySize][150], incommingBytesArrayNr = 0, incommingBytesArrayNrToParse = 0;
unsigned int incommingDataLength[incommingDataArraySize] = { 0,0,0 };

//// Data LED blink times: data available: light on; no data for 2 seconds: blinking
//unsigned int LED_WIFI_time = 0;
//unsigned int LED_WIFI_pulse = 2000;   //light on in ms 
//unsigned int LED_WIFI_pause = 1500;    //light off in ms
//boolean LED_WIFI_ON = false;
//unsigned long timeout, timeout2;



//ToDo: - Rate / flowmeter doesn't work, interrupt mode ect not defined
//      - line 260 PWM + Interrupt mode ect...
//		  - Right side Rate caluclation / output / Motordrive
//      - Manual mode for Rate Control



////values to decide position of section switch
//#define SWOFF 300 // analog in is lower than .. for off
//#define SWON  700 // analog in is lower than .. for auto/on
//
////analog value of the toggle switches (default: middle=unpressed)
//int MainSWVal = (SWOFF + SWON) / 2;
//int MainSWValOld = (SWOFF + SWON) / 2;
//int RateSWLeftVal = (SWOFF + SWON) / 2;
//int RateSWRightVal = (SWOFF + SWON) / 2;
//
////state of the switches HIGH = ON/Auto LOW = OFF	
//boolean SectSWVal[16] = { HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH };
//boolean AutoSWVal = HIGH;
byte debugmodeSwitchesBak = 3;  //0 = false 1 = true 3 = not used

//loop time variables in microseconds
const unsigned long LOOP_TIME_sendDataToAOG = 1000;// 400; //in msec; 1000 = 1 Hz
const unsigned long LOOP_TIME_WiFiAndBlinkHandle = 50;
const unsigned long SectSWDelayTime = 200;// 1500;//1400; //time the arduino waits after manual Switch is used before acception command from AOG in msec
unsigned long now, lastTime_LOOP_WiFiAndBlinkHandle = 0, lastTime_LOOP_DataToAOG = 0;
unsigned long DataFromAOGTime = 0;
unsigned long SectAutoSWTime = LOOP_TIME_sendDataToAOG;
unsigned long SectAutoSWlastTime = LOOP_TIME_sendDataToAOG;
unsigned long SectMainSWlastTime = LOOP_TIME_sendDataToAOG;
unsigned long RateSWlastTimeLeft = LOOP_TIME_sendDataToAOG;
unsigned long RateSWDelayTime = 250; //time to pass before a new toggle of switch is accepted = if hold down, time between steps
unsigned long RateSWlastTimeRight = LOOP_TIME_sendDataToAOG;
unsigned long SectSWcurrentTime = LOOP_TIME_sendDataToAOG;


/* no rate control in AOG
//Kalman variables Left
float rateKLeft = 0;
float PcLeft = 0.0;
float GLeft = 0.0;
float PLeft = 1.0;
float XpLeft = 0.0;
float ZpLeft = 0.0;
float XeRateLeft = 0; //the filtered flowrate
const float varRateLeft = 100; // variance, smaller, more filtering
const float varProcessLeft = 10;

//Kalman variables Right
float rateKRight = 0;
float PcRight = 0.0;
float GRight = 0.0;
float PRight = 1.0;
float XpRight = 0.0;
float ZpRight = 0.0;
float XeRateRight = 0; //the filtered flowrate
const float varRateRight = 100; // variance, smaller, more filtering
const float varProcessRight = 10;
*/

//program flow
//bool NewDataFromAOG = false, NewSettingFromAOG = false;

//bit 0 on byte[0] is section 1
byte SectGrFromAOG[] = { 0, 0 }; //Sections ON / OFF Bitwise !!! 
byte RelayOUT[] = { 0,0 }; //Sections ON / OFF Bitwise !!! 
byte RelayOUTOld[] = { 0,0 };
byte SectSWOffToAOG[] = { 0,0 };
byte SectMainToAOG = 0;
byte uTurnRelay = 0;
//  byte flowRateSW1 = 0;
boolean SectMainSWpressed = false;
boolean SectSWpressed = false;
boolean SectSWpressedLoop = false;
boolean SectAuto = true;
boolean SectAutoOld = true;
boolean SectAutoSWpressed = false;
//boolean RateAuto = false;
boolean RateManUpLeft = false;
boolean RateManUpRight = false;
boolean RateManDownLeft = false;
boolean RateManDownRight = false;

boolean SectMainOn = false;

boolean newDataToAOG = false;// newDataFromAOG = false;

float gpsSpeed = 0; //speed from AgOpenGPS
//float rateSetPointLeft = 0.0;
//float countsThisLoopLeft = 0;
//float rateErrorLeft = 0; //for PID
//int rateAppliedLPMLeft = 0;
//float rateSetPointRight = 0.0;
//float countsThisLoopRight = 0;
//float rateErrorRight = 0; //for PID
//int rateAppliedLPMRight = 0;
//
//unsigned long accumulatedCountsLeft = 0;
//float flowmeterCalFactorLeft = 50;
//unsigned long accumulatedCountsRight = 0;
//float flowmeterCalFactorRight = 50;
//unsigned long accumulatedCountsDual = 0;
//
//float pulseAverageLeft = 0;
float pulseAverageRight = 0;

//the ISR counter
volatile unsigned long pulseCountLeft = 0, pulseDurationLeft;
volatile unsigned long pulseCountRight = 0, pulseDurationRight;
bool state = false;
///////////////////////////////////////////////////

#include <SoftwareSerial.h>
const int SS_RX_PIN = 3;
const int SS_TX_PIN = 4;

SoftwareSerial Serialrt(SS_RX_PIN, SS_TX_PIN);


#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
MCP2515 mcp2515(10);
bool spray = 0;

int boom = 0;
int boomA = 0;



////////////////////////////////////////////////////////:


void setup()
{
	delay(200);//wait for power to stabilize
	delay(200);//wait for IO chips to get ready
	//set up communication
	Serial.begin(Set.BaudRate);
	delay(10);
	delay(50);
	Serial.println();
	Serial.println("Section Control for AgOpenGPS");
	Serial.print("Version: "); Serial.println(vers_nr);
	Serial.println();
	if (Set.aogVersion == 17) {
		SCToAOG[0] = FromAOGSentenceHeader[2];
		SCToAOGOld[0] = FromAOGSentenceHeader[2];
		SCToAOG[1] = SCDataToAOGHeaderV17;
		SCToAOGOld[1] = SCDataToAOGHeaderV17;
		SCToAOG[2] = 0;
		SCToAOG[3] = 0;
		SCToAOG[4] = 0;
		SCToAOG[5] = 0;
		SCToAOG[6] = 0;
		DataToAOGLength = SCDataSentenceToAOGLengthV17;
		incomSentenceDigit = 2;
	}
	else {
		SCToAOG[0] = FromAOGSentenceHeader[0];   //0x80
		SCToAOG[1] = FromAOGSentenceHeader[1];   //0x81
		SCToAOG[2] = FromSCHeader;				 //0x7B
		SCToAOG[3] = SCDataToAOGHeader;			 //0xEA
		SCToAOG[4] = SCDataSentenceToAOGLength - 6; //length of data = all - header - length - CRC
		SCToAOGOld[0] = FromAOGSentenceHeader[0];
		SCToAOGOld[1] = FromAOGSentenceHeader[1];
		SCToAOGOld[2] = FromSCHeader;
		SCToAOGOld[3] = SCDataToAOGHeader;
		SCToAOG[4] = SCDataSentenceToAOGLength - 6;
		SCToAOG[6] = 0;
		SCToAOG[7] = 0;
		SCToAOG[8] = 0;
		DataToAOGLength = SCDataSentenceToAOGLength;
		incomSentenceDigit = 0;
	}





//	//set GPIOs
//	assignGPIOs();
//
//	delay(50);

   Serialrt.begin(115200);

    
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();
  
  Serialrt.println("------- CAN Read ----------");
  Serialrt.println("ID  DLC   DATA");


}


//-------------------------------------------------------------------------------------------------

void loop() {
	//Serial.println("begin of loop");
      if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
   if (canMsg.can_id== 0x90A85077){
        Serialrt.print(canMsg.can_id, HEX); // print ID
    Serialrt.print(" "); 
    Serialrt.print(canMsg.can_dlc, HEX); // print DLC
    Serialrt.print(" ");
    
//    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
//      Serialrt.print(canMsg.data[i],BIN);
//      Serialrt.print(" ");
//    }
    Serialrt.print(canMsg.data[3],BIN);
    Serialrt.print(" ");
    Serialrt.print(canMsg.data[6],BIN);
  if ( bitRead(canMsg.data[3], 5) ){
    boomA =~canMsg.data[6]; 
    Serialrt.print("Y ");
    Serialrt.print(" ");
  }else{
    boomA =~0;
        Serialrt.print("N ");
    Serialrt.print(" ");
  }
  if ( bitRead(canMsg.data[3], 7) ) Serialrt.print("7 ");
  if ( bitRead(canMsg.data[3], 6) ) Serialrt.print("6 ");
  if ( bitRead(canMsg.data[3], 5) ) Serialrt.print("5 ");
  if ( bitRead(canMsg.data[3], 4) ) Serialrt.print("4 ");
  if ( bitRead(canMsg.data[3], 3) ) Serialrt.print("3 ");
  if ( bitRead(canMsg.data[3], 2) ) Serialrt.print("2 ");
  if ( bitRead(canMsg.data[3], 1) ) Serialrt.print("1 ");
  if ( bitRead(canMsg.data[3], 0) ) Serialrt.print("0 ");

    Serialrt.println();
 
   }
}


  
	for (;;) {
		if (!getDataFromAOGUSB()) { break; }
	}
	//Serial.println("after get data");

	//new data from AOG? Data comes via extra task and is written into byte array. Parsing called here
	//if (incommingDataLength[incommingBytesArrayNrToParse] != 0) { parseDataFromAOG(); }
	//else { delay(1); }//wait if no new data to give time to other tasks 

	//read switches/inputs
//	if ((Set.SectSWInst) || (Set.SectMainSWType != 0))
//	{
	//	SectSWRead(); //checks if section switch is toggled and sets RelayOUT

		//reset debugSwitches: has been change to true, if debugmodeDataToAOG = true for 1 loop
//		if ((Set.debugmodeSwitches) && (debugmodeSwitchesBak == 0)) { Set.debugmodeSwitches = false; }

//		if (Set.aogVersion == 17) {
//			//SCToAOG 5+6 set in ReadSwitches_buildBytes HiByte, LowByte if bit set -> AOG section forced ON
//			SCToAOG[7] = SectSWOffToAOG[1]; //HiByte if bit set -> AOG section forced off
//			SCToAOG[8] = SectSWOffToAOG[0]; //LowByte if bit set -> AOG section forced off
//			SCToAOG[9] = SectMainToAOG; // Bits: AOG section control AUTO, Section control OFF, Rate L+R ...
//			//new data?
//			for (byte idx = 2; idx < DataToAOGLength; idx++) {
//				if (SCToAOG[idx] != SCToAOGOld[idx]) {
//					newDataToAOG = true;
//					SCToAOGOld[idx] = SCToAOG[idx];
//				}
//			}
//		}
	//	else {
			SCToAOG[10] = boomA;//random(256);//SectSWOffToAOG[0]; //LowByte if bit set -> AOG section forced off
			SCToAOG[12] = random(256);//SectSWOffToAOG[1]; //HiByte if bit set -> AOG section forced off
			SCToAOG[5] = 128;//SectMainToAOG; // Bits: AOG section control AUTO, Section control OFF, Rate L+R ...
			//new data?
			for (byte idx = 5; idx < DataToAOGLength; idx++) {
				if (SCToAOG[idx] != SCToAOGOld[idx]) {
					newDataToAOG = true;
					SCToAOGOld[idx] = SCToAOG[idx];
				}
			}
	//	}
	//}
	//no switches: set Relais as AOG commands
//	else { RelayOUT[0] = SectGrFromAOG[0]; RelayOUT[1] = SectGrFromAOG[1]; }

	//SetRelays();

	//Rate switches and motor drive
//	if ((Set.RateSWLeftInst == 1) || (Set.RateSWRightInst == 1)) { RateSWRead(); }
//
//	if (Set.RateControlLeftInst == 0) { motorDrive(); } //if Manual do everytime, not only in timed loop

	now = millis();

	//50ms LOOP
//	if (now > LOOP_TIME_WiFiAndBlinkHandle + lastTime_LOOP_WiFiAndBlinkHandle) {
//
//		//check if data is comming in as it should
//		if ((now > (DataFromAOGTime + 600)) && (DataFromAOGTime > 0)) {
//			if (SectAuto) { WiFi_LED_blink(3); }
//			else { WiFi_LED_blink(0); }
//
//			//check WiFi connection and reconnect if neccesary
//			//if ((SCSet.DataTransVia == 1) && (NetWorkNum > 0)) { WiFi_connection_check(); }
//		}
//		else {
//			if (!LED_WIFI_ON) {// turn LED on
//				digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level);
//				LED_WIFI_ON = true;
//			}
//		}
//
//		now = millis();
//		lastTime_LOOP_WiFiAndBlinkHandle = now;
//	}

	

	//1000 ms timed loop: runs with 1Hz or if new data from switches to send data to AOG
	//must run, also if nothing changed on switches, to keep GUI syncron (important if a section switch is off)
	if ((now > LOOP_TIME_sendDataToAOG + lastTime_LOOP_DataToAOG) || (newDataToAOG)) {

		lastTime_LOOP_DataToAOG = millis();

		if (Set.aogVersion != 17) {
			//add the checksum
			int CRCtoAOG = 0;
			for (byte i = 2; i < sizeof(SCToAOG) - 1; i++)
			{
				CRCtoAOG = (CRCtoAOG + SCToAOG[i]);
			}
			SCToAOG[sizeof(SCToAOG) - 1] = CRCtoAOG;

		}
  Serial.write(SCToAOG, DataToAOGLength);
	//	AOGDataSend();
		if (newDataToAOG) {
			delay(5);
			newDataToAOG = false;
			//AOGDataSend(); //send 2. time for safety
      Serial.write(SCToAOG, DataToAOGLength);
			//delay(10);
			//AOGDataSend(); //send 3. time for safety
		}

		if (Set.debugmodeDataToAOG) {
			Serial.print("Data to AOG ");
			for (byte n = 0; n < DataToAOGLength; n++) {
				Serial.print(SCToAOG[n]); Serial.print(" ");
			}
			Serial.println();
			//printing swich input 1x:
			if (!Set.debugmodeSwitches) {
				debugmodeSwitchesBak = 0;
				Set.debugmodeSwitches = true;
			}
			else debugmodeSwitchesBak = 1;
		}

	}
}
