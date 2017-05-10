#include "ProcessRoutine.h"
#include "protocol.h"

void ProcessPacket(const int key_id, unsigned char *packet)
{
	unsigned char packet_type = packet[1];
	switch (packet_type)
	{

	case PacketType::CS_MOVE:
	{
		CS_MOVE_PACKET *received_data = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		std::string text = std::to_string(key_id) + " client send move packet : ";
		switch (received_data->dir)
		{
		case Const::MoveDirection::None: text += "None";	break;
		case Const::MoveDirection::Left: text += "Left";	break;
		case Const::MoveDirection::Right:text += "Right";	break;
		case Const::MoveDirection::Down: text += "Down";	break;
		case Const::MoveDirection::Up:	 text += "Up";		break;
		default: text += "None";	break;
		}
		DisplayDebugText(text);
		ProcessReceivePacket::CS_MOVE_PACKET(key_id, received_data->dir);
		break;
	}
	default:
		DisplayDebugText("Unknown Packet Type Detected...");
		return;
	}
	return;
}

void ProcessReceivePacket::CS_MOVE_PACKET(int key_id, BYTE dir)
{
	int x = gClientsList[key_id].player.pos.x;
	int y = gClientsList[key_id].player.pos.y;
	switch (dir)
	{
	case Const::MoveDirection::Left:	
		x--; 
		if (x <= 0) 
			x = 0;	
		break;
	case Const::MoveDirection::Right:	
		x++; 
		if (x >= BOARD_COLUMN) 
			x = BOARD_COLUMN - 1; 
		break;
	case Const::MoveDirection::Up:
		y--;
		if (y <= 0)
			y = 0;
		break;
	case Const::MoveDirection::Down:
		y++;
		if (y >= BOARD_LINE)
			y = BOARD_LINE - 1;
		break;
	}

	SC_POSITION_INFO_PACKET packet;
	packet.size = sizeof(SC_POSITION_INFO_PACKET);
	packet.type = PacketType::SC_POSITION_INFO;
	packet.id = key_id;
	packet.x_pos = x;
	packet.y_pos = y;

	SendPacket(key_id, reinterpret_cast<UCHAR*>(&packet));
	gClientsList[key_id].player.pos.x = x;
	gClientsList[key_id].player.pos.y = y;

	std::string text = std::to_string(key_id) + " Pos : " + std::to_string(x) + ", " + std::to_string(y);
	DisplayDebugText(text);
	return;
}