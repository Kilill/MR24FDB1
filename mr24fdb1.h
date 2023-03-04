#ifndef _RC_RADAR_H__
#define _RC_RADAR_H__
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



+----------------------------------------------------------------------------------------+
|                                  Frame structure                                       │
+----------------+---------+----------------+------------+-----------+---------+---------+
|   Start Flag   | Length  | Function Class | Func Group | FUnction  | Data    | Crc     |
+----------------+---------+----------------+------------+-----------+---------+---------+
|      -         |  0  1   |       2        |     3      |     4     | 5->5+n  | Len - 2 |
+----------------+---------+----------------+------------+-----------+---------+---------+
| 1 Byte (0x55 ) | 2 Bytes |   1 Byte       |   1 Byte   |  1 Byte   | n Bytes | 2 Bytes |
+----------------+---------+----------------+------------+-----------+---------+---------+

Starting code:	1 Byte = 0x55 not counted in length but counted in crc computation
Length:			2 Bytes low byte first = Lenght (2) + Function class (1) + Func group (1) Function (1) + Number of data bytes (n) + Crc (2) =
				7 + n
Function Class:	1 byte	(Function Code)
Function Group:	1 byte (Address 1) Function clasification
Function:		1 Byte (address 2) Specific function
Data:			n Bytes of data
Crc:			2 Bytes low byte first

crc is calculated over the complete packet including header but ofc exluding crc bytes
be aware that "Length" does include the crc bytes but does not count the header

Both the Get and Set commands will result in a responce sent back

+----------+------+-------------------+------+----------------------+------+----------------+-------------------------------------+
| Function Class  |   Function Group  |           Function          |        Data           |               Notes                 |
| Name     | Code |     Name          | Code |   Name               | Code |                |                                     |
+----------+------+-------------------+------+----------------------+------+----------------+-------------------------------------+
| Get      | 0x01 |                    (Read)  Get information, data reported back in responce packet                             |
+----------+------+-------------------+------+------------------------------------------------------------------------------------+
|                 | Identification    | 0x01 |                                                                                    |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Device id            | 0x01 |                | Report device id                    |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Software             | 0x02 |                | Report software version             |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Hardware             | 0x03 |                | Report hardware version             |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Protocol             | 0x04 |                | Report protocl version              |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | Radar Information | 0x03 | Presence             | 0x05 |                | Report Presence                     |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Movment              | 0x06 |                | Report movenent strength            |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | System Paramter   | 0x04 | Threshold            | 0x0C |                | Report threshold setting            |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Scene                | 0x10 |                | Report scene setting                |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | Fall parameters   | 0x05 | Fall Function        | 0x0B |                | Report fall state of fall functon   |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Fall Warning Time    | 0x0C |                | Rerport fall warning time setting   |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Sensitiviy      | 0x0E |                | Report fall sensitivity setting     |
+----------+------+--------------------------+----------------------+------+----------------+-------------------------------------+
| Set      | 0x02 |                   (Write)  Set parameters  [ Copy Order ]                                                     |
+----------+------+-------------------+------+------------------------------------------------------------------------------------+
|                 | System Paramter   | 0x04 |                                                                                    |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Threshold Gear       | 0x0C | 0x1-0xA        |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Scene                | 0x10 | 0x00           | Default                             |
|                                            | Setting              |      |                |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x01           | Area Detction                       |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x02           | Bathroom                            |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x03           | Bedroom                             |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x04           | Livingroom                          |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x05           | Office                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x06           | Hotell                              |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | UnoccupiedTimeout    | 0x12 | 0x0-0x8        | Set unoccupied timeout              |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x00           | Do not use ?                        |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x01           | 10 Sec                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x02           | 30 Sec                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x03           | 1 Min                               |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x04           | 2 Min                               |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x05           | 5 Min                               |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x06           | 10 Min                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x07           | 30 Min                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x08           | 60 Min                              |
|                 +-------------------+------+-----------------------------+----------------+-------------------------------------+
|                 | Other Functions   | 0x05 |                                                                                    |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Reboot               | 0x04 |                |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Start Upgrade        | 0x08 | 4 Bytes 		| Firmware size                       |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | n Bytes        | Firmware Version                    |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Upgrad Packet        | 0x09 | 4 Bytes 		| Packet Offset                       |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 1024 Bytes     | Packet Data                         |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | End Of Upgrade       | 0x0A | 0x0F           | Signal end of Upgrade               |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Function        | 0x0B | 0x00           | Off                                 |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x01           | On                                  |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Warning Time    | 0x0C | 0x00 - 0x06    | 1 min - 7 min                       |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x07           | 10 min                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x08           | 15 min                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x09           | 30 min                              |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Sensitivity     | 0x0E | 0x01 - 0xA     | Defaut 0x04 , Higer more sensitive  |
+----------+------+--------------------------+----------------------+------+----------------+-------------------------------------+
| Responce | 0x03 |                   (Passive reporting)   Responces to Get and Set                                              |
+----------+------+-------------------+------+------------------------------------------------------------------------------------+
|                 | Identification    | 0x01 |                                                                                    |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Device ID            | 0x01 | 12 Bytes       |                                     |
|                                            |----------------------+------+----------------+-------------------------------------+
|                                            | Software             | 0x02 | 10 Bytes       |                                     |
|                                            | Version              |      |                |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Hardware             | 0x03 | 8 Bytes        |                                     |
|                                            | Version              |      |                |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Protocol             | 0x04 | 8 Bytes        |                                     |
|                                            | Version              |      |                |                                     |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | Radar info        | 0x03 |                                                                                    |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Presence             | 0x05 | 00 FF FF       | Unoccupied                          |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 01 00 FF       | Stationary                          |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 01 01 01       | Movement                            |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Movement Value       | 0x06 | 4 Bytes Float  |                                     |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | System Info       | 0x04 |                                                                                    |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | Sensitivity          | 0x0C | 0x01-0x0A      | see Set command above               |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Scene Setting        | 0x10 | 0x01-0x05      | see Set command above               |
|                                            +----------------------+------+----------------+-------------------------------------+
|   V1.7 Doc                                 | Unoccupied Timeout   | 0x12 | 0x0-0x8        | Unoccupied timeout value            |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x00           | Do not use ?                        |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x01           | 10 Sec                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x02           | 30 Sec                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x03           | 1 Min                               |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x04           | 2 Min                               |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x05           | 5 Min                               |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x06           | 10 Min                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x07           | 30 Min                              |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x08           | 60 Min                              |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | Other Info        | 0x05 |                                                                                    |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                            | OTA Upgrade Start ACK| 0x08 | 0x00           | Fail                                |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x01           | Success                             |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Feedback OTA         | 0x09 | 0x0F           | ACK                                 |
|                                            | Transmission         |      |                |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Function Switch | 0x0B | 0x00           | off                                 |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x01           | On                                  |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Warning Time    | 0x0C | 0x00 - 0x09    |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Sensistivity    | 0x0E | 0x01 - 0x0a    |                                     |
+----------+------+--------------------------+----------------------+------+----------------+-------------------------------------+
| Reports  | 0x04 |                          (Active Reports)                                                                     |
+----------+------+-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | Identification    | 0x01 | Software Version     | 0x02 |                | Software upgrade complete           |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | Radar             | 0x03 | Presence (Enviroment)| 0x05 | 00 FF FF       | None                                |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                                                          | 01 00 FF       | Presence                            |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 01 01 01       | Movement                            |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Movment Strength     | 0x06 | 4 bytes Float  | Amount of movment detected 0-100    |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Location / Direction | 0x07 |                | Location / Direction of presence    |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                                                          | 01 01 01       | Nothing detected                    |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 01 01 02       | Close                               |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 01 01 03       | Far                                 |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 01 01 04       | Aproaching                          |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 01 01 05       | Leaving                             |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                 | Other Info        | 0x05 | Heartbeat            | 0x01 | 00 FF FF       | Unoccupied                          |
|                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                                                          | 01 00 FF       | Stationary                          |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 01 01 01       | Movement                            |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Abnormal Reset       | 0x02 | 0x0F           |                                     |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Init successfull     | 0x0A | 0x0F           |                                     |
+----------+------+--------------------------+----------------------+------+----------------+-------------------------------------+
| Fall     | 0x06 |                          Fall detection reports                                                               |
+----------+------+--------------------------+----------------------+------+----------------+-------------------------------------+
|                 | Alarm             | 0x01 | Fall Alarm           | 0x01 | 0x00           | Suspected Fall                      |
+                 +-------------------+------+----------------------+------+----------------+-------------------------------------+
|                                                                          | 0x01           | Real Fall                           |
|                                                                          +----------------+-------------------------------------+
|                                                                          | 0x02           | No Fall                             |
|                                            +----------------------+------+----------------+-------------------------------------+
|                                            | Fall Warning         | 0x02 | 0x00 - 0x04    | Fall Warning count, 0 to 4          |
+--------------------------------------------+----------------------+------+----------------+-------------------------------------+
*/

#include <stdint.h>
#include <string>
using namespace std;

class MR24FDB_Exception ;
/** MR24FDB1 class
 * 
 * Class to handle comunicating with the mr24fdb1 radar module
 */
class MR24FDB1 {
	friend class MR24FDB_Exception; 
public :
	static const int DEVICE_ID_LEN=12;				///< Length of device id data
	static const int HARDWARE_VER_LEN=8;			///< Length of hardware version data
	static const int SOFTWARE_VER_LEN=8;			///< Lenght of software version data
	static const int PROTOCOL_VER_LEN=8;			///< Length of protocol version data
	static const int MAXPACKET_DATA_LENGTH = 50;	///< max data length

	/** packet struct / uinion
	 *
	 * Packet structure, from after func the union is used for convinience and
	 * type conversions
	 * not included ere is the 2 byte crc at the end
	 */
	#pragma pack(1)
	typedef	struct {
		uint8_t			frameStart;						///< included in the struct since it's used in crc calculation
		uint16_t		length;							///< Length of packet, does not count frameStart byte
		uint8_t			funcClass;						///< Function Class
		uint8_t			funcGroup;						///< Function Group
		uint8_t			func;							///< Function

		union {											///< data 1-n bytes data, convinience definitions
			uint8_t		scene;							///< scene valu
			uint8_t		unoccupiedTime;					///< Unoccupied time
			uint8_t		sensitivity;					///< Sensitivity value
			uint8_t		fallEnable;						///< fall enable swithc valie
			uint8_t		fallWarningTime;				///< fall warning time
			uint8_t		fallSensitivity;				///< fall sensittuvity setting
			struct {									///< firmware  2 values
				uint16_t size;							///< size of firmware
				uint16_t version;						///< n bytes of version so version here 
			};
			uint32_t	packetOffset;					///< Firmware
			uint8_t 	deviceID[DEVICE_ID_LEN+1];		///< Device id
			uint8_t		softwareVer[SOFTWARE_VER_LEN+1];///< Software version
			uint8_t		hardwareVer[HARDWARE_VER_LEN+1];///< Hardware version
			uint8_t		protocolVer[PROTOCOL_VER_LEN+1];///< Protocol version
			uint8_t		presence[3];					///< Presence patter
			uint8_t		location[3];					///< Location pattern
			uint8_t		heartBeat[3];					///< Heartbear pattern
			float 		movement;						///< movement
			uint8_t		upgradeStartAck;				///< upgrade start ack
			uint8_t		upgradePacketAck;				///< upgrade packet ack
			uint8_t		abnReset;						///< abnormal Reset flag
			uint8_t		fallAlarm;						///< Fall alarm value
			uint8_t		fallWarning;					///< Fall warning value
			uint32_t	lval;							///< uint32_t conversion  
			uint8_t		data[MAXPACKET_DATA_LENGTH];	///< paddout
		};
	} Packet_t;
	#pragma pack()

	const uint8_t MESSAGE_HEAD = 0x55; 						///< Data frame header
	static const unsigned long MAX_SERIAL_WAIT_TIME = 2000;	///< max time in ms to wait for serial char to arrive
	static const int MAXPACKET_LENGTH = sizeof(Packet_t);	///< max length of a packet from the chip

	/** string representations of code/class/func/Scene etc
	 *
	 */
	static const char *ClassStr[];				///< Text representation of function classes
	static const int ClassStrCount;				///< Number of function class strings

	static const char *GroupStr[];				///< Text representation of function groups
	static const int GroupStrCount;				///< Number of function group strings

	static const char *FuncStr[];				///< Text representation of functions
	static const int FuncStrCount;				///< Number of function strings

	static const char *SceneStr[];				///< Text representation of scenes
	static const int SceneStrCount;				///< Number of function strings

	static const char* UnoccupiedTimeStr[];		///< Text represntations of Unoccupied settings
	static const int UnoccupiedTimeStrCount;	///< Number of  Unoccupied settings strings


	/** Class of functions
	 * 
	 * First byte of packet (not counting length) indicates
	 * the class the packet belongs to
	 */
	enum FuncClasses
	{
		GET				= 0x01, ///< Read Operation
		SET				= 0x02, ///< Write Operation
		RESPONCE		= 0x03, ///< Respones from GET and SET commands
		REPORT			= 0x04, ///< Active continnous reports
		FALL			= 0x06,	///< Fall reports
	};

	/** Group of functions
	 * 
	 * They are chared over GET, SET, RESPONCE, FALL  type packets
	 * but FALL has its own groups and functions
	 */
	enum FuncGroups
	{
		IDENT			= 0x01, ///< Identifications and versions
		UNDEF_CLASS		= 0x02, ///< Undef group
		RADAR			= 0x03, ///< Radar / Senor
		SYSTEM 			= 0x04, ///< Sytemt related
		OTHER			= 0x05,	///< Other functions
		FALLC			= 0x05,	///< Fall group used mainly in get...
	};
	
	/** Function codes
	 * 
	 * third byte indicates specific function
	 */
	enum Functions {
		DEVICE				= 0x01,
		HEARTBEAT			= 0x01,
		SOFTWARE			= 0x02,
		HARDWARE			= 0x03,
		PROTOCOL			= 0x04,
		REBOOT				= 0x04,
		PRESENCE			= 0x05,	
		MOVEMENT			= 0x06,
		LOCATION			= 0x07,
		UPGRADE				= 0x08,
		UPGRADE_PKT			= 0x09,
		INIT_SUCCESS		= 0x0A,
		END_OF_UPGRADE		= 0x0A,
		FALL_ENABLE			= 0x0B, ///< Fall get/responce enable switch
		SENSITIVITY			= 0x0C, ///< Movement sensitivity
		FALL_WARN_TIME		= 0x0C,
		UNKONWN_FUNC_D		= 0x0D, 		
		FALL_SENSITIVITY	= 0x0E, ///< Fall get/set sensititvity
		UNKONWN_FUNC_F		= 0x0F, 		
		SCENE				= 0x10,	
		UNKONWN_FUNC_11		= 0x11, 		
		UNOCCUPIED_TIME		= 0x12,	
	};
	
	/* value constants */
	
	/** Presence states
	 * 
	 */
	enum Presence_t
	{
		P_NOBODY			= 0x00,		///< No presence detected 
		P_PRESENT			= 0x01,		///< Somebody Present and stationary
		P_MOVING			= 0x02,		///< Somebody present and moving
	};
	
	/** Presence bitmap patterns
	 * 
	 */
	#define P_NOBODY_BP		0XFFFF00	///< No presence
	#define P_PRESENT_BP	0xFF0001	///< Presence
	#define P_MOVING_BP		0x010101	///< Presence and moving
	
	
	/** Location / Direction states
	 * 
	 */
	enum Location_t
	{
		L_STATIONARY		= 0x01,	  	///< No one or stationary
		L_CLOSE				= 0x02,		///< Something close
		L_AWAY				= 0x03, 	///< Something far
		L_APROACHING		= 0x04,		///< Something aproaching
		L_LEAVING			= 0x05		///< Something leaving
	};
	
	/** Location / Direction patterns bitmaps
	 * 
	*/
	#define L_STATIONARY_BP	0x010101
	#define L_CLOSE_BP		0x020101
	#define L_AWAY_BP		0x030101
	#define L_APROACHING_BP	0x040101
	#define L_LEAVING_BP	0x050101

	/** Heartbeat states
	 * 
	 */ 
	enum HeartBeat_t	
	{
		HB_NONE				= 0x00,		///< No presence detected
		HB_STATIONARY		= 0x01,		///< Presence detecte and stationary
		HB_MOVEMENT			= 0x02,		///< Presence detected and moving
	};
	
	/** Heart Beat pattern bitmaps
	 * 
	 */
	#define HB_NONE_BP			0xFFFF00
	#define HB_STATIONARY_BP	0xFF0001
	#define HB_MOVEMENT_BP		0x010101
	
	/** Scene settings
	 * 
	 */
	enum Scene_t {
		DEFAULT_SC 				= 0x00, 	///< Default setting at power on
		AREA_DETECT_SC			= 0x01,		///< General Area detect
		BATHROOM_SC				= 0x02,		///< Bathroom top mounted
		BEDROOM_SC				= 0x03,		///< Bedroom top mounted
		LIVING_ROOM_SC			= 0x04,		///< Living room top mounted
		OFFICE_SC				= 0x05,		///< Office top mounted
		HOTEL_SC				= 0x06,		///< Hotell top mounted (room?? foayer ?? restroom ?? hallway ??)
	};
	
	/** Unoccupied timeout values
	 *
	 */
	enum UnoccupiedTime_t {
		UT_NONE 				= 0x00, 	///< "Do not use forced entry unnoccupied function"
		UT_10S					= 0x01,		///< 10 Sec
		UT_30S					= 0x02,		///< 30 Sec
		UT_1M					= 0x03,		///< 1 Minute
		UT_2M					= 0x04,		///< 2 Minutes
		UT_5M					= 0x05,		///< 5 Minutes
		UT_10M					= 0x06,		///< 10 Minutes
		UT_30M					= 0x07,		///< 30 Minutes
		UT_60M					= 0x08,		///< 60 Minutes
	};

	// --------- Fall Radar -------
	
	/** Fall Radar report groups
	 * 
	 * Fall alarm as all of its own groups
	 */

	enum FallGroups {
		F_ALARM_CL				= 0x01		// Fallradar actually only has one group , but for consistensy...
	};

	/** Fall Function Codes in fall reports
	*/
	enum FallFunctions			///< Alarm has all its own groups
	{
		F_ALARM					= 0x01,	///< fall info
		F_WARNING				= 0x02,	///< alarm time points 
	};

	/** Fall Alarm types
	 */
	enum FallAlarm_t
	{
		F_SUSPECTED				= 0x00,		///< Only suspected fall
		F_REAL					= 0x01,		///< An actuall fall
		F_NO_FALL				= 0x02,		///< No fall detected
		NO_FALL_DATA			= 0xFF,		///< No Fall data aquired
	};

	/** Fall Warning Types
	 */
	enum FallWarnCount_t {
		NO_FALL_WARNING			= 0x00,		///< No warning issued yet
		FALL_WARNING_1			= 0x01,		///< First  Warning at 5 min
		FALL_WARNING_2			= 0x02,		///< Second Warning at 10 min
		FALL_WARNING_3			= 0x03,		///< Third  Warning at 30 min
		FALL_WARNING_4			= 0x04,		///< Fourth Warning at 60 min
	};
	
	/** Fall Warning time setting
	 */
	enum FallWarnTime_t {
		MIN_1					= 0x00,		///< 1 Minute Warn time
		MIN_2					= 0x01,		///< 2 Minutes Warn time
		MIN_3					= 0x02,		///< 3 Minutes Warn time
		MIN_4					= 0x03,		///< 4 Minutes Warn time
		MIN_5					= 0x04,		///< 5 Minutes Warn time
		MIN_6					= 0x05,		///< 6 Minutes Warn time
		MIN_7					= 0x06,		///< 7 Minutes Warn time
		MIN_10					= 0x07,		///< 10 Minutes Warn time
		MIN_15					= 0x08,		///< 15 Minutes Warn time
		MIN_30					= 0x09		///< 30 Minutes Warn time
	} ;
	
	/** General result codes
	 * 
	 * stuff before Res_Error are status returns
	 * after Res_Error are error/fail/fault conditions
	 * 
	 * !!!!!!! If you move / add to this you need to add to ResStr and / or ErrStr
	 * in decodeError.cpp to keep them in sync
	 */
	enum Result_t
	{
/* 0*/	Res_Ok = 0x00,				///< All ok no errors
/* 1*/	Res_Init,					///< In Init process
/* 2*/	Res_Presence,				///< Presence detected
/* 3*/	Res_Movement,				///< Movment strength
/* 4*/	Res_Location,				///< Location report received
/* 5*/	Res_HeartBeat,				///< HeartBeat info
/* 6*/	Res_Reset,					///< A reset has occured
/* 7*/	Res_Fall,					///< Fall type report received
/* 8*/	Res_UpgradeInProgress,		///< Upgrade in progess state, !! upgrades not implemented yet !!
/* 9*/	Res_UpgradeSuccess,			///< Returned on succsesfull upgrade !! upgrades not implemented yet !!
/*10*/  Res_InitSuccess,			///< Returned after successful initialization
/*11*/	Res_Undefined,				///< Undefined status
// responces
/*12*/	Res_DeviceId_Rsp,			///< Device id responce
/*13*/	Res_SoftwareVer_Rsp,		///< Sofware Version responce
/*14*/	Res_HardwareVer_Rsp,		///< Hardware Version responce
/*15*/  Res_ProtocolVer_Rsp,		///< Protocol Version responce
/*16*/	Res_Presence_Rsp,			///< Presence responce
/*17*/	Res_Movement_Rsp,			///< Movement responce
/*18*/	Res_Sensitivity_Rsp,		///< Sensitivty responce
/*19*/	Res_Scene_Rsp,				///< Scene resoonce
/*20*/	Res_UnoccupiedTime_Rsp,		///< Unoccupied timer responce
/*21*/	Res_UpgradeStartOk_Rsp,		///< Upgrade start ack responce
/*22*/	Res_UpgradePacketAck_Rsp,	///< Upgrade packet ack responce
/*23*/	Res_FallSensitivity_Rsp,	///< Fall Sensitivity responce
/*24*/	Res_FallEnable_Rsp,			///< Fall enable switch responce
/*25*/	Res_FallWarnTime_Rsp,		///< Fall Warning time responce

// fault conditions below
/*100*/	Res_Error = 100,
		Res_Unimplmented,		///< Unimplemented function called
		Res_Timeout,			///< Timed out waiting on Serial port
		Res_PacketToLong,		///< Packet lenght longer than spec
		Res_UnknownPktClass,	///< Uknown packet class (Packet Code) not one of Packet_t
		Res_Unexpected_Get,		///< GET packets should not be sent fron the module
		Res_Unexpected_Set,		///< SET packets should not be sent fron the module
		Res_InvCrc,				///< crc calculation does not match packet crc
		Res_NoPacketReceived,	///< No valid packet was received
//
		// - Responce
		Res_InvRsp,				///< Invalid Responce
		Res_InvRspGroup,		///< Invalid Resonce  group was reseviced 
		Res_InvRspFunc,			///< Invalid Responce function

		// - Report 
		Res_InvRep,				///< Invalid Activer report
		Res_InvRepGroup,		///< Invalid Active report group was reseviced 
		Res_InvRepFunc,			///< Invalid Active Report functions

		// -- Identification
		Res_InvIdentFunc,		///< Invalid identfication function recevice

		// -- Radar
		Res_InvRadarFunc,		///< Invalid radar function code was received
		Res_InvLocation,		///< Invalid location indicator received
		Res_InvPresence,		///< Invalid Presence code received
	
		// -- Other
		Res_InvOtherFunc,		///< Invalid Other report func
		Res_InvHeartBeat,		///< Invalid heartbeat code received
		Res_InvReset,			///< Invalid Reset data byt should be 0x0f
		Res_InvScene,			///< Invalid Scene id
		Res_InvUnoccupiedTime,	///< Invalid unoccupied time
		
		// - Fall
		Res_InvFallGroup,		///< Invalid Fall group received (currnetly only 1 group defined)
		Rea_InvFallAlarmFunc,	///< Unvalid Fall Alarm Func received
		Res_InvFallAlarm,		///< Invalid Fall alarm type received
		Res_InvFallWarnCount,	///< Invalid Fall Warning count received
		Res_InvFallWarnTime,	///< Invalid Fall Warning count received

		Res_UpgradeStartFail_Rsp,	///< and upgrade atempt failed
		Res_UpgradeFail,		///< Upgrade failed
};
	
	// -------- Class Variables ------

	uint8_t RXBit;
	uint8_t TXBit;
	

	Packet_t 	Packet;					///< incoming packet from radar module
	bool		PacketReceived = false;		///< a valid packet has been received

	Result_t	Status;					///< Current status running fault, reset....
	long		Status_ts;			    ///< Last time changed

	Result_t	UpgradeStatus;
	long		UpgradeStatus_ts;

	Presence_t	Presence;				///< last active report Envrioment Status
	long		Presence_ts;			///< Presence time stamp

	float		Movement;				///< movment strengtht
	long		Movement_ts;			///< time stamp;

	Location_t	Location;				///< location far, close none
	long		Location_ts;			///< time stamp;
		
	HeartBeat_t	HeartBeat;				///< Heartbeat detection
	long		HeartBeat_ts;			///< time stamp;

	uint8_t		Sensitivity;			///< senstitivity setting 0-0x0A
	long		Sensitivity_ts;			///< time stamp;
	
	Scene_t		Scene;					///< Scene setting
	long		Scene_ts;				///< Time stamp

	UnoccupiedTime_t UnoccupiedTime;	///< Unoccupied time
	long		UnoccupiedTime_ts;		///< Time stamp
	
	bool		FallEnable;
	long		FallEnable_ts;

	FallAlarm_t	FallAlarm;				///< latest fall status
	long		FallAlarm_ts;			///< time stamp;
	
	FallWarnCount_t FallWarning;		///< last fallwarning received
	long		FallWarning_ts;			///< time stamp;

	FallWarnTime_t	FallWarnTime;		///< last fallwarning time received
	long		FallWarnTime_ts;		///< time stamp;

	uint8_t 	FallSensitivity;		///< last fallwarning received
	long		FallSensitivity_ts;		///< time stamp;

	bool		UpgradeStartAck;
	long		UpgradeStartAck_ts;
	
	
	string		DeviceId;		///< device id
	string		SoftwareVer;	///< Software Version
	string		HardwareVer;	///< Software Version
	string		ProtocolVer;	///< Protocl Version


	bool		GotResponce;

	// ------------- Constructor/Destructor -------------------------


		/** Constructor
		 * 
		 * @param rxBit	GPIO pin for rx default 16
		 * @param txBit	GPIO pin for tx default 17
		 */
		MR24FDB1(uint8_t rxBit, uint8_t txBit);
		MR24FDB1() : MR24FDB1(16,17) {};
		
	// -------------------- Public functions -------------------------
																					
	/** initalize radar
	 * 
	 * @return	Res_Ok of successfull
	 */
	Result_t init();

	/** get one byte from serial
	 * 
	 * Wait around for MaxTime ms for a byte to arrive
	 * returns	Res_OK on success
	 * 			Res_Timeout on timeout
	 * 
	 * @param inByte	pointer where to store byte
	 * @return Res_Ok	on sucess
	 */
	Result_t 	readByte(uint8_t *inByte);

	/** get a packet from the chip
	 * 
	 * Read a complete packet from the chip with crc check
	 * 
	 * @return MR24FDB1::Result_t	Res_Ok if successfull.
	 */
	Result_t 	readPacket();

	/** parse packet
	 * 
	 * parse the incommig packet and set internal variables accordingly
	 * Throws and exception with class MR24FDB1_Exception in case of error
	 * 
	 * @return Resuslt_t xyz depending ong result
	 */

	Result_t 	parsePacket();

	/** send a "GET" packet to the module and wait for responce
	 * 
	 * @param fGroup	the function group to request
	 * @param func		the function to request
	 * @param responce
	 */ 

	Result_t sendGetAndWait(FuncGroups fGroup, Functions func,Result_t responce);

	/** send a "GET" packet to the module
	 * 
	 * @param fGroup	the function group to request
	 * @param func		the function to request
	 */ 

	void sendGet(FuncGroups fGroup, Functions func);


	/** wait until packet of the specified type is recieved
	 * 
	 * throws ans exception on error
	 * 
	 * TODO: timeout....
	 * @param waitResult result to wait for
	 */

	Result_t waitFor(Result_t waitResult);

	// ----------- identification

	/** requests Device Id
	 *
	 * if not yet cached
	 * 	sends a request for device id and stores it internally
	 * else return chached value
	 * throws exeption on error
	 *
	 *
	 * @returns		pointer to DeviceId
	 */
	string & getDeviceId();

	/** requests Software verion
	 *
	 * if not yet chached
	 * 	sends a request for software version and stores it internally
	 * else return cached value
	 * throws exeption on error
	 *
	 * @returns		string reference to SoftwareRev
	 */
	string & getSoftwareVer();

	/** requests Hardware version
	 *
	 * if not yet cached
	 * 	sends a request for hardware version and stores it internally
	 * else returns cachec value
	 * throws exeption on error
	 *
	 * @returns		string reference to HardwareRev
	 */
	string & getHardwareVer();

	/** request Protocol Version
	 *
	 * sends a request for protocol version and stores it internally
	 * will also set a time stamp when data is received
	 * throws exeption on error
	 *
	 * @returns		string reference to ProtocolVer
	 */

	string & getProtocolVer();

	
	// ----- get Radar information
	
	/** get presence from module
	 * 
	 * if time since last read >age or age<0
	 * 	Sends request for presence
	 * 	stores result in Presence
	 * 	will also set a time stamp when data is received
	 * else return cached value
	 * throws exeption on error
	 * 
	 * @param age	if stored value older than age request new
	 * @returns		presence
	 */

	Presence_t getPresence(long age=0);

	/** return last time stamp for presence
	 * 
	 * @return		timestmap
	 */
	long getPresenceTs() {return Presence_ts;};

	/** get movement
	 * 
	 * if age of cached value > age fetch new valie
	 * else return cached value
	 * 
	 * Module sends a movement report more or less every 2 sec
	 * 
	 * @param  age	if stored value older than age request new
	 * @returns		Movment value
	 */
	float	getMovement(long age=0);

	/** return last time stamp for movement
	 * 
	 * @return		timestmap
	 */
	long getMovmentTs() {return Movement_ts;};

	/** get location
	 *
	 */

	Location_t getLocation() {return Location;}

	/** return last time stamp for location
	 * 
	 * @return		timestmap
	 */
	long getLocationTs(){return Location_ts;}

	/** get Heartbeat
	 *
	 */
	HeartBeat_t getHeartBeat() {return HeartBeat;}

	/** return last time stamp for heartbeat
	 * 
	 * @return		timestmap
	 */
	long getHeartBeatTs(){return HeartBeat_ts;}

	// ---- get System paramters

	/** get sensitivity parameter
	 * 
	 * if time since last read >age or age<0
	 * 	Sends request for sensitivity
	 *	store result in Sensitivity
	 *	sets time stamp when data is received
	 * else return cached value
	 * throws exeption on error
	 *
	 * @param age	if stored value older than age request new
	 * @returns		Sensitiviy_t sensitivity
	 */
	uint8_t	getSensitivity(long age=0);

	/** return last time stamp for sensitivity
	 * 
	 * @return		timestmap
	 */
	long	getSensitivity_ts(){return Sensitivity_ts;};

	/** get scene parameter
	 * 
	 * if time since last read >age or age<0
	 * 	Sends request for Scene
	 *	store result in Scene
	 * 	set a time stamp when data is received
	 * else return cached value
	 * throws exeption on error
	 *
	 * @param age	if stored value older than age request new
	 * @returns		Sensitiviy_t sensitivity
	 */

	Scene_t	getScene(long age=0);

	/** return last time stamp for scene
	 * 
	 * @return		timestmap
	 */
	long	getScene_ts() {return Scene_ts;};

	/** get unoccupied timeout parameter
	 * 
	 * @return		Unccoupied_t last set time value
	 */
	UnoccupiedTime_t getUnoccupiedTime() {return UnoccupiedTime;};

	/** return last time stamp for unoccupied timer
	 * 
	 * @return		timestmap
	 */
	long	getUnoccupiedTime_ts() {return UnoccupiedTime_ts;};

	
	// ----- Fall parameters

	/** get status of fall enable switch
	 * 
	 * if time since last read >age or age<0
	 *  sends request for FalleEnable value
	 *	store result in FeallEnable
	 *	set a time stamp when data is received
	 * else return cached value
	 * throws exeption on error
	 *
	 * @param age	if stored value older than age request new
	 * @returns		true=enabled, false=disabled
	 */

	bool	getFallEnable(long age);

	/** get status of fall warning time setting
	 * 
	 * if time since last read >age or age<0
	 *  sends request for FallWarnTime value
	 *	store result in FallWarnTime
	 *	 set a time stamp when data is received
	 * else return cached value
	 * throws exeption on error
	 *
	 * @param		age if stored value older than age request new
	 * @returns		FallWarnTime_t FallWarnTime 
	 */

	FallWarnTime_t	getFallWarnTime(long age);


	/** get status of fall sensitivty setting
	 * 
	 * if time since last read >age or age<0
	 *  sends request for FallSensitivity value
	 *	store result in FallSensitivity
	 *	set a time stamp when data is received
	 * else return cached value
	 * throws exeption on error
	 *
	 * @param age		if stored value older than age request new
	 * @return			FallSensitivity FallWarnTime 
	 */

	uint8_t	getFallSensitivity(long age);
	
	// --------------- Set functions -----------

	/** send a "GET" packet to the module
	 * 
	 * Sends a "set" packet
	 * Values are allways one byt with one exception: Upgrade Packet
	 * that is 4 + 1024 Bytes and is not handled here
	 * actually firemware upgrades are not implemented yet.
	 * 
	 * @param group 	The function class to request
	 * @param func		The function to request
	 * @param val		Value to send
	 * @param len		length of value 
	 */ 

	void sendSet(FuncGroups group, Functions func, uint8_t val,uint16_t len);

	/** set sensitivity 
	 * 
	 * @param sensVal	new sensitivity value
	 */

	void setSensitivity(uint8_t sensVal);

	/** set scene 
	 * 
	 * @param newScene	scene to set
	 */
	void setScene(Scene_t newScene);
	
	/** set unoccupied timeut
	 *
	 * @param time value to set 
	 */
	
	void setUnoccupiedTime(UnoccupiedTime_t time);

	/** reboot
	 * 
	 * sends a reboot command
	 */
	
	Result_t reboot();
	
	/** send an upgrade packet
	 * 
	 *  NOT IMPLEMENTED
	 *
	 * @param size		firmware size
	 * @param versionP	pointer to version info
	 * @param verLen	length of version info
	 */
	void startUpgrade(uint32_t size, uint8_t * versionP, uint16_t verLen);

	/** send an upgrade packet
	 *
	 *  NOT IMPLEMENTED
	 *  also, question are packets allways 1024 bytes
	 *  and is offset in bytes or packets
	 *
	 * @param offset 	offset of packet
	 * @param packetP	pointer to packet of 1024 bytes
	 */

	void sendUpgradePacket(uint32_t offset, uint8_t * packetP);

	/** Send end of upgrade packet
	 * 
	 * Signals end of upgrade process to the module
	 * module should respond with a Report/Identification/Software Version ie 0x04 0x01 0x02
	 */ 
	void endOfUpgrade(); 

	/** enable / disable fall detection
	 * 
	 * @param onOff		true=enabled , false=disabled
	 */
	void setFallDetect(bool onOff);
	
	/** set fall warn time parameter
	 * 
	 * @param fallTime
	 */ 				
	void setFallWarnTime(FallWarnTime_t fallTime);
	
	/** set fall detection sensitivity
	 * 
	 * @param sensVal	value from 1 - 10
	 */
	void setFallSensitivity(uint8_t sensVal);

			
	// --------------- utility ----------------
	/** dumpt packet in hex/asci format
	 * 
	 * Dumps content of packet in hexformat and ascii
	 * if length param=0 then use packets own length field
	 * 
	 * @param length	length of packet to dump
	 */
	void dumpPacket(int length=0);
	
	/** send test
	 * 
	 * Continously request presence and movement
	 * System will hang in endless loop if invoked
	 */

	Result_t testSend();

	// -------------- Privare functions ---------------------
private:
	
	/** parse Radar group responces and reports
	 * 
	 * decode RADAR type resports and responces
	 * set class variable and timestampps and return corresponding
	 * Res_..._Rsp on successfull decode
	 * 
	 * @param fClass	the class handler that called (responce or report)
	 * @return			Res_..._Rsp on successful decode;
	 */

	Result_t decodeRadar(FuncClasses fClass);

	/** send a packet to the module
	 *
	 * Module will respond with a responce packet
	 * 
	 * @param txPacket	Packet to transmit
	 * @param len		length of packet to send
	 * @return			Res_ok if transmission successfull
	 */
	void sendPacket(uint8_t *txPacket,int len);

	/** parse reponce type packet
	 * 
	 * @returns			Result_t Res_..._Rsp if valid responce decoded
	 * @throw			exception on fail
	 */
	Result_t 	parseResponce();

	/** parse report type packet
	 * 
	 * @returns			Result_t Res_..._Rsp if valid responce decoded
	 * @throw			exception on fail
	 */
	Result_t	parseReport();

	/** parse fall type packet
	 * 
	 * @returns			Result_t Res_..._Rsp if valid responce decoded
	 * @throw			exception on fail
	 */
	Result_t parseFallReport();

	/** calculate crc16 of current packet
	 * 
	 * @param frameP	pointer to buffer 
	 * @param len		to calculcate over
	 * @return			calculated crc
	 */
	uint16_t 	calculateCrc16(uint8_t *frameP,uint16_t len);

	/** calculate crc16 of current packet
	 * 
	 * @return uint16_t crc	calculated crc
	 */
	uint16_t 	calculateCrc16();


	/** calculate and add crc16 at end of buffer
	 * 
	 * buffer must be large enough
	 * crc low and high byte  is added to frameP[len-2] and frameP[len-1] respectively
	 *
	 * @param frameP	pointer to buffer 
	 * @param len		to calculcate over
	 * @return			crc	calculated crc
	 */
	uint16_t 	addCrc16(uint8_t *frameP, uint16_t len);

		
};

/** Exception class
 * 
 * used for throwing exceptions
 */ 
class MR24FDB1_Exception {
public:

	/** result of an operartion
	 */
	MR24FDB1::Result_t	Result;

	#pragma pack(1)
	union {
		uint32_t CGF; 			///< packed class/group/function
		struct {
			uint8_t FClass;
			uint8_t FGroup;
			uint8_t Func;
		};
	};

	union {
		uint32_t Val;			///< 4 data bytes
		uint8_t Valb[4];
	};

	#pragma pack()

	static const char *ResStr[];
	static const char *ErrStr[];
		
	/** default constructor
	 *
	 */
	MR24FDB1_Exception() {
		CGF=0;
		Val=0;
		Result=MR24FDB1::Res_Ok;
	}

	/** throw constructor packed values version
	 *
	 * used in throw call with packed arguments
	 * 
	 * @param resultArg the result of the operation that failed
	 * @param cgfArg packed Class,Group, Function
	 * @param valArg packed 4 bytes of data
	 */
	MR24FDB1_Exception(MR24FDB1::Result_t resultArg, uint32_t cgfArg=0, uint32_t valArg=0) {
		Result=resultArg;
		CGF=cgfArg;
		Val=valArg;
	}
	
	/** throw constructor with discreete values
	 * 
	 * Used in throw with discrete CGF and Data values
	 * 
	 * @param resultArg	Result of failing operarion
	 * @param classArg	function class of fail
	 * @param groupArg	function group of fail
	 * @param funcArg	function of fail
	 * @param valArg	packed 4 bytes of data
	 */
	MR24FDB1_Exception(MR24FDB1::Result_t resultArg, uint8_t classArg,uint8_t groupArg , uint8_t funcArg, uint32_t valArg=0) {
		Result=resultArg;
		FClass=classArg;
		FGroup=groupArg;
		Func=funcArg;
		Val=valArg;
	}
	
	/** decode error from the Result,class,group , function etc...
	 * 
	 * and print in clear text
	 */
	void decodeError();
};
#endif
