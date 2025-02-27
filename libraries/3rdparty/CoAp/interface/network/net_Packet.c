#line __LINE__ "net_Packet.c"
/*******************************************************************************
 * Copyright (c)  2015  Dipl.-Ing. Tobias Rohde, http://www.lobaro.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *******************************************************************************/
#include "../../coap.h"

void _rom PrintRawPacket(NetPacket_t* pckt)
{

	IotLogInfo("Packet size:%d rssi:%d hops: %d \r\nRawData (hex) :", pckt->size, pckt->metaInfo.Dat.RfPath.RSSI, pckt->metaInfo.Dat.RfPath.HopCount);
	int i;
	for(i=0; i<pckt->size; i++)
	{
		IotLogInfo("0x%02x, ", pckt->pData[i]);
	}
	IotLogInfo("\r\nRawData (char):");
	for(i=0; i<pckt->size; i++)
	{
		IotLogInfo("%c", pckt->pData[i]);
	}
}
