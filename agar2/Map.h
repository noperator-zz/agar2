#pragma once
#include <stdint.h>
#include <vector>
#include "Vec.h"
#include <memory>

class Cell;
namespace Map
{
	constexpr uint32_t BLOCK_SIZE = 100;
	constexpr uint32_t NUM_BLOCKS = 160;
	constexpr uint32_t MAP_SIZE = BLOCK_SIZE * NUM_BLOCKS;

	enum Event
	{
		CELL_MOVE = 0,
		CELL_ADD_STATIC,
		CELL_REMOVE_STATIC,
		CELL_ADD_DYNAMIC,
		CELL_REMOVE_DYNAMIC,

		__LAST
	};

	struct Block
	{
		Block()
			: ChangeID(0), Data() {}

		uint64_t ChangeID;
		std::vector<Cell *> Data;
	};


	extern Block StaticMap[NUM_BLOCKS][NUM_BLOCKS];
	extern std::vector<Cell *> DynamicMap;
	extern std::vector<Cell *> CellEvents[__LAST];

	struct BlockHistory
	{
		BlockHistory(Block * block)
			: Ptr(block), LastChangeID(block->ChangeID) {}

		Block * Ptr;
		uint64_t LastChangeID;
	};

	void Init();
	static vec2 RandomPosition()
	{
		return vec2((float)(rand() % MAP_SIZE), (float)(rand() % MAP_SIZE));
	}

	//int32_t CellData(uint8_t ** ptr);
	bool AddStatic(Cell * const cell);
	bool RemoveStatic(Block * block, uint32_t index);
	bool RemoveStatic(Cell * const cell);
	bool AddDynamic(Cell * const cell);
	bool RemoveDynamic(uint32_t Index);
	bool RemoveDynamic(Cell * const cell);
	Block * EdgeInside(Cell * cell, vec2i Start, vec2i End);
	Block * Inside(vec2 Point, vec2i Start, vec2i Size);

	 vec2i IndexFromCoord(vec2 Coord);
	 Block * BlockFromCoord(vec2 Coord);
	 Block * BlockFromCoord(float x, float y);
};

