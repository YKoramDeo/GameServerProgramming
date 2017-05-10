#pragma once
#include "stdafx.h"

#define MY_SERVER_PORT		9212

#define NUM_THREADS			6
#define MAX_USER			10

#define MAX_BUFF_SIZE		4000
#define MAX_PACKET_SIZE		255

#define OP_RECV				1
#define OP_SEND				2

#define NIL					-999

namespace Const
{
	enum MoveDirection
	{
		None = -1, Up, Down, Left, Right
	};
}

enum PacketType
{
	// client -> server
	CS_LOGIN,
	CS_LOGOUT,
	CS_MOVE,
	CS_ATTACK,
	CS_CHAT,
	// server -> client
	SC_LOGIN_OK,
	SC_LOGIN_FAIL,
	SC_POSITION_INFO,
	SC_CHAT,
	SC_STAT_CHANGE,
	SC_REMOVE_OBJECT,
	SC_ADD_OBJECT
};

#pragma pack(push,1)
struct Object
{
	int id;
	POINT pos;
};

// pakcet header
struct Packet
{
	BYTE size;
	BYTE type;
};

// ************************************************** client -> server
struct CS_LOGIN_PACKET : public Packet
{
	WCHAR id_str[10];
};

struct CS_MOVE_PACKET : public Packet
{
	BYTE dir;
};

struct CS_ATTACK_PACKET : public Packet {};

struct CS_CHAT_PACKET : public Packet
{
	WCHAR chat_str[100];
};

struct CS_LOGOUT_PACKET : public Packet {};

// ************************************************** server -> clinet

struct SC_LOGIN_OK_PACKET : public Packet
{
	WORD id;
	WORD x_pos;
	WORD y_pos;
	WORD hp;
	WORD level;
	DWORD exp;
};

struct SC_LOGIN_FAIL_PACKET : public Packet { };

struct SC_POSITION_INFO_PACKET : public Packet
{
	WORD id;
	WORD x_pos;
	WORD y_pos;
};

struct SC_CHAT_PACKET : public Packet
{
	WCHAR char_str[100];
};

struct SC_STAT_CHANGE_PACKET : public Packet
{
	WORD hp;
	WORD level;
	DWORD exp;
};

struct SC_REMOVE_OBJECT_PACKET : public Packet
{
	WORD id;
};

struct SC_ADD_OBJECT_PACKET : public Packet
{
	WORD id;
	BYTE type;
};

#pragma pack(pop)