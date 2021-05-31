#include "Message.h"


Message::Message()
	: ACK(false), _data(new uint8_t[MAX_SIZE]), Data(_data + LWS_PRE), Ptr(Data)
{
	Clear();
}

Message::Message(Message&& other)
	: ACK(std::move(other.ACK)), _data(std::move(other._data)), Data(std::move(other.Data)), Ptr(std::move(other.Ptr))
{
	other.ACK = false;
	other._data = nullptr;
	other.Data = nullptr;
	other.Ptr = nullptr;
}


Message& Message::operator =(Message&& other)
{
	if (this != &other)
	{
		ACK = std::move(other.ACK);
		_data = std::move(other._data);
		Data = std::move(other.Data);
		Ptr = std::move(other.Ptr);

		other.ACK = false;
		other._data = nullptr;
		other.Data = nullptr;
		other.Ptr = nullptr;
	}
	return *this;
}


Message::~Message()
{
	delete[] _data;
}

bool Message::IsEmpty()
{
	return Data == Ptr;
}

void Message::Acknowledge()
{
	ACK = true;
}

bool Message::Send(struct lws* wsi)
{
	int n = Ptr - Data;
	if (n && (lws_write(wsi, Data, n, LWS_WRITE_BINARY) == n))
	{
		//printf("lws: %u\n", n);
		ACK = false;
		Clear();
		return true;
	}
	return false;
}

inline uint32_t Message::Size()
{
	return Ptr - Data;
}

inline int32_t Message::SpaceLeft()
{
	return _data + MAX_SIZE - Ptr;
}

bool Message::Connected()
{
	return (SpaceLeft() > (MAX_SIZE >> 2));
}

void Message::Clear()
{
	memset(_data, 0, MAX_SIZE);
	Ptr = Data;
}

//void Message::AddMessage(MessageType Type, void * Data)
//{
//	switch (Type)
//	{
//	case MSG_ADDCELL:
//		AddCell Message((Cell *)Data);
//		memcpy(Ptr, &Message, sizeof(AddCell));
//		Ptr += sizeof(AddCell);
//		break;
//
//	case MSG_MOVECELL:
//		MoveCell Message((Cell *)Data);
//		memcpy(Ptr, &Message, sizeof(MoveCell));
//		Ptr += sizeof(MoveCell);
//		break;
//
//	case MSG_REMOVECELL:
//		RemoveCell Message((Cell *)Data);
//		memcpy(Ptr, &Message, sizeof(RemoveCell));
//		Ptr += sizeof(RemoveCell);
//		break;
//
//	default:
//		break;
//	}
//
//}

void Message::AddCell(Cell* cell)
{
	SETUINT8(Ptr, MSG_ADDCELL);
	SETUINT32(Ptr, cell->ID);
	SETFLOAT(Ptr, cell->Center().x);
	SETFLOAT(Ptr, cell->Center().y);
	SETFLOAT(Ptr, cell->Radius);
	SETUINT32(Ptr, cell->Color);
	SETUINT8(Ptr, *(uint8_t*)&(cell->Flag));
	SETINT8(Ptr, -1);
}

void Message::RemoveCell(uint32_t ID)
{
	SETUINT8(Ptr, MSG_REMOVECELL);
	SETUINT32(Ptr, ID);
}

void Message::MoveCell(Cell* cell)
{
	SETUINT8(Ptr, MSG_MOVECELL);
	SETUINT32(Ptr, cell->ID);
	SETFLOAT(Ptr, cell->Center().x);
	SETFLOAT(Ptr, cell->Center().y);
	SETFLOAT(Ptr, cell->Radius);
	SETUINT32(Ptr, cell->Color);
}

void Message::Position(vec2 Position)
{
	SETUINT8(Ptr, MSG_POSITION);
	SETFLOAT(Ptr, Position.x);
	SETFLOAT(Ptr, Position.y);
}