/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General License for more details.
 *
 *  You should have received a copy of the GNU General License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  (C) Copyright Kevin Timmerman 2007
 */

#include "harmony.h"
#include "hid.h"
#include "remote.h"

static const uint8_t TYPE_REQUEST = 0;
static const uint8_t TYPE_RESPONSE = 1;

static const uint8_t COMMAND_INVALID = 0x00;
static const uint8_t COMMAND_EXECUTE_ACTION = 0x01;
static const uint8_t COMMAND_EXECUTE_REPEATED_ACTION = 0x02;
static const uint8_t COMMAND_CONTINUE_REPEATED_ACTION = 0x03;
static const uint8_t COMMAND_FINISH_REPEATED_ACTION = 0x04;
static const uint8_t COMMAND_INITIATE_DIAGNOSTIC_TCP_CHANNEL = 0x10;
static const uint8_t COMMAND_UDP_ECHO = 0x11;
static const uint8_t COMMAND_UDP_PING = 0x12;
static const uint8_t COMMAND_TCP_ECHO = 0x13;
static const uint8_t COMMAND_TCP_PING = 0x14;
static const uint8_t COMMAND_READ_MEMORY_HEADER = 0x15;
static const uint8_t COMMAND_READ_MEMORY_DATA = 0x16;
static const uint8_t COMMAND_READ_MEMORY_DONE = 0x17;
static const uint8_t COMMAND_WRITE_MEMORY_HEADER = 0x18;
static const uint8_t COMMAND_WRITE_MEMORY_DATA = 0x19;
static const uint8_t COMMAND_WRITE_MEMORY_DONE = 0x1A;
static const uint8_t COMMAND_RESET = 0x1B;
static const uint8_t COMMAND_CALCULATE_CHECKSUM = 0x1C;
static const uint8_t COMMAND_INITIATE_UPDATE_TCP_CHANNEL = 0x40;
static const uint8_t COMMAND_START_UDPATE = 0x41;
static const uint8_t COMMAND_WRITE_UDPATE_HEADER = 0x42;
static const uint8_t COMMAND_WRITE_UPDATE_DATA = 0x43;
static const uint8_t COMMAND_WRITE_UPDATE_DONE = 0x44;
static const uint8_t COMMAND_GET_UDPATE_CHECKSUM = 0x45;
static const uint8_t COMMAND_FINISH_UPDATE = 0x46;
static const uint8_t COMMAND_READ_REGION = 0x47;
static const uint8_t COMMAND_READ_REGION_DATA = 0x48;
static const uint8_t COMMAND_READ_REGION_DONE = 0x49;
static const uint8_t COMMAND_START_RAW_IR_TCP_CHANNEL = 0x50;
static const uint8_t COMMAND_CACHE_RAW_IR_HEADER = 0x51;
static const uint8_t COMMAND_CACHE_RAW_IR_DATA = 0x52;
static const uint8_t COMMAND_CACHE_RAW_IR_DONE = 0x53;
static const uint8_t COMMAND_EXECUTE_RAW_IR = 0x54;
static const uint8_t COMMAND_START_RAW_IR = 0x55;
static const uint8_t COMMAND_CONTINUE_RAW_IR = 0x56;
static const uint8_t COMMAND_FINISH_RAW_IR = 0x57;
static const uint8_t COMMAND_INITIATE_SYSTEM_TCP_CHANNEL = 0x60;
static const uint8_t COMMAND_GET_SYSTEM_INFO = 0x61;
static const uint8_t COMMAND_GET_INTERFACE_LIST = 0x62;
static const uint8_t COMMAND_IS_INTERFACE_SUPPORTED = 0x65;
static const uint8_t COMMAND_GET_GUID = 0x67;
static const uint8_t COMMAND_SET_GUID = 0x68;
static const uint8_t COMMAND_GET_NAME = 0x6A;
static const uint8_t COMMAND_SET_NAME = 0x6B;
static const uint8_t COMMAND_GET_LOCATION = 0x6C;
static const uint8_t COMMAND_SET_LOCATION = 0x6D;
static const uint8_t COMMAND_GET_CURRENT_TIME = 0x70;
static const uint8_t COMMAND_UPDATE_TIME = 0x71;
static const uint8_t COMMAND_INITIATE_ZWAVE_TCP_CHANNEL = 0x80;
static const uint8_t COMMAND_SEND_LONG_ZWAVE_REQUEST_HEADER = 0x81;
static const uint8_t COMMAND_SEND_LONG_ZWAVE_REQUEST_DATA = 0x82;
static const uint8_t COMMAND_SEND_LONG_ZWAVE_REQUEST_DATA_DONE = 0x83;
static const uint8_t COMMAND_SEND_SHORT_ZWAVE_REQUEST = 0x84;
static const uint8_t COMMAND_SEND_SHORT_ZWAVE_RESPONSE = 0x85;
static const uint8_t COMMAND_SET_NODE_ID = 0x86;
static const uint8_t COMMAND_GET_NODE_ID = 0x87;
static const uint8_t COMMAND_GET_HOME_ID = 0x88;
static const uint8_t COMMAND_SET_HOME_ID = 0x89;
static const uint8_t COMMAND_SEND_LONG_ZWAVE_RESPONSE_HEADER = 0x8A;
static const uint8_t COMMAND_SEND_LONG_ZWAVE_RESPONSE_DATA = 0x8B;
static const uint8_t COMMAND_SEND_LONG_ZWAVE_RESPONSE_DATA_DONE = 0x8C;
static const uint8_t COMMAND_INITIATE_LEARNIR_TCP_CHANNEL = 0xA0;
static const uint8_t COMMAND_LEARNIR = 0xA1;
static const uint8_t COMMAND_LEARNIR_HEADER = 0xA2;
static const uint8_t COMMAND_LEARNIR_DATA = 0xA3;
static const uint8_t COMMAND_LEARNIR_DONE = 0xA4;
static const uint8_t COMMAND_LEARNIR_STOP = 0xA5;

static const uint8_t STATUS_NULL = 0x00;
static const uint8_t STATUS_OK = 0x01;
static const uint8_t STATUS_BUSY = 0x02;
static const uint8_t STATUS_BAD_VERSION = 0x03;
static const uint8_t STATUS_UNKNOWN_HANDLE = 0x04;
static const uint8_t STATUS_UNKNOWN_ACTION = 0x05;
static const uint8_t STATUS_ALREADY_ABORTED = 0x06;
static const uint8_t STATUS_NO_MORE_DATA = 0x07;
static const uint8_t STATUS_INVALID_ADDRESS = 0x08;
static const uint8_t STATUS_INVALID_TCP_COMMAND = 0x09;
static const uint8_t STATUS_BAD_DATA_LENGTH = 0x0A;
static const uint8_t STATUS_BAD_REGION = 0x0B;
static const uint8_t STATUS_INVALID_ARGUMENT = 0x0C;
static const uint8_t STATUS_DEVICE_NOT_READY = 0x0D;
static const uint8_t STATUS_INVALID_RESPONSE = 0x0E;



int CRemoteZ_HID::UDP_Write(uint8_t typ, uint8_t cmd, unsigned int len/*=0*/, uint8_t *data/*=NULL*/)
{
	if(len>60) return 1;
	uint8_t pkt[68];
	pkt[0]=0;
	pkt[1]=3+len;
	pkt[2]=1; // UDP
	pkt[3]=typ;
	pkt[4]=cmd;
	if(data && len) memcpy(pkt+5,data,len);

	return HID_WriteReport(pkt);
}

int CRemoteZ_HID::UDP_Read(uint8_t &status, unsigned int &len, uint8_t *data)
{
	uint8_t pkt[68];
	int err;
	if ((err=HID_ReadReport(pkt))) return err;
	if(pkt[1]<4) return 1;
	status=pkt[5];
	len=pkt[1]-4;
	if(!len) return 0;
	memcpy(data,pkt+6,len);
	return 0;
}

int CRemoteZ_HID::TCP_Write(uint8_t typ, uint8_t cmd, unsigned int len/*=0*/, uint8_t *data/*=NULL*/)
{
	return 0;
}
int CRemoteZ_HID::TCP_Read(uint8_t &status, unsigned int &len, uint8_t *data)
{
	return 0;
}


int CRemoteZ_TCP::UDP_Write(uint8_t typ, uint8_t cmd, unsigned int len/*=0*/, uint8_t *data/*=NULL*/)
{
	return 0;
}

int CRemoteZ_TCP::UDP_Read(uint8_t &status, unsigned int &len, uint8_t *data)
{
	return 0;
}

int CRemoteZ_TCP::TCP_Write(uint8_t typ, uint8_t cmd, unsigned int len/*=0*/, uint8_t *data/*=NULL*/)
{
	return 0;
}
int CRemoteZ_TCP::TCP_Read(uint8_t &status, unsigned int &len, uint8_t *data)
{
	return 0;
}


int CRemoteZ_Base::Reset(uint8_t kind)
{
	return 0;
}

int CRemoteZ_Base::GetIdentity(TRemoteInfo &ri)
{
	UDP_Write(TYPE_REQUEST, COMMAND_GET_SYSTEM_INFO);
	uint8_t rsp[60];
	unsigned int len;
	uint8_t status;
	UDP_Read(status,len,rsp);

	const uint16_t usb_vid = rsp[1]<<8 | rsp[0];
	const uint16_t usb_pid = rsp[3]<<8 | rsp[2];
	ri.architecture = rsp[5]<<8 | rsp[4];
	ri.fw_ver_major = rsp[7]<<8 | rsp[6];
	ri.fw_ver_minor = rsp[9]<<8 | rsp[8];
	ri.fw_type = rsp[10];
	ri.skin = rsp[12]<<8 | rsp[11];
	const unsigned int hw = rsp[14]<<8 | rsp[13];
	ri.hw_ver_major = hw>>4;
	ri.hw_ver_minor = hw&0x0F;
	ri.flash_mfg=0x01;	/// todo
	ri.flash_id=0x49;	/// todo

	setup_ri_pointers(ri);
	
	UDP_Write(TYPE_REQUEST, COMMAND_GET_GUID);
	UDP_Read(status,len,rsp);

	make_serial(rsp,ri);

	ri.config_bytes_used=0;
	ri.max_config_size=1;

	return 0;
}

int CRemoteZ_Base::ReadFlash(uint32_t addr, const uint32_t len, uint8_t *rd, unsigned int protocol, bool verify/*=false*/)
{
	return 0;
}

int CRemoteZ_Base::InvalidateFlash(void)
{
	return 0;
}

int CRemoteZ_Base::EraseFlash(uint32_t addr, uint32_t len, const TRemoteInfo &ri)
{
	return 0;
}

int CRemoteZ_Base::WriteFlash(uint32_t addr, const uint32_t len, const uint8_t *wr, unsigned int protocol)
{
	return 0;
}

int CRemoteZ_Base::GetTime(const TRemoteInfo &ri, THarmonyTime &ht)
{
	UDP_Write(TYPE_REQUEST, COMMAND_GET_CURRENT_TIME);
	uint8_t time[60];
	unsigned int len;
	uint8_t status;
	UDP_Read(status,len,time);
	//printf("len: %i\n",len);

	ht.year=time[1]<<8 | time[0];
	ht.month=time[2];
	ht.day=time[3];
	ht.hour=time[4];
	ht.minute=time[5];
	ht.second=time[6];
	ht.dow=time[7]&7;
	ht.utc_offset=static_cast<int16_t>(time[9]<<8 | time[8]);
	if(len>16)
		ht.timezone=reinterpret_cast<char*>(time+16);
	else
		ht.timezone="";

	return 0;
}

int CRemoteZ_Base::SetTime(const TRemoteInfo &ri, const THarmonyTime &ht)
{
	return 0;
}

int CRemoteZ_Base::LearnIR(string *learn_string/*=NULL*/)
{
	return 0;
}
