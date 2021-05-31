#pragma once
#include <stdint.h>
#include "libwebsockets.h"
#include "Cell.h"

#define SETFLOAT(ptr, value) *(float*)ptr = (float)value; ((uint8_t*)ptr) += sizeof(float);
#define SETUINT32(ptr, value) *(uint32_t*)ptr = (uint32_t)value; ((uint8_t*)ptr) += sizeof(uint32_t);
#define SETUINT8(ptr, value) *((uint8_t*)ptr) = (uint8_t)value; ((uint8_t*)ptr) += sizeof(uint8_t);
#define SETINT8(ptr, value) *((int8_t*)ptr) = (int8_t)value; ((uint8_t*)ptr) += sizeof(int8_t);

#define GETUINT8(ptr) *((uint8_t*)(ptr))
#define GETUINT16(ptr) *((uint16_t*)(ptr))
#define GETUINT32(ptr) *((uint32_t*)(ptr))
#define GETINT16(ptr) *((int16_t*)(ptr))
#define GETFLAOT(ptr) *((float*)(ptr))



class Message
{
public:
	Message();
	~Message();

	Message::Message(Message&& other);
	Message& operator =(Message&& other);


	
	void Clear();
	void AddCell(Cell * cell);
	void MoveCell(Cell * cell);
	void RemoveCell(uint32_t ID);
	void Position(vec2 Position);
	//void AddMessage(MessageType Type, void * Data);
	bool Send(struct lws* wsi);
	uint32_t Size();
	int32_t SpaceLeft();
	bool Connected();
	bool IsEmpty();
	void Acknowledge();

	bool ACK;

	enum MessageType
	{
		MSG_ADDCELL = 0x00,
		MSG_REMOVECELL = 0x01,
		MSG_MOVECELL = 0x02,

		MSG_POSITION = 0x10,

		MSG_MOUSE = 0x80,
		MSG_SPLIT = 0x81,
		MSG_FEED = 0x82,

		MSG_ACK = 0xFF
	};


#pragma pack(push, 1)
//	struct AddCell
//	{
//		AddCell(Cell * cell)
//			: MSG_ID(MSG_ADDCELL), ID(cell->ID), x(cell->Center().x), y(cell->Center().y), radius(cell->Radius), color(cell->Color), flag(cell->Flag.u) {}
//
//		AddCell(uint32_t ID, float x, float y, float radius, uint32_t color, uint8_t flag)
//			: MSG_ID(MSG_ADDCELL), ID(ID), x(x), y(y), radius(radius), color(color), flag(flag) {}
//
//		uint8_t MSG_ID;
//		uint32_t ID;
//		float x;
//		float y;
//		float radius;
//		uint32_t color;
//		uint8_t flag;
//	};
//
//	struct MoveCell
//	{
//		MoveCell(Cell * cell)
//			: MSG_ID(MSG_MOVECELL), ID(cell->ID), x(cell->Center().x), y(cell->Center().y), radius(cell->Radius), color(cell->Color) {}
//
//		MoveCell(uint32_t ID, float x, float y, float radius, uint32_t color, uint8_t flag)
//			: MSG_ID(MSG_MOVECELL), ID(ID), x(x), y(y), radius(radius), color(color) {}
//
//		uint8_t MSG_ID;
//		uint32_t ID;
//		float x;
//		float y;
//		float radius;
//		uint32_t color;
//	};
//
//	struct RemoveCell
//	{
//		RemoveCell(Cell * cell)
//			: MSG_ID(MSG_REMOVECELL), ID(cell->ID) {}
//
//		RemoveCell(uint32_t ID)
//			: MSG_ID(MSG_REMOVECELL), ID(ID) {}
//
//		uint8_t MSG_ID;
//		uint32_t ID;
//	};

	struct Mouse
	{
		Mouse()
			: MSG_ID(MSG_MOUSE), x(0), y(0) {}


		Mouse(uint8_t * const Ptr)
			: MSG_ID(GETUINT8(Ptr)), x(GETINT16(Ptr+1)), y(GETINT16(Ptr+3)) {}

		uint8_t MSG_ID;
		int16_t x;
		int16_t y;
	};

#pragma pack(pop)


private:

	//static constexpr uint32_t MAX_SIZE = 1 << 27;
	static constexpr uint32_t MAX_SIZE = 1 << 16;

	uint8_t * _data;
	uint8_t * Data;
	uint8_t * Ptr;
};