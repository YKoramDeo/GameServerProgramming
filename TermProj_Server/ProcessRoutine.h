#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "IOCP.h"

void ProcessPacket(const int, unsigned char*);

namespace ProcessReceivePacket
{
	void CS_MOVE_PACKET(int, BYTE);
}