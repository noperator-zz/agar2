#pragma once

#include <vector>
#include "libwebsockets.h"
#include "Cell.h"
#include "Map.h"
#include "Message.h"

class Player
{
public:
	Player(struct lws * wsi_);
	~Player();
	Player(Player&& other);
	Player& operator =(Player&& other);


	void CollideStatic();
	void CollideDynamic();
	void CollideSelf();
	void CollideOtherPlayer(uint32_t Start);

	void Update();
	void RemoveBlock(Map::Block * block);
	void AddBlock(Map::Block * block);
	void UpdateBlocks();
	void CalculateCamera();
	bool Connected();
	//ONLY CALL THIS FROM WRITABLE CALLBACK
	bool Send(float CurrentTime);
	void Mouse(vec2 Mouse);

	void Acknowledge();

	struct lws * wsi;
	std::vector<Cell *> Cells;
	vec2 Camera;
	bool DoSplit = false;
	bool DoFeed = false;
private:
	//top left corner and width/height
	vec2i TopLeft, BottomRight;
	float TotalRadius;

	float SendTime = 0.f;

	//list of currently visible Blocks
	std::vector<Map::Block *> Blocks;
	std::vector<Cell *> CellHistory;
	Message WSMessage;

	bool CanSend(float CurrentTime);
	void ParseEvents();
	void DeleteStatic(uint32_t ID);
	void DeleteDynamic(Cell * cell);
	void UpdateView();

	void Init();

};