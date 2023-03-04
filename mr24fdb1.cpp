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

#include "Arduino.h"
#include "mr24fdb1.h"

//#define DEBUG_LEVEL (DEF_DBGL|INFO_DBGL)
#include "dbgLevels.h"
#define DEBUG_LEVEL (DBG_DEF_L|DBG_INFO_L|DBG_123FV_L|DBG_VERB_L)
#include "debug.h"

#define COUNT(arr) (sizeof(arr)/sizeof(char *))
const char* MR24FDB1::ClassStr[]={ 
	/*0*/ "NoClass",	
	/*1*/ "GET",
	/*2*/ "SET",
	/*3*/ "RESPONCE",
	/*4*/ "REPORT",
	/*5*/ "FALL"
};
const int MR24FDB1::ClassStrCount=COUNT(ClassStr);

const char* MR24FDB1::GroupStr[]={ 
	/*0*/ "NoGroup",
	/*1*/ "IDENT",
	/*2*/ "Uknown 2",
	/*3*/ "RADAR",
	/*4*/ "SYSTEM",
	/*5*/ "OTHER"
};
const int MR24FDB1::GroupStrCount= COUNT(GroupStr);

const char* MR24FDB1::FuncStr[]={ 
	/*00*/	"NoFunc",
	/*01*/	"Device - Heartbeat",
	/*02*/	"Software",
	/*03*/	"Hardware",
	/*04*/	"Protocol",
	/*05*/	"Presence",	
	/*06*/	"Movement",
	/*07*/	"Location",
	/*08*/	"Upgrade",
	/*09*/	"Upgrade pkt - ACK",
	/*0A*/	"Init Success",
	/*0B*/	"Fall enable",
	/*0C*/	"Sensitivity",
	/*0D*/	"Unkown Func 0x0D",	 // this func code is not documented anywhere
	/*0E*/	"Fall Sensitivity",
	/*0F*/	"Uknown Func 0x0F", // this func code is not documented anywhere
	/*10*/	"Scene",
	/*11*/	"Uknown Func 0x11", // this func code is not documented anywhere
	/*12*/	"UnOccupied Time", 	// Unoccupied timeout
};
const int MR24FDB1::FuncStrCount = COUNT(FuncStr);

const char* MR24FDB1::SceneStr[] = {
	"Default",
	"Area detection",
	"Bathroom",
	"Bedroom",
	"Living Room",
	"Office",
	"Hotell"
};
const int MR24FDB1::SceneStrCount = COUNT(SceneStr);

const char* MR24FDB1::UnoccupiedTimeStr[] = {
	"no Timeout",
	"10 Sec",
	"30 Sec",
	"1 Min",
	"2 Min",
	"5 Min",
	"10 Min",
	"30 Min",
	"60 Min",
};
const int MR24FDB1::UnoccupiedTimeStrCount = COUNT(SceneStr);

// TODO: this needs pins/serial and baudrate as args....
MR24FDB1::MR24FDB1(uint8_t rxbit, uint8_t txbit) {

	PacketReceived=false;
	memset(&Packet,0,sizeof(Packet));

	Status = Res_Undefined;		///< running status
	Status_ts = millis();
	
	UpgradeStatus = Res_Undefined; 
	UpgradeStatus_ts = 0;
	
	Presence=P_NOBODY;			///< last active report Envrioment Status
	Presence_ts=0;				///< Presence time stamp

	Movement=0;					///< movment strengtht
	Movement_ts=0;				///< time stamp

	Location=L_STATIONARY;					///< presence location far, close none
	Location_ts=0;				///< time stamp
		
	HeartBeat=HB_NONE;			///< Heartbeat detection
	HeartBeat_ts=0;
	
	Sensitivity=0;
	Sensitivity_ts=0;
	
	FallAlarm=NO_FALL_DATA;
	FallAlarm_ts=0;
	
	FallWarning=NO_FALL_WARNING;
	FallWarning_ts=0; 
	
	FallSensitivity=0;
	FallSensitivity_ts=0;

	Scene = DEFAULT_SC;
	Scene_ts=0;

	UnoccupiedTime = UT_10S;
	UnoccupiedTime_ts=0;

	UpgradeStartAck = false;
	UpgradeStartAck_ts = 0;
	
	FallEnable=false;
	FallEnable_ts=0;
	
	RXBit = rxbit; 
	TXBit = txbit; 
	
	GotResponce=false;
	DeviceId[0]=SoftwareVer[0]=ProtocolVer[0]=0;
	

}

MR24FDB1::Result_t MR24FDB1::init() {
	uint8_t aByte=0;
	MR24FDB1::Result_t retVal;
	INFO(" Radar Init\n");

	//Serial1.begin(9600, SERIAL_8N1, RXBit, TXBit);
	Serial2.begin(9600, SERIAL_8N1);
	delay(1000);
	INFO("Reading a packet");
	retVal=readPacket();
	//	Reboot seems do do nothing....
	//	It should give an init successfull responce
//	retVal=reboot();
	Status=Res_Init;
	Status_ts=millis();
	INFO("Radar Init Done\n");
	return MR24FDB1::Res_Ok;
}

MR24FDB1::Result_t MR24FDB1::readByte(uint8_t * inByte) {
	unsigned long time;

	time = millis();

	while (Serial2.available() <= 0)
	{
		if(millis() - time > MAX_SERIAL_WAIT_TIME) {
		  WARN("readByte Timeout\n");
		  return MR24FDB1::Res_Timeout;
		}
	}
	*inByte = Serial2.read();
	return MR24FDB1::Res_Ok;
}

// --- Fetch datapacket and process
// TODO: rewrite this to use read(buffer *,lenght) to be mor efficient....
// also implement intrupt driven read and firing of events for the different report types...

MR24FDB1::Result_t MR24FDB1::readPacket()
{
	uint8_t		inByte; 						// incoming byte
	int 		lenCount = 0;							// lenght counter
	bool		done = false;
	Result_t	retVal=MR24FDB1::Res_Ok;
	uint8_t 	*pp;
	uint16_t	crc;
	uint16_t	pktCrc;
	uint16_t 	pktLength = 0;

	unsigned long time;
	unsigned long maxTime;

	time = millis();
	maxTime = 1000;			// max millisec to wait

	PacketReceived = false;
	
	// wait for header
	inByte = 0;

	DBG3("readPacket: Waiting for start code\n");
	while (inByte != MESSAGE_HEAD) {
		if((retVal=readByte(&inByte)) != Res_Ok) return retVal;
	}
	Packet.frameStart=inByte;
	
	DBG3("Getting Length....:");

	// got length bytes yet ?
	
	while(lenCount<2) {
		if((retVal=readByte(&inByte)) != Res_Ok) return retVal;
		pktLength |= inByte<<(8*lenCount);
		lenCount++;
	}

	if (pktLength > MAXPACKET_LENGTH)  {
		DBG3NL;
		ERROR("Length (%d) to high, Aborting\n",pktLength);
		return Res_PacketToLong;
	}

	DBG3V_S("%d (%4.4x)",pktLength,pktLength);
	DBG3NL;
	
	Packet.length=pktLength;

	DBG3("Getting rest of packet\n");
	pp = ((uint8_t *) &Packet.funcClass) ; // step over length and header

	for (int pi = 2; pi <= pktLength;pi++) {		// pi=2 -> we alreade received 2 bytes and header doesn count
		if((retVal=readByte(&inByte)) != Res_Ok) return retVal;
		*pp++=inByte;
	}
	
	uint8_t *RawPkt = (uint8_t *)&Packet;
	pktCrc=RawPkt[pktLength] | (RawPkt[pktLength-1]<<8); // get crc at end of received packet
														 // and yes pktLength is a coorect index since the received length
														 // does not count the header byte...
	crc=calculateCrc16();										// Calculate actual crc on received packet
	if (pktCrc != crc){
		DBG3NL;
		ERROR("CRC Check Failed: PktCrc: %d (%4.4x), CaclCrc: %d (%4.4x)\n", pktCrc,pktCrc,crc,crc);
		throw MR24FDB1_Exception( Res_InvCrc,0,0,0,crc);
		return Res_InvCrc;
	}

	PacketReceived = true;
	return Res_Ok;
}

// -- Send Packet

void MR24FDB1::sendPacket(uint8_t *txPacket,int len) {
	int count=0;
#if IS_VERB_L
	if(DbgLevel&DBG_VERB_L) {
		printf("SendPacket:");
		dbgHexDump(txPacket,len);
		putchar('\n');
	}
#endif
	while(Serial2.availableForWrite()<len) {
		delay(1);
		count++;
		if(count>1000) {
			FAIL("Transmit buffer never empties\n");
			throw MR24FDB1_Exception( Res_Timeout,0,0,0);
		}
	}
	Serial2.write(txPacket,len);
}

// -------------------------   GET request ---------------------------------

// -- SendGet

void MR24FDB1::sendGet(FuncGroups fGroup, Functions func) {
	uint8_t txPacket[]={MESSAGE_HEAD,0x07,0x00,GET,fGroup,func,0x00,0x00};
	DBG3("Sending \"GET %s/%s\" packet",GroupStr[fGroup],FuncStr[func]);
	DBG3V_S(" (0x%2.2x/ 0x%2.2x)",fGroup,func);
	DBG3NL;
	addCrc16(txPacket,sizeof(txPacket)); 
	sendPacket(txPacket,sizeof(txPacket));
}

// -- SendGetAndWait

MR24FDB1::Result_t MR24FDB1::sendGetAndWait(FuncGroups fGroup, Functions func,Result_t responce) {
	uint tries=0;
	bool done=false;
	MR24FDB1::Result_t result=Res_Ok;
	DBG3("SendGetAndWait for %2.2d% => s\n",responce,MR24FDB1_Exception::ResStr[responce] );
	while((result!=responce) && (tries++<5)) {
		DBG3("Sending GET resquest [%2.2d] of 5\n",tries);
		sendGet(fGroup,func);
		result=waitFor(responce);
	}
	if(result == Res_Timeout || tries>=5) {
		ERROR("getAndWait: Timeout waiting for (%2.2d) %s\n",responce, MR24FDB1_Exception::ResStr[responce]);
		throw MR24FDB1_Exception( Res_Timeout, GET,fGroup,func);
	}
	return result;
}

// -- waitFor 

MR24FDB1::Result_t MR24FDB1::waitFor(Result_t waitRes) {
	int count=1;
	Result_t result=Res_Error;
	DBG3V_S("waitFor: ");
	DBG3V_S(" %2.2d => %s",waitRes, MR24FDB1_Exception::ResStr[waitRes]);
	while(result!=waitRes && count<10) { 		//timeout after 10 packets is more than enough
		result=readPacket();
		try {
			result=parsePacket();
			VERB_S("waitfor [%d] - Got %2.2d => %s\n",count++,result, (result<Res_Error) ? MR24FDB1_Exception::ResStr[result] : "");
		}
		catch (MR24FDB1_Exception e) {
			ERROR("waitfor %2.2d => %s\n",waitRes,  MR24FDB1_Exception::ResStr[waitRes]);
			e.decodeError();
		}
		count++;
	}
	if(count>=10) {
		WARN("waitFor: Timeut waiting for %2.2d = %s\n",waitRes, MR24FDB1_Exception::ResStr[waitRes]);
		result=Res_Timeout;
	}
	return result;
	
}

string & MR24FDB1::getDeviceId() {
	DBG1("getDeviceId");
	if (DeviceId.length() == 0) {
		DBG1V_S(", No cache. Requesting from module ");
		sendGetAndWait(IDENT,DEVICE,Res_DeviceId_Rsp);  // might not come back, throws exception on timeout
	}
#if IS_1V
	dbgHexDump((const uint8_t *)DeviceId.c_str(),DEVICE_ID_LEN,false);
#endif
	DBG1NL;
	return DeviceId;
}


string &  MR24FDB1::getSoftwareVer() {
		DBG1("getSoftwareVer"); if(SoftwareVer.length() == 0){
		DBG1V_S(" No cache requesting from module ");
		sendGetAndWait(IDENT,SOFTWARE,Res_SoftwareVer_Rsp);  
	}
#if IS_1V
	dbgHexDump((const uint8_t *)SoftwareVer.c_str(),SOFTWARE_VER_LEN,false);
#endif
	DBG1NL;
	return SoftwareVer;
}

string & MR24FDB1::getHardwareVer() {
	DBG1("getHardwareVer");
	if(HardwareVer.length() == 0) {
		DBG1V_S(" No cache requesting from module ");
		sendGetAndWait(IDENT,HARDWARE,Res_HardwareVer_Rsp);
	}
#if IS_1V
	dbgHexDump((const uint8_t *)HardwareVer.c_str(),HARDWARE_VER_LEN,false);
#endif
	DBG1NL;
	return HardwareVer;
}

string & MR24FDB1::getProtocolVer() {
	DBG1("getProtocolVer");
	if(ProtocolVer.length()==0) {
		DBG1V_S(" No cache requesting from module ");
		sendGetAndWait(IDENT,PROTOCOL,Res_ProtocolVer_Rsp);
	}
#if IS_1V
	dbgHexDump((const uint8_t *)ProtocolVer.c_str(),PROTOCOL_VER_LEN,false);
#endif
	DBG1NL;
	return ProtocolVer;
}

MR24FDB1::Presence_t MR24FDB1::getPresence(long age) {
	DBG1("getPresence");
	if(age<=0 || millis()-Presence_ts > age) {
		DBG1V_S(" cache to old, getting new");
		sendGetAndWait(RADAR,PRESENCE,Res_Presence_Rsp);
	}
	DBG1V_S(": 0x%2.2x",Presence);
	DBG1NL;
	return Presence;
}


float MR24FDB1::getMovement(long age) {
	DBG1("getMovement");
	if(age<=0 || millis()-Movement_ts > age) {
		DBG2V_S(" cache to old, getting new ");
		sendGetAndWait(RADAR,MOVEMENT,Res_Movement_Rsp);
	}
	DBG1V_S(": %f\n",Movement);
	DBG1NL;
	return Movement;
}

uint8_t	MR24FDB1::getSensitivity(long age){
	DBG1("getSensitivity");
	if(age<=0 || millis()-Sensitivity_ts > age) {
		DBG1V_S("cache to old, getting new ");		
		sendGetAndWait(SYSTEM,SENSITIVITY,Res_Sensitivity_Rsp);
	}
	DBG1V_S(": %d",Sensitivity);
	DBG1NL;
	return Sensitivity;
}

MR24FDB1::Scene_t	MR24FDB1::getScene(long age){
	DBG1("getScene");
	if(age<=0 || millis()-Scene_ts>age) {
		DBG1V_S(" cache to old, getting new");		
		sendGetAndWait(SYSTEM,SCENE,Res_Scene_Rsp);
	}
	DBG1V_S(": %d",Sensitivity);
	DBG1NL;
	return Scene;
}


bool MR24FDB1::getFallEnable(long age){
	DBG1("getFallEnable");
	if(age<=0 || millis()-FallEnable_ts > age) {
		DBG1V_S(" cache to old, getting new ");		
		sendGetAndWait(OTHER,FALL_ENABLE,Res_FallEnable_Rsp);
	}
	DBG1V_S(": %s",FallEnable?"On":"Off");
	DBG1NL;
	return FallEnable;
}

MR24FDB1::FallWarnTime_t MR24FDB1::getFallWarnTime(long age){
	DBG1("getFallWarnTime");
	if(age<=0 || millis()-FallWarnTime_ts>age){
		DBG1V_S(" cache to old, getting new ");		
		sendGetAndWait(OTHER,FALL_WARN_TIME,Res_FallWarnTime_Rsp);
	}
	DBG1V_S(": %s",FallWarnTime?"On":"Off");
	DBG1NL;
	return FallWarnTime;
}

uint8_t	MR24FDB1::getFallSensitivity(long age){
	DBG1("getFallSensitivity");
	if(age<=0 || millis()-FallSensitivity_ts>age){
		DBG1V_S(" cache to old, getting new ");		
		sendGetAndWait(OTHER,FALL_SENSITIVITY,Res_FallSensitivity_Rsp);
	}
	DBG1V_S(": %d",FallSensitivity);
	DBG1NL;
	return FallSensitivity;
}

// -------------------------   SET request ---------------------------------

// TODO: should we wait for confirmation , ie the correct responce packet ?
// TODO: should  we verify it returned responce value is equal to requested value ?

void MR24FDB1::sendSet(FuncGroups fGroup, Functions func, uint8_t data, uint16_t dataLen) {
						
	uint8_t txPacket[]={ // construct txpacket 
		MESSAGE_HEAD,	// 0 allways 55
		0x00,0x00,		// 1,2 length low high
		SET,			// 3 "Set" Function Class
		fGroup,			// 4 Function group
		func,			// 5 Function
		data,			// 6 data
		0,0				// 7-8 crc
	}; 
	
	// some commands does not have data so need to adjust length
	// shifting crc down one byte
	
	// TODO: this needs to be changed if/when the firmware upgrade routines are written
	
	uint16_t pktLen=(dataLen == 0)? sizeof(txPacket)-1:sizeof(txPacket); 

	DBG2("Sending SET/%s/%s",GroupStr[fGroup],FuncStr[func]);
	DBG2V_S(" (0x%2.2x)",data);
	DBG2NL;
	
	// set length of packet beore crc calc...
	txPacket[1]=pktLen&0x00ff;
	txPacket[2]=pktLen&0xff00>>8;
	addCrc16(txPacket,pktLen);
#if V3_L
	dbgHexDump(txPacket,pktLen);
#endif
	sendPacket(txPacket,pktLen);
}

// ------------------ SYSTEM group

void MR24FDB1::setSensitivity(uint8_t sensVal){
	DBG1("Set Sensitivity");
	DBG1V_S(":  %d\n",sensVal);
	DBG1NL;
	sendSet(SYSTEM,SENSITIVITY,sensVal,1);
}

void MR24FDB1::setScene(Scene_t newScene) { // gives incoorrect responce: system/sensitvity
	DBG1("Set Scene");
	if(newScene>HOTEL_SC) {
		DBG1NL;
		ERROR("Invalid scene 0x%02.2x\n",newScene);
		throw MR24FDB1_Exception( Res_InvScene, SET,OTHER,SCENE);
	}
	DBG1V_S(": 0x%02.2x => %s",newScene,SceneStr[newScene]);
	DBG1NL;
	sendSet(SYSTEM,SCENE,newScene,1);
}

void MR24FDB1::setUnoccupiedTime(UnoccupiedTime_t time) { // gives incoorrect responce: system/sensitvity
	DBG1("Set Unoccupied Timeout");
	if(time>UT_60M) {
		DBG1NL;
		ERROR("Invalid time 0x%02.2x\n",time);
		throw MR24FDB1_Exception( Res_InvScene, SET,OTHER,SCENE);
	}
	DBG1V_S(": 0x%02.2x",time);
	DBG1NL;
	sendSet(SYSTEM,UNOCCUPIED_TIME,time,1);
}


// ----------- OTHER group ---------------

MR24FDB1::Result_t MR24FDB1::reboot() {
	MR24FDB1::Result_t result;
	DBG1("Rebooting Module ... \n");
	Status=Res_Undefined;
	sendSet(OTHER,REBOOT,0,0);
	result=waitFor( Res_InitSuccess);
	DBG1V("reboot: got 0%02.2x =>%s\n",result,MR24FDB1_Exception::ResStr[result]); 
	if(result != Res_InitSuccess) {
		WARN("No init succes after reboot!\n");
	}
	return result;
// should probably wait around here for a bit and clear out garbage comming from module during reboot
}

/* the hole upgrade thing is not implemented yet since there is not
 * realy any documentation on the feature
*/
void MR24FDB1::startUpgrade(uint32_t offset, uint8_t * versionP,uint16_t verLen){ 
	ERROR("startUpgrade not implented\n");
	throw MR24FDB1_Exception( Res_Unimplmented, SET,OTHER,UPGRADE);
}

void MR24FDB1::sendUpgradePacket(uint32_t offset, uint8_t * buffP){ 
	ERROR("sendUpgradePacket not implented\n");
	throw MR24FDB1_Exception( Res_Unimplmented, SET,OTHER,UPGRADE_PKT);
}

void MR24FDB1::endOfUpgrade(){ 
	ERROR("endOfUpgrade not implented\n");
	throw MR24FDB1_Exception( Res_Unimplmented, SET,OTHER,END_OF_UPGRADE);
};

void MR24FDB1::setFallDetect(bool onOff) {
	DBG1("SetFallDetect");
	DBG1V_S(": %s\n",onOff?"Enable":"Disable");
	DBG1NL;
	sendSet(OTHER,FALL_ENABLE,onOff?1:0,1);
}
	
void MR24FDB1::setFallWarnTime(FallWarnTime_t fallTime){
	DBG1("setFallWarnTime");
	DBG1V_S(": %d\n",fallTime);
	DBG1NL;
	sendSet(OTHER,FALL_WARN_TIME,fallTime,1);
}

void MR24FDB1::setFallSensitivity(uint8_t fallSensVal){
	DBG1("setFallSensistivity");
	DBG1V_S(": %d\n",fallSensVal);
	DBG1NL;
	sendSet(OTHER,FALL_SENSITIVITY,fallSensVal,1);
				
}
	

/// -------------------------- Parse incominig packets ----------------------

MR24FDB1::Result_t MR24FDB1::parsePacket() {
	DBG3("parsePacket :\n");
	switch(Packet.funcClass) {
		// can get and set ever happen as incomming from chip?
		case GET:					// get data packet !!! can this even happen ??  return parseGet(); break;
		case SET:					// Set data packet ??? can this happen on received data
			ERROR("Get / Set Packet received. This should not happen! (Func Class): %2.2x\n",Packet.funcClass);
			throw MR24FDB1_Exception( (Packet.funcClass==GET?Res_Unexpected_Get:Res_Unexpected_Set), Packet.funcClass,Packet.funcGroup,Packet.func);
			break;
		case RESPONCE:				// Passive report
			return parseResponce();
			break;
		case REPORT:
			return parseReport();
			break;
		case FALL:
			return parseFallReport();
			break;
		default:
			ERROR("Uknown Packet type (Func Class): %2.2x\n",Packet.funcClass);
			throw MR24FDB1_Exception( Res_UnknownPktClass, Packet.funcClass,Packet.funcGroup,Packet.func);
	}
}

// ----------------- responce ---------------------

/** parse an responce packet
 */
MR24FDB1::Result_t MR24FDB1::parseResponce() {
	uint32_t longVal;
	Result_t retVal=Res_InvRep;

	DBG2("Got: RESPONCE/");

	switch(Packet.funcGroup) {

		case IDENT:
			DBG2_S("Ident/");
			switch(Packet.func) {
				case DEVICE:
					DBG2_S("Device ID");
					Packet.deviceID[DEVICE_ID_LEN]='\0';
					DeviceId= string((char *) Packet.deviceID);
					retVal=Res_DeviceId_Rsp;
					DBG2V_S(": %s",DeviceId.c_str());
					break;

				case SOFTWARE:
					DBG2_S("Software Ver");
					Packet.softwareVer[SOFTWARE_VER_LEN]='\0';
					SoftwareVer=string((char *) Packet.softwareVer);
					retVal=Res_SoftwareVer_Rsp;
					DBG2V_S(": %s",SoftwareVer.c_str());
					break;

				case HARDWARE:
					DBG2_S("Hardware Version");
					Packet.hardwareVer[HARDWARE_VER_LEN]='\0';
					HardwareVer=string((char *) Packet.hardwareVer);
					retVal=Res_HardwareVer_Rsp;
					DBG2V_S(": %s",HardwareVer.c_str());
					break;

				case PROTOCOL:
					DBG2_S("Protocol Version");
					Packet.protocolVer[PROTOCOL_VER_LEN]='\0';
					ProtocolVer=string((char *) Packet.protocolVer);
					DBG2V_S(": %s",ProtocolVer.c_str());
					retVal=Res_ProtocolVer_Rsp;
					break;

				default:
					DBG2NL;
					WARN("Uknown Identification function: 0%02.2x\n");
					throw MR24FDB1_Exception( Res_InvIdentFunc,RESPONCE,IDENT,Packet.func);
			}
			break;

		case  RADAR:
			retVal=decodeRadar(RESPONCE);
			break;

		case SYSTEM: 
			DBG2_S("System/");
			switch(Packet.func) {
				case SENSITIVITY:
					DBG2_S("Sensitivity");
					retVal=Res_Sensitivity_Rsp;
					Sensitivity=Packet.sensitivity;
					Sensitivity_ts=millis();
					DBG2V_S(": %d (0x%02.2x)",Sensitivity,Sensitivity);
					break;
					
				case SCENE:
					DBG2_S("Scene");
					retVal=Res_Scene_Rsp;
					Scene=static_cast<Scene_t>(Packet.scene);
					Scene_ts=millis();
					DBG2V_S(": %s", SceneStr[Scene]);
					break;

				case UNOCCUPIED_TIME:
					DBG2_S("Unoccupied:");
					retVal=Res_UnoccupiedTime_Rsp;
					UnoccupiedTime=static_cast<UnoccupiedTime_t>(Packet.unoccupiedTime);
					UnoccupiedTime_ts=millis();
					DBG2V_S(": 0%02.2x", UnoccupiedTime);
					break;

				default:
					DBG2NL;
					ERROR("Uknown system function: 0x%02.2x\n",Packet.func);
					throw(Res_InvRspFunc,RESPONCE,SYSTEM,Packet.func);
					break;
			}
			break;

		case OTHER:
			DBG2_S("Other/");
			switch(Packet.func) {

				case UPGRADE:	// ota upgrade start ACK
					DBG2_S("Upgrade Start ACK");
					UpgradeStartAck=Packet.upgradeStartAck;
					UpgradeStartAck_ts=millis();
					retVal=(UpgradeStartAck  == 0)? Res_UpgradeStartFail_Rsp:Res_UpgradeStartOk_Rsp;
					DBG2V_S(": %s",UpgradeStartAck == 0? "Failed":"Ok");
					break;

				case FALL_ENABLE:
					DBG2_S("Fall switch Enable");
					FallEnable=Packet.fallEnable;
					FallEnable_ts=millis();
					retVal=Res_FallEnable_Rsp;
					DBG2V_S(": %s",FallEnable==0?"Off":"On");
					break;

				case FALL_WARN_TIME:		// 
					DBG2_S("Fall warning time");
					if (Packet.fallWarningTime>MIN_30) {
						throw(Res_InvFallWarnTime,RESPONCE,OTHER,Packet.func,Packet.fallWarning);
					}
					FallWarnTime=static_cast<FallWarnTime_t>(Packet.fallWarningTime);
					FallWarnTime_ts=millis();
					retVal=Res_FallWarnTime_Rsp;
					DBG2V_S(": %d",FallWarnTime);
					break;

				case FALL_SENSITIVITY:		// Responce to Set sensitivity
					DBG2_S("Fall sensitivity");
					FallSensitivity=Packet.fallSensitivity;
					FallSensitivity_ts=millis();
					retVal=Res_FallSensitivity_Rsp;
					DBG2V_S(": %d",FallSensitivity);
					break;
				
				default:
					DBG2NL;
					ERROR("Inv function 0x%02.2x\n",Packet.func);
					throw(Res_InvRspFunc,RESPONCE,OTHER,Packet.func);
					break;
			}
			break;
		default:
			DBG2NL;
			ERROR("Inv group 0x%02.2x\n",Packet.funcClass);
			throw(Res_InvRspFunc,RESPONCE,Packet.funcClass,Packet.func);
			break;
	}
	DBG2NL;
	return retVal;
}

/** parse an active report packet
 */

MR24FDB1::Result_t MR24FDB1::parseReport() {
	DBG2("REPORT/");
	uint32_t longVal;
	Result_t retVal=Res_InvRep;
	

	switch(Packet.funcGroup) {

		// -- Identitiy --
		case IDENT:					// Sent afte successfull upgrade
			DBG2_S("Ident/");
			if(Packet.func != SOFTWARE) {  // only 1 func in this group
				DBG1NL;
				ERROR("Inv function in IDENT group received : 0x%02.2x\n",Packet.func);
				throw MR24FDB1_Exception( Res_InvIdentFunc,REPORT,IDENT,Packet.func);
			} 
			DBG2_S("Software");
			DBG2V_S(": Upgrade success");
			UpgradeStatus = Res_UpgradeSuccess;
			retVal=	Res_UpgradeSuccess;
			break;
			
		// -- Radar group --
		
		case RADAR:					// Radar reports
			retVal=decodeRadar(REPORT);
			break;
			
		// --- Other
		
		case OTHER:
			DBG2_S("Other/");
			switch(Packet.func) {

				/// --- Heart beat

				case HEARTBEAT:
					DBG2_S("Heart Beat");
					longVal = Packet.heartBeat[0] | Packet.heartBeat[1]<<8 | Packet.heartBeat[2]<<16;
					switch(longVal) {
						case HB_NONE_BP:
							DBG2V_S(": No one present ");
							HeartBeat = HB_NONE;
							break;
						case HB_STATIONARY_BP:
							DBG2V_S(": Stationary");
							HeartBeat = HB_STATIONARY;
							break;
						case HB_MOVEMENT_BP:
							DBG2V_S(": Movment detected");
							HeartBeat = HB_MOVEMENT;
							break;
						default:
							DBG2NL;
							ERROR("Inv heart beat : 0x%02.2x 0x%02.2x 0x%02.2x\n",Packet.heartBeat[0] , Packet.heartBeat[1] , Packet.heartBeat[2]);
							throw MR24FDB1_Exception(Res_InvHeartBeat,REPORT,OTHER,HEARTBEAT, longVal);
							break;
					}
					HeartBeat_ts=millis();
					retVal=Res_HeartBeat;
					break;

				/// --- Software -> reset
				
				case SOFTWARE:  // Abnormal reset
					DBG2NL;
					if(Packet.abnReset == 0x0F ) {
						WARN("Abnormal Reset\n");
						retVal = Res_Reset;
					} else {
						ERROR("Inv Reset Class : 0x%02.2x\n",Packet.abnReset);
						throw MR24FDB1_Exception(Res_InvReset,REPORT,OTHER,SOFTWARE,Packet.abnReset);
					}
					break;
				
				case INIT_SUCCESS:
					DBG2_S("Init Success");
					Status=retVal=Res_InitSuccess;
					break;

				default: // Inv Other group
					DBG2NL;
					ERROR("Inv Other Function : 0x%02.2x\n",Packet.func);
					throw MR24FDB1_Exception(Res_InvOtherFunc,REPORT,OTHER,Packet.func);
			}
			break;
		default: // Inv Rerport group
			DBG2NL;
			ERROR("Inv Report group: 0x%02.2x\n",Packet.funcGroup);
			throw MR24FDB1_Exception(Res_InvRepGroup,REPORT,Packet.funcGroup,Packet.func);
	}
	DBG2NL;
	return retVal;
};

// ---  pase a fall packet
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!! this is untested !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
MR24FDB1::Result_t MR24FDB1::parseFallReport() {
	FallAlarm=NO_FALL_DATA;
	DBG2("FALL/");
    switch(Packet.funcGroup) {
        case F_ALARM_CL:
			DBG2_S("ALARM: ");
			switch(Packet.func) {

				case F_ALARM:
					switch (Packet.fallAlarm) {
						case F_SUSPECTED:
							DBG2V_S(": Suspected fall detected.");
							FallAlarm=F_SUSPECTED;
							break;
						case F_REAL:
                  			DBG2V_S(": Real fall detected.");
							FallAlarm=F_REAL;
							break;
						case F_NO_FALL:
                  			DBG2V_S(": No fall detected.n");
							FallAlarm=F_NO_FALL;
                  			break;
						default:
							DBG2NL;
							FallAlarm=NO_FALL_DATA;
							ERROR("An invalid fall alarm type (%2.2x) was received\n",Packet.fallAlarm);
							throw MR24FDB1_Exception(Res_InvFallAlarm,F_ALARM_CL,FALL,Packet.fallAlarm);
							break;
              		} // fallAlarm func
              		break;

				case F_WARNING:
					DBG2_S("WARNING");
              		switch (Packet.fallWarning){
                		case NO_FALL_WARNING:
                  			DBG2V_S(": No warning message at this time.");
							FallWarning=NO_FALL_WARNING;
                  		break;
                		case FALL_WARNING_1:
							DBG2V_S(": First fall Warning\n");
							FallWarning=FALL_WARNING_1;
						break;
						case FALL_WARNING_2:
							DBG2V_S(": Second fall warning\n");
							FallWarning=FALL_WARNING_2;
							break;
						case FALL_WARNING_3:
							DBG2V_S(": Third fall warning\n");
							FallWarning=FALL_WARNING_3;
							break;
						case FALL_WARNING_4:
							DBG2V_S(": Fourth fall warning\n");
							FallWarning=FALL_WARNING_4;
							break;
						default:
							DBG2NL;
							ERROR("An invalid fall Warning type (%2.2x) was received\n",Packet.fallWarning);
							throw MR24FDB1_Exception(Res_InvFallWarnCount,FALL,F_ALARM_CL,F_WARNING,Packet.fallWarning);
							break;
                	} // Warning func
              		break;

				default:
					DBG2NL;
					ERROR("An invalid Fall Radar Alarm Type was received (%d)\n",Packet.func);
					throw MR24FDB1_Exception(Rea_InvFallAlarmFunc,FALL,F_ALARM_CL,Packet.func);
		                                     
					break;
            } // Alarm group
          	break;

		default:
			DBG2NL;
			ERROR("Invalid Fall group (%2.2x)\n", Packet.funcGroup);
			throw MR24FDB1_Exception(Res_InvFallGroup,FALL,Packet.funcGroup,0);
			break;
    }
	DBG2NL;
	return MR24FDB1::Res_Fall;
}

//  --- decode radar

MR24FDB1::Result_t MR24FDB1::decodeRadar(MR24FDB1::FuncClasses fClass){
	MR24FDB1::Result_t retVal;
	uint32_t longVal;

	DBG2_S("Radar/");
	switch(Packet.func) { // --- Presence
		case PRESENCE:
			DBG2_S("Presence");
			switch(Packet.lval & 0x00FFFFFF) {
				case P_NOBODY_BP:
					DBG2V_S(": Nobody detected");
					Presence=P_NOBODY;
					break;
				case P_PRESENT_BP:
					DBG2V_S(": Presence detected");
					Presence=P_PRESENT;
					break;
				case P_MOVING_BP:
					DBG2V_S(": Movement detected");
					Presence=P_MOVING;
					break;
				default:
					DBG2NL;
					ERROR("Inv presence type received : 0x%02.2x 0x%02.2x 0x%02.2x \n",Packet.presence[0] , Packet.presence[1], Packet.presence[2]);
					throw MR24FDB1_Exception( Res_InvPresence,fClass,Packet.funcGroup,Packet.func,longVal);
					break;
			}
			Presence_ts=millis();
			retVal=(fClass==REPORT)?Res_Presence:Res_Presence_Rsp;
			break;
		// --- Movement
		case MOVEMENT:
			DBG2_S("Movement");
			Movement=Packet.movement;
			Movement_ts=millis();
			retVal=fClass==REPORT?Res_Movement:Res_Movement_Rsp;
			DBG2V_S(": %4.1f",Movement);
			break;

		// --- Location
		// Only happens in responces
		case LOCATION:		
			DBG2_S("Location");
			longVal = Packet.location[0] | Packet.location[1]<<8 | Packet.location[2]<<16;
			switch(longVal) {
				case L_STATIONARY_BP:
					DBG2V_S(": Stationary no one detected");
					Location = L_STATIONARY;
					break;
				case L_CLOSE_BP:
					DBG2V_S(": Someone is Close");
					Location = L_CLOSE;
					break;
				case L_AWAY_BP:
					DBG2V_S(": Someone Far");
					Location = L_AWAY;
					break;
				case L_APROACHING_BP:
					DBG2V_S(": Somenone is Aproaching");
					Location = L_APROACHING;
					break;
				case L_LEAVING_BP:
					DBG2V_S(": Someone is leaving");
					Location = L_LEAVING;
					break;
				default:
					DBG2NL;
					ERROR("Inv location indicator: 0x%02.2x 0x%02.2x 0x%02.2x 0x%02.2x\n",Packet.location[0],Packet.location[1],Packet.location[2]);
					throw MR24FDB1_Exception(Res_InvLocation,fClass,RADAR,LOCATION, longVal);
					break;
			}
			Location_ts=millis();
			retVal=Res_Location;
			break;

		default:
			DBG2NL;
			ERROR("Inv Radar function: 0%02.2x\n",Packet.func);
			throw MR24FDB1_Exception(Res_InvRadarFunc,fClass,RADAR,Packet.func);
			break;
	}
	return retVal;
}

/// --------------------------- CRC -------------------------------------
static const unsigned char crcHiTab[256]= {
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40
};

static const unsigned char  crcLoTab[256]= {
  0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
  0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
  0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
  0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
  0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
  0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
  0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
  0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
  0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
  0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
  0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
  0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
  0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
  0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
  0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
  0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
  0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
  0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
  0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
  0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
  0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
  0x41, 0x81, 0x80, 0x40
};

// Add crc to end of  buffer
uint16_t MR24FDB1::addCrc16(uint8_t *frameP,uint16_t len) {
	DBG3(" Adding crc");

	#pragma pack(1)
	union {
		uint16_t w;
		uint8_t b[2];
	} crc;
	#pragma pack()
	crc.w=calculateCrc16(frameP,len-2);
	frameP[len-1]=crc.b[0];
	frameP[len-2]=crc.b[1];
	DBG3V_S(": %d - 0x%2.2x 0x%2.2x", crc.w, crc.b[0],crc.b[1]);
	DBG3NL;
	return(crc.w);
}

// calculate crc on incoming packet
uint16_t MR24FDB1::calculateCrc16(){
	return calculateCrc16((uint8_t *)&Packet, Packet.length-1);
}

// calculate crc16 on a buffer;
uint16_t MR24FDB1::calculateCrc16(uint8_t *frameP, uint16_t len){
	uint8_t crcHi = 0xFF;
	uint8_t crcLo = 0xFF;
	int index=0;
	while(len--){
		index = crcLo ^ *frameP++;
		crcLo = ( crcHi ^ crcHiTab[index]);
		crcHi = crcLoTab[index];
	}
	return (uint16_t)(crcLo << 8 | crcHi);
}

void MR24FDB1::dumpPacket(int length) {
	dbgHexDump((uint8_t *) &Packet,(length==0)?Packet.length:length+1);
}
MR24FDB1::Result_t MR24FDB1::testSend() {

#define GREEN_LED 25
#define YELLOW_LED 26
#define RED_LED 27

	Result_t result;
	int count=0;
	int count2=0;
	/*
	pinMode(GREEN_LED,OUTPUT);
	pinMode(YELLOW_LED,OUTPUT);
	pinMode(RED_LED,OUTPUT);
	*/

	
	INFO("testSend\n");
	while(true) {
		printf("[%d] getting Presence:\n",count++);
		getPresence(1);
		readPacket();
		parsePacket();
		getMovement(1);
		readPacket();
		parsePacket();
	}
}
