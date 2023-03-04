#include "mr24fdb1.h"
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

const char * MR24FDB1_Exception::ResStr[] = {
/* 0*/		"Ok",
/* 1*/		"Init in Progess",
/* 2*/		"Presence",
/* 3*/		"Movement Strength",
/* 4*/		"Location or Direction",
/* 5*/		"HeartBeat",
/* 6*/		"Abnormal Reset Occured",
/* 7*/		"Fall Report",
/* 8*/		"Upgrade In Progress",
/* 9*/		"Upgrade Success",
/*10*/		"Init Success",
/*11*/		"Undefined",
/*12*/		"Device Id Responce",
/*13*/		"Software Version Responce",
/*14*/		"Hardware Version Responce",
/*15*/		"Protocol Version Responce",
/*16*/		"Presence Responce",
/*17*/		"Movement Responce",
/*18*/		"Sensitivity Responce",
/*19*/		"Scene Responce",
/*20*/		"Unoccupied Time Responce",
/*21*/		"Upgrade Start Ok Responce",
/*22*/		"Upgrade Packet Ack Responce",
/*23*/		"FallSensitivity Responce",
/*24*/		"FallEnable Responce",
/*25*/		"FallWarnTime Responce",
};

const char * MR24FDB1_Exception::ErrStr[] = {
			"General Error",			// obs!, Result_t has this at 100
			"Unimplemented",
			"Timeout",
			"Packet To Long",
			"Unkown Pkt Type",
			"Unexpected GET Packet",
			"Unexpected SET Packet",
			"Invalid Crc",
			"NoPacket Received",
			"Invalid Rsp",
			"Invalid Responce Group",
			"Invalid Responce Func",
			"Invalid Report",
			"Invalid Report Group",
			"Invalid Report Func",
			"Invalid Identification Func",
			"Invalid Radar Func",
			"Invalid Location",
			"Invalid Presence",
			"Invalid Other Func",
			"Invalid Heart Beat",
			"Invalid Reset",
			"Invalid Scene Id"
			"Invalid Unoccupied Time",
			"Invalid Fall Group",
			"Invalid Fall Alarm Func",
			"Invalid Fall Alarm",
			"Invalid Fall Alarm Warning Count",
			"Invalid Fall Alarm Warning Time",
			"Upgrade Start Failed",
			"Upgrade Failed"
};

#define strLen(strArr) (sizeof(strArr)/sizeof(char *))

void MR24FDB1_Exception::decodeError() {

	char * sp;
	printf("Error decode:\n");

	if (Result <100 ) {
		printf("Cause (R).: %d => %s\n",Result, Result < strLen(ResStr)
			? ResStr[Result]
			: "Errcode out of bounds cant print error string, you need to update errDecode.c"
		);
	} else {
		printf("Cause (E).: %d => %s\n",Result, Result-MR24FDB1::Res_Error < strLen(ErrStr)	// Res_Error == 100
			? ErrStr[Result-MR24FDB1::Res_Error]
			: "Errcode out of bounds cant print error string, you need to update errDecode.c"
		);
			
	}

	printf("Class ....: 0x%2.2x => %s \n",FClass, FClass < MR24FDB1::ClassStrCount
		? MR24FDB1::ClassStr[FClass]
		: "func code out of bounds cant print code"
	);
	printf("Group ....: 0x%2.2x => %s\n",FGroup, FGroup < MR24FDB1::GroupStrCount
		? MR24FDB1::GroupStr[FGroup]
		: "class code out of bounds cant print class"
	);
	printf("Func .....: 0x%2.2x => %s\n",Func, Func < MR24FDB1::FuncStrCount
		? MR24FDB1::FuncStr[Func]
		: "func out of bounds cant print func"
	);
	printf("Val ......: 0x%2.2x 0%2.2x 0%2.2x 0%2.2x \n",Valb[0],Valb[1],Valb[2],Valb[3]);
}
