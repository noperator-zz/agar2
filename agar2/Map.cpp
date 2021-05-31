#include "Map.h"
#include "Game.h"

Map::Block Map::StaticMap[Map::NUM_BLOCKS][Map::NUM_BLOCKS];
std::vector<Cell *> Map::DynamicMap;
std::vector<Cell *> Map::CellEvents[Map::__LAST];

 vec2i Map::IndexFromCoord(vec2 Coord)
{
	return vec2i((min((int32_t)max(Coord.x, 0.f), (int32_t)MAP_SIZE-1)) / (int32_t)BLOCK_SIZE, (min((int32_t)max(Coord.y, 0.f), (int32_t)MAP_SIZE-1)) / (int32_t)BLOCK_SIZE);
}

 Map::Block * Map::BlockFromCoord(float x, float y)
{
	return BlockFromCoord(vec2(x, y));
}

 Map::Block * Map::BlockFromCoord(vec2 Coord)
{
	vec2i Block = IndexFromCoord(Coord);
	return  &StaticMap[Block.x][Block.y];
}


bool Map::AddStatic(Cell * const cell)
{
	Block * Block = BlockFromCoord(cell->Center());
	//check that it doesnt exist
	for (const auto blockCell : Block->Data)
	{
		if (cell->ID == blockCell->ID)
			return false;
	}
	Block->Data.push_back(cell);
	Block->ChangeID++;
	//TODO create add event
	//Map::CellEvents[CELL_ADD_STATIC].push_back(cell);
	return true;
}

bool Map::RemoveStatic(Block * block, uint32_t index)
{
	if (block->Data.size() > index)
	{
		block->Data.erase(block->Data.begin() + index);
		block->ChangeID++;
		return true;
	}
	return false;
}

bool Map::RemoveStatic(Cell * const cell)
{
	Block * Block = BlockFromCoord(cell->Center());
	auto blockCell = Block->Data.begin();
	while (blockCell < Block->Data.end())
	{
		if (cell->ID == (*blockCell)->ID)
		{
			Block->Data.erase(blockCell);
			Block->ChangeID++;
			return true;
		}
		blockCell++;
	}
	return false;
}


bool Map::AddDynamic(Cell * const cell)
{
	DynamicMap.push_back(cell);
	return true;
}

bool Map::RemoveDynamic(uint32_t Index)
{
	if (DynamicMap.size() > Index)
	{
		DynamicMap.erase(DynamicMap.begin() + Index);
		return true;
	}
	return false;
}

bool Map::RemoveDynamic(Cell * const cell)
{
	int32_t x = 0;
	for (x = DynamicMap.size()-1; x >= 0; x--)
	{
		if (cell == DynamicMap[x])
		{
			DynamicMap.erase(DynamicMap.begin() + x);
			return true;
		}
	}
	return false;
}

void Map::Init()
{
	printf("Init\n");

	uint32_t x = 0;
	uint32_t y = 0;
	for (x = 0; x < NUM_BLOCKS; x++)
	{
		for (y = 0; y < NUM_BLOCKS; y++)
		{
			StaticMap[x][y] = Block();
		}
	}
	DynamicMap.clear();

#ifdef FREE_MAP
	for (x = 0; x < Game::MAX_FOOD; x++)
	{
		Cell * const food = new Cell();
#else
	for (x = 0; x < (Map::NUM_BLOCKS * Map::NUM_BLOCKS); x++)
	{
		Cell * const food = new Cell(Game::FOOD_RADIUS, vec2(Map::BLOCK_SIZE / 2 + Map::BLOCK_SIZE * (x / Map::NUM_BLOCKS), Map::BLOCK_SIZE / 2 + Map::BLOCK_SIZE * (x % Map::NUM_BLOCKS)));
#endif
		food->IsFood(true);
		AddStatic(food);
	}

#ifdef FREE_MAP
	for (x = 0; x < Game::MAX_VIRUS; x++)
	{
		Cell * const virus = new Cell(Game::VIRUS_MIN_RADIUS, Map::RandomPosition(), 0x00FF00);
		virus->IsVirus(true);
		AddDynamic(virus);
	}
#endif
}


Map::Block * Map::EdgeInside(Cell * cell, vec2i Start, vec2i End)
{

	End += vec2i(1, 1);
	vec2 Pos = cell->Center() / (float)BLOCK_SIZE;
	vec2i Posi = IndexFromCoord(cell->Center());

	float Radius = cell->Radius / (float)BLOCK_SIZE;

	if (((Pos.x + Radius) >= Start.x) && ((Pos.y + Radius) >= Start.y) && ((Pos.x - Radius) <= End.x) && ((Pos.y - Radius) <= End.y))
	{
		return &StaticMap[Posi.x][Posi.y];
	}
	return nullptr;
}

Map::Block * Map::Inside(vec2 Point, vec2i Start, vec2i End)
{
	//cell X > TL X
	vec2i Pos = IndexFromCoord(Point);

	if ((Pos.x >= Start.x) && (Pos.y >= Start.y) && (Pos.x <= End.x) && (Pos.y <= End.y))
	{
		return &StaticMap[Pos.x][Pos.y];
	}
	return nullptr;
}