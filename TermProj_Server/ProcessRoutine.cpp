#include "ProcessRoutine.h"

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
		break;
	}
	default:
		DisplayDebugText("Unknown Packet Type Detected...");
		return;
	}
	return;
}