/*
  Copyright (c) 2022 Kim Lilliestierna. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library in the LICENSE file. If not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  or via https://www.gnu.org/licenses/gpl-3.0.en.html

*/

#include <Arduino.h>
#include <mr24fdb1.h>


// Enable all debug macros
#define DEBUG_LEVEL (DBG_ALL_L)

// enable just default (ERROR,FAIL,WARNING) plus INFO
// #define DEBUG_LEVEL (DBG_DEF_L|DBG_INFO_L)

#include "debug.h"

// create radar instance
MR24FDB1 radar;

// set up some blinken lights
#define LED  2
#define GREEN_LED 25
#define YELLOW_LED 26
#define RED_LED 27

void setup()
{
	long last;
	uint8_t buffer[255];
	uint8_t pat[]={0x55,0x01,0x02,0x3,0xAA};
	pinMode(LED,OUTPUT);
	pinMode(GREEN_LED,OUTPUT);
	pinMode(YELLOW_LED,OUTPUT);
	pinMode(RED_LED,OUTPUT);

	// Setup serial and wait for it to start
	Serial.begin(115200);
	delay(500);

	setDbgLvl(DBG_INFO_L,ON,NOCLR); // turn on fail,error and warn set by default adding info

	INFO("Setup...\n");

	// blink the lights to signal startup
	digitalWrite(GREEN_LED,HIGH);
	delay(500);
	digitalWrite(YELLOW_LED,HIGH);
	delay(500);
	digitalWrite(RED_LED,HIGH);
	delay(1000);

	radar.init();

	digitalWrite(RED_LED,LOW);
	delay(500);
	digitalWrite(YELLOW_LED,LOW);
	delay(500);
	digitalWrite(GREEN_LED,LOW);
		
	
	INFO("Clearing incoming serial buffer from radar module\n");

	radar.readPacket();

	try {
		
		// get some info, uncomment as needed
/*		
		INFO("Main: Gett Device Id:\n");
		memcpy(buffer,radar.getDeviceId().c_str(),MR24FDB1::DEVICE_ID_LEN);
		dbgHexDump(buffer,MR24FDB1::DEVICE_ID_LEN);
		putchar('\n');
		radar.readPacket();
	
		printf("Hardware Ver:\n");
		memcpy(buffer,radar.getHardwareVer().c_str(),MR24FDB1::HARDWARE_VER_LEN);
		dbgHexDump(buffer,MR24FDB1::HARDWARE_VER_LEN);
		putchar('\n');
		radar.readPacket();

		printf("Software Ver:\n");
		memcpy(buffer,radar.getSoftwareVer().c_str(),MR24FDB1::SOFTWARE_VER_LEN);
		dbgHexDump(buffer,MR24FDB1::SOFTWARE_VER_LEN);
		putchar('\n');
		radar.readPacket();

		printf("Protocol Ver:\n");
		memcpy(buffer,radar.getProtocolVer().c_str(),MR24FDB1::PROTOCOL_VER_LEN);
		dbgHexDump(buffer,MR24FDB1::PROTOCOL_VER_LEN);
		putchar('\n');
		radar.readPacket();
*/

		INFO("main: Disabling fall detection\n");
		radar.setFallDetect(false);
		// just wait around for next packet
		radar.waitFor(MR24FDB1::Res_Movement);

		INFO("main: Setting sensistivity to 4\n");
		radar.setSensitivity(5);
		// just wait around for next packet
		radar.waitFor(MR24FDB1::Res_Movement);

		INFO("main: setting scene to office\n");
		radar.setScene(MR24FDB1::OFFICE_SC);
		// this should respond with a scene responce but module bugged
		// so just wait around for next packet
		radar.waitFor(MR24FDB1::Res_Movement);
		
		INFO("setup: Disableing unoccupied timeout\n");
		radar.setUnoccupiedTime(MR24FDB1::UT_NONE);
		// respond with unoccupied time responce but we dont care
		// so just wait around for next packet
		radar.waitFor(MR24FDB1::Res_UnoccupiedTime_Rsp);

	}

	catch (MR24FDB1_Exception e) {
		ERROR("setup: Radar setup failed\n");
		e.decodeError();
	}

	INFO("Setup Done\n");
//	SET_DBG((DBG_DEF_L|DBG_123FV_L),ON,CLR);
//	SET_DBG(DBG_12FV_L,ON,NOCLR);
	SET_DBG(DBG_DEF_L,ON,CLR);
	// Presence|Location|Heartbeat|Movement
	printf("|             | P | L | H |  M  |\n");
}

void loop()
{
	static MR24FDB1::Result_t result;
	float move;
	MR24FDB1::Presence_t presence;
	MR24FDB1::Location_t location;
	MR24FDB1::HeartBeat_t heartbeat;
	static long lastTime;
	long now,delta;
	uint8_t pch,lch,hch;	
	static int loopCount=0;

	digitalWrite(LED,HIGH);

	now=millis();
	delta=now-lastTime;
	lastTime=now;

	try {

		radar.readPacket();
		result=radar.parsePacket();

		DBG1("main: getting Presence\n");
		presence=radar.getPresence(5000);
		switch(presence) {
			case MR24FDB1::P_NOBODY:
				pch=' ';
				break;
			case MR24FDB1::P_PRESENT:
				pch='P';
				break;
			case MR24FDB1::P_MOVING:
				pch='M';
				break;
			default:
				pch='x';
				WARN("Uknown Presence received ");
		}

		DBG1("main: getting Location\n");
		location=radar.getLocation();
			switch(location) {
			case MR24FDB1::L_STATIONARY:
				lch=' ';
				break;
			case MR24FDB1::L_CLOSE: 
				lch='C';
				break;
			case MR24FDB1::L_AWAY:
				lch='F';
				break;
			case MR24FDB1::L_APROACHING:
				lch='v';
				break;
			case MR24FDB1::L_LEAVING:
				lch='^';
				break;
			default:
				lch='x';
		}

		DBG1("main: getting HeartBeat\n");
		heartbeat=radar.getHeartBeat();

		switch(heartbeat) {
			case MR24FDB1::HB_NONE:
				hch=' ';
				break;
			case MR24FDB1::HB_STATIONARY:
				hch='S';
				break;
			case MR24FDB1::HB_MOVEMENT:
				hch='M';
				break;
			default:
				hch='x';
		}
		// get freshest movement strength
		move=radar.getMovement(5000);

		if(move) {
			if(move>3) {
				digitalWrite(YELLOW_LED,HIGH);
			} else {
				digitalWrite(YELLOW_LED,LOW);
			}
			digitalWrite(GREEN_LED,HIGH);
		} else {
			digitalWrite(GREEN_LED,LOW);
			digitalWrite(YELLOW_LED,LOW);
		}
	
		printf("[%04.4d : %4.4d]| %c | %c | %c | %03.0f |\r", loopCount++,delta ,pch ,lch ,hch ,move);
		fflush(stdout);
	}
	catch(MR24FDB1_Exception e) {
		ERROR("Getting presence/location/heartbeat / movement failed\n");
		e.decodeError();
		// dont care why reset the chip
		radar.reboot();
	}
	
	digitalWrite(LED,LOW);

//    radar.Situation_judgment();       //Use radar built-in algorithm to output human motion status
}