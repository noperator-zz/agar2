#include "Player.h"
#include "Game.h"
#include <time.h>
#include <memory>
#include <algorithm>
#include <iterator>

Player::Player(struct lws * wsi_)
	: wsi(wsi_)
{
	Init();
}

Player::Player(Player&& other)
	: wsi(std::move(other.wsi)), TopLeft(std::move(other.TopLeft)), BottomRight(std::move(other.BottomRight)), Cells(std::move(other.Cells)), SendTime(std::move(other.SendTime)),
	Blocks(std::move(other.Blocks)), WSMessage(std::move(other.WSMessage)), CellHistory(std::move(other.CellHistory)), TotalRadius(std::move(other.TotalRadius)), Camera(std::move(other.Camera))
{

}


Player& Player::operator =(Player&& other)
{
	if (this != &other)
	{
		wsi = std::move(other.wsi);
		TopLeft = std::move(other.TopLeft);
		BottomRight = std::move(other.BottomRight);
		Cells = std::move(other.Cells);
		SendTime = std::move(other.SendTime);
		Blocks = std::move(other.Blocks);
		WSMessage = std::move(other.WSMessage);
		CellHistory = std::move(other.CellHistory);
		TotalRadius = std::move(other.TotalRadius);
		Camera = std::move(other.Camera);
	}
	return *this;
}

Player::~Player()
{

}

void Player::Init()
{
#ifndef FREE_PLAY
	Cell * newCell = new Cell(Game::PLAYER_START_RADIUS, vec2((Map::MAP_SIZE / 2) - Map::BLOCK_SIZE / 2, (Map::MAP_SIZE / 2) - Map::BLOCK_SIZE / 2));
#else
	Cell * newCell = new Cell(1000);// Game::PLAYER_START_RADIUS);
#endif // !FREEPLAY
	newCell->Flag.IsPlayer = true;
	Cells.push_back(newCell);

	UpdateView();

	//add all visible dynamic cells
	//for (auto cell : Map::DynamicMap)
	//{
	//	if (Map::EdgeInside(cell, TopLeft, BottomRight))
	//	{
	//		//Map::CellEvents[Map::CELL_MOVE].push_back(cell);
	//		CellHistory.push_back(cell);
	//	}
	//}

	//for (auto cell : Map::VirusMap)
	//{
	//	if (Map::Inside(cell->Center(), TopLeft, BottomRight))
	//	{
	//		CellHistory.push_back(cell);
	//	}
	//}

	//add all visible player cells
	//for (auto& player : Game::Players)
	//{
	//	for (auto cell : player.Cells)
	//	{
	//		if (Map::EdgeInside(cell, TopLeft, BottomRight))
	//		{
	//			CellHistory.push_back(cell);
	//		}
	//	}
	//}
}

void Player::Acknowledge()
{
	WSMessage.Acknowledge();
}

void Player::CollideStatic()
{
	for (auto cell : Cells)
	{
		for (auto& x : Map::StaticMap)
		{
			for (auto& mapBlock : x)
			{
				int32_t loop = 0;
				for (loop = 0; loop < mapBlock.Data.size(); loop++)
				{
					Cell * staticCell = mapBlock.Data[loop];
					Cell::Collision Result = cell->Collide(staticCell);
					if (Cell::BIGGER == Result)
					{
						//eat it
						cell->AddSize(staticCell->Size());

						Map::CellEvents[Map::CELL_REMOVE_STATIC].push_back(staticCell);
						//remove the cell immediately so that other cells dont try to collide with it as well.
						Map::RemoveStatic(&mapBlock, loop);
						loop--;
					}
				}
			}
		}
	}
}

void Player::CollideDynamic()
{
	int32_t myLoop = 0;
	for (myLoop = 0; myLoop < Cells.size(); myLoop++)
	{
		Cell * cell = Cells[myLoop];
		int32_t loop = 0;
		for (loop = 0; loop < Map::DynamicMap.size(); loop++)
		{
			Cell * dynamicCell = Map::DynamicMap[loop];


			Cell::Collision Result = cell->Collide(dynamicCell);
			if (Cell::BIGGER == Result)
			{
				//eat it
				cell->AddSize(dynamicCell->Size());
				Cell::CellFlag Flag = dynamicCell->Flag;

				Map::CellEvents[Map::CELL_REMOVE_DYNAMIC].push_back(dynamicCell);
				Map::DynamicMap.erase(Map::DynamicMap.begin() + loop);
				loop--;

				if (Flag.IsVirus && (Cells.size() < 16))
				{
					//TODO virus colide
					Cell * popCell = cell;
					Cell * newCell = nullptr;
					uint8_t bigPop = 0;
					float splitRatio = 0.4;

					vec2 Angle = vec2(1, 0);// vec2::Rand();
					while ((popCell->Size() > 500) && (bigPop < 4) && (Cells.size() < 16))
					{
						//split in half
						float size = popCell->Size();
						popCell->AddSize(-size * splitRatio);

						float newR = Cell::GetRadius(size * (1 - splitRatio));
						float SumR = popCell->Radius + newR;
						Angle = Angle.Rotate(6.28 / 4.);
						vec2 Position = popCell->Center() + (Angle * SumR);
						newCell = new Cell(newR, Position, popCell->Color);
						//newCell = new Cell(newR, popCell->Center(), popCell->Color);
						newCell->Flag.IsPlayer = true;
#ifdef NEW_COMBINE
						popCell->CombineTime = Game::CurrentTime + (size / 50);
#endif
						newCell->CombineTime = Game::CurrentTime + (size / 50);
						//newCell->AddForce((newCell->Center() - popCell->Center()).Normalize() * newCell->Size() * 10000);
						newCell->_AddVelocity((newCell->Center() - popCell->Center()).Normalize() * Game::VIRUS_VELOCITY);
						//newCell->Velocity = (newCell->Center() - popCell->Center()).Normalize() * 1000;// Angle * 500;

						Cells.push_back(newCell);
						bigPop++;
						popCell = newCell;
					}

					// //rest of balls come from popCell
					float popSize = cell->Size();
					uint32_t SmallBalls = min(16 - Cells.size(), popSize / Game::PLAYER_MIN_SIZE);
					if (SmallBalls)
					{
						float SizeSmall = min(2 * Game::PLAYER_MIN_SIZE, popSize / (float)(SmallBalls));
						float RadiusSmall = Cell::GetRadius(SizeSmall);
						uint32_t x = 0;
						float s = -(float)SmallBalls * SizeSmall;
						cell->AddSize(s);
						float SumR = cell->Radius + RadiusSmall;

						for (x = 0; x < SmallBalls; x++)
						{
							vec2 Angle = vec2::Rand();
							vec2 Position = cell->Center() + (Angle * SumR);
							newCell = new Cell(Cell::GetRadius(SizeSmall), Position, cell->Color);
							newCell->Flag.IsPlayer = true;
							newCell->CombineTime = Game::CurrentTime + (SizeSmall / 50);
							//newCell->Velocity = Angle * 100;
							//newCell->AddForce((newCell->Center() - dynamicCell->Center()).Normalize() * newCell->Size() * 10000);// Angle * 500;
							newCell->_AddVelocity((newCell->Center() - dynamicCell->Center()).Normalize() * Game::VIRUS_VELOCITY);
							//newCell->Velocity = (newCell->Center() - dynamicCell->Center()).Normalize() * 1000;// Angle * 500;

							Cells.push_back(newCell);
						}
					}

					myLoop--;
					break;
				}
			}
			else if ((Cell::SMALLER == Result) && !(dynamicCell->Flag.IsFood || dynamicCell->Flag.IsVirus))
			{
				dynamicCell->AddSize(cell->Size());

				Map::CellEvents[Map::CELL_REMOVE_DYNAMIC].push_back(cell);
				//WSMessage.RemoveCell(cell->ID);
				Cells.erase(Cells.begin() + myLoop);
				myLoop--;
				break;
			}
		}
	}
}

void Player::CollideSelf()
{
	int32_t myLoop = 0;
	for (myLoop = 0; myLoop < Cells.size(); myLoop++)
	{
		Cell * cell = Cells[myLoop];
		int32_t loop = 0;
		for (loop = myLoop + 1; loop < Cells.size(); loop++)
		{
			Cell * otherCell = Cells[loop];
			float Overlap = 0;
			float PercentOverlap = 0;
			Cell::Collision Result = cell->Collide(otherCell, &Overlap, &PercentOverlap);

			// if both cells are ready to combine
			if ((Game::CurrentTime > cell->CombineTime) && (Game::CurrentTime > otherCell->CombineTime))
			{
				//only let the bigger ball eat the smaller ball
				if (Result >= Cell::OVERLAP)
				{
					//only the bigger ball is supposed to eat
					if (cell->Radius > otherCell->Radius)
					{
						//eat it
						cell->AddSize(otherCell->Size());
						Map::CellEvents[Map::CELL_REMOVE_DYNAMIC].push_back(otherCell);
						Cells.erase(Cells.begin() + loop);
						loop--;
					}
					else
					{
						otherCell->AddSize(cell->Size());
						Map::CellEvents[Map::CELL_REMOVE_DYNAMIC].push_back(cell);
						Cells.erase(Cells.begin() + myLoop);
						// TODO wtf is -1?
						myLoop = max(-1, myLoop - 1);
						continue;
					}
				}
			}
			else
			{
				if (Result > Cell::Collision::NONE)
				{
					float MousePower = cell->MouseVector.Length() < cell->Radius;// max(0, ((cell->Radius - cell->MouseVector.Length()) / cell->Radius) - 0.5);
#ifdef CELL_FORCE
	#ifdef MOVE_SIZE
					float Size = cell->Size();
					float otherSize = otherCell->Size();
	#else
					float Size = cell->Radius;// cell->Size();
					float otherSize = otherCell->Radius;// otherCell->Size();
	#endif
					float Alpha = otherSize / (Size + otherSize);
#else
					float Alpha = 0.5;
#endif
					//Alpha -= 10 * MousePower;


					float strengthMult = 0.5;
					float maxStrength = 50;
					vec2 Intersect = (otherCell->Center() - cell->Center()).Normalize() * min(maxStrength, Overlap * strengthMult);

					
					//cell->AddForce(Intersect * -max(0, Alpha));// *cell->Size());
					//otherCell->AddForce(Intersect * (1 - Alpha));// *otherCell->Size());
					cell->_MoveRel(Intersect * -max(0, Alpha));
					otherCell->_MoveRel(Intersect * (1 - Alpha));
				}
			}
		}
	}
}


void Player::CollideOtherPlayer(uint32_t Start)
{
	uint32_t x = 0;
	for (x = Start; x < Game::Players.size(); x++)
	{
		int32_t myLoop = 0;
		for (myLoop = 0; myLoop < Cells.size(); myLoop++)
		{
			Cell * cell = Cells[myLoop];
			int32_t loop = 0;
			for (loop = 0; loop < Game::Players[x].Cells.size(); loop++)
			{
				Player * player = &Game::Players[x];
				Cell * otherCell = player->Cells[loop];
				Cell::Collision Result = cell->Collide(otherCell);
				if (Cell::BIGGER == Result)
				{
					//eat it
					cell->AddSize(otherCell->Size());

					Map::CellEvents[Map::CELL_REMOVE_DYNAMIC].push_back(otherCell);
					//DeleteDynamic(otherCell);

					player->Cells.erase(player->Cells.begin() + loop);
					loop--;
				}
				else if (Cell::SMALLER == Result)
				{
					otherCell->AddSize(cell->Size());

					Map::CellEvents[Map::CELL_REMOVE_DYNAMIC].push_back(cell);
					//WSMessage.RemoveCell(cell->ID);
					Cells.erase(Cells.begin() + myLoop);
					myLoop--;
					break;
				}
			}
		}
	}
}

//#define SPLIT_OVERLAPPING
void Player::Update()
{
	//if (!WSMessage.IsEmpty())
	//{
	//	printf("Skipping update\n");
	//	return;
	//}

	if (DoSplit)
	{
		DoSplit = false;
		uint32_t x = 0;

		uint32_t OriginalSize = Cells.size();
		for (x = 0; (x < OriginalSize) && (Cells.size() < 16); x++)
		{
			Cell * cell = Cells[x];
			if (cell->Radius > 42)
			{
				float SizeBefore = cell->Size();
				float Radius = Cell::GetRadius(SizeBefore / 2);
				cell->Radius = Radius;
#ifdef SPLIT_OVERLAPPING
				Cell * newCell = new Cell(Radius, cell->Center(), cell->Color);
#else
				Cell * newCell = new Cell(Radius, cell->Center() + (cell->MouseVector.Normalize() * 2 * Radius), cell->Color);
#endif
#ifdef NEW_COMBINE
				cell->CombineTime = Game::CurrentTime + (SizeBefore / 50);
#endif
				newCell->CombineTime = Game::CurrentTime + (SizeBefore / 50);
				//newCell->AddForce(cell->MouseVector.Normalize() * Game::SPLIT_FORCE * newCell->Size());
				newCell->_AddVelocity(cell->MouseVector.Normalize() * Game::SPLIT_VELOCITY);
				//newCell->Velocity = cell->MouseVector.Normalize() * 2000;
				newCell->Flag.IsPlayer = true;
				Cells.push_back(newCell);
			}
		}
	}

	if (DoFeed)
	{
		DoFeed = false;
		for (auto cell : Cells)
		{
			//if big enough to eject mass
			if ((cell->Size() - Game::FEED_SIZE) > Game::PLAYER_MIN_SIZE)
			{
				cell->AddSize(-Game::FEED_SIZE);

				float Radius = Cell::GetRadius(Game::FEED_SIZE * Game::FEED_MULT);
				vec2 Direction = cell->MouseVector.Normalize();
				vec2 Perp = Direction.Perp();
				Direction += Perp * Game::FEED_DEVIATION * (((float)rand() / RAND_MAX) - 0.5);
				vec2 Position = cell->Center() + (Direction * (cell->Radius + Radius));

				Cell * feed = new Cell(Radius, Position, cell->Color);
				feed->AddForce(Direction * Game::FEED_FORCE * feed->Size());
				//feed->Velocity = Direction;
				feed->IsFood(true);
				Map::AddDynamic(feed);
				//Map::CellEvents[Map::CELL_MOVE].push_back(feed);
			}
		}
	}

	int32_t x = 0;
	while (x < CellHistory.size())
		//while (history < CellHistory.end())
	{
		Cell * history = CellHistory[x];
		if (!Map::Inside(history->Center(), TopLeft, BottomRight))
		{
			CellHistory.erase(CellHistory.begin() + x);
			WSMessage.RemoveCell(history->ID);
		}
		else
		{
			x++;
		}
	}

	ParseEvents();
	UpdateBlocks();
	//printf("%u - X: %5u, Y: %5u\n", wsi, (uint32_t)Position.x, (uint32_t)Position.y);
	WSMessage.Position(Camera);
}

void Player::RemoveBlock(Map::Block* block)
{
	for (auto cell : block->Data)
	{
		WSMessage.RemoveCell(cell->ID);
	}
}


void Player::AddBlock(Map::Block* block)
{
	for (auto cell : block->Data)
	{
		WSMessage.AddCell(cell);
	}
}


//Update which blocks are visible
void Player::UpdateBlocks()
{
	vec2i OldTL = TopLeft;
	vec2i OldBR = BottomRight;
	std::vector<Map::Block*> OldBlocks = std::move(Blocks);

	UpdateView();

	//for (auto cell : Map::DynamicMap)
	//{
	//	if (Map::EdgeInside(cell, TopLeft, BottomRight))
	//	{
	//		CellHistory.push_back(cell);
	//	}
	//}

	if ((OldTL != TopLeft) || (OldBR != BottomRight))
	{
		int32_t o = 0, oStart = 0, oEnd = OldBlocks.size() - 1, n = 0, nStart = 0, nEnd = Blocks.size() - 1;

		n = nStart;
		o = oStart;

		std::vector<Map::Block *> New, Old;

		std::set_difference(
			OldBlocks.begin(),
			OldBlocks.end(),
			Blocks.begin(),
			Blocks.end(),
			std::back_inserter(Old));


		std::set_difference(
			Blocks.begin(),
			Blocks.end(),
			OldBlocks.begin(),
			OldBlocks.end(),
			std::back_inserter(New));


		for (auto block : Old)
		{
			RemoveBlock(block);
		}
		for (auto block : New)
		{
			AddBlock(block);
		}
	}
}

void Player::UpdateView()
{
	CalculateCamera();
#ifdef FREE_MAP
	float view = (4 + Cells.size() * 3) * (TotalRadius / Cells.size()) + 1000;
#else
	float view = (float)Map::BLOCK_SIZE;
#endif
	TopLeft = Map::IndexFromCoord(Camera - vec2(view*1.6, view));
	BottomRight = Map::IndexFromCoord(Camera + vec2(view*1.6, view));

	Blocks.clear();

	int32_t x, y;
	for (x = TopLeft.x; x <= BottomRight.x; x++)
	{
		for (y = TopLeft.y; y <= BottomRight.y; y++)
		{
			Blocks.push_back(&Map::StaticMap[x][y]);
		}
	}
}

void Player::CalculateCamera()
{
	TotalRadius = 0;
	for (auto cell : Cells)
	{
		TotalRadius += cell->Radius;
	}

	float Inv = 1.f / TotalRadius;
	vec2 Pos;
	for (auto cell : Cells)
	{
		float weight = cell->Radius * Inv;
		Pos += (cell->Center() * weight);
	}
	float a = 0.95;
	Camera = (Camera * a + Pos * (1.-a));
}

bool Player::Connected()
{
	return WSMessage.Connected();
}

//ONLY CALL THIS FROM WRITABLE CALLBACK
bool Player::Send(float CurrentTime)
{
	if (CanSend(CurrentTime) && WSMessage.Send(wsi))
	{
		SendTime = CurrentTime;
		return true;
	}
	return false;
}

void Player::Mouse(vec2 Mouse)
{
	for (auto cell : Cells)
	{
		cell->MouseVector = Mouse - cell->Center();
	}
}



//Update which cells are visible
void Player::ParseEvents()
{
	//determine which blocks are currently visible

	Map::Block * NeedsUpdate;

	for (const auto& EventVector : Map::CellEvents)
	{
		uint32_t EventType = &EventVector - &Map::CellEvents[0];
		for (const auto cell : EventVector)
		{
			switch (EventType)
			{
			case Map::CELL_MOVE:
				if ((NeedsUpdate = Map::EdgeInside(cell, TopLeft, BottomRight)))
				{
					//If we've seen this dynamic cell before it will be in CellHistory
					bool Done = false;
					for (const auto history : CellHistory)
					{
						//if we already know about this cell
						if (cell == history)
						{
							Done = true;
							if (cell->Flag.IsPlayer || cell->GetVelocity())
							{
								WSMessage.MoveCell(cell);
							}
							break;
						}
					}
					if (!Done)
					{
						//not seen before, do an add event instead
						//TODO make this not a copy of CELL_ADD_DYNAMIC
						//if the cell is in our bounding box

						//not in CellHistory. Add it
						CellHistory.push_back(cell);
						//let the client know about it
						WSMessage.AddCell(cell);
					}
				}
				break;

			case Map::CELL_ADD_STATIC:
				//if the cell is in our bounding box
				if ((NeedsUpdate = Map::Inside(cell->Center(), TopLeft, BottomRight)))
				{
					//let the client know about it
					WSMessage.AddCell(cell);
				}
				break;

			case Map::CELL_REMOVE_STATIC:
				if ((NeedsUpdate = Map::Inside(cell->Center(), TopLeft, BottomRight)))
				{
					DeleteStatic(cell->ID);
				}
				break;

			case Map::CELL_ADD_DYNAMIC:
				////if the cell is in our bounding box
				//if ((NeedsUpdate = Map::Inside(cell->Center(), TopLeft, BottomRight)))
				//{
				//	bool Done = false;
				//	for (const auto history : CellHistory)
				//	{
				//		//if we already know about this cell
				//		if (cell->ID == history)
				//		{
				//			Done = true;
				//			break;
				//		}
				//	}
				//	if (!Done)
				//	{
				//		//not in CellHistory. Add it
				//		CellHistory.push_back(cell->ID);
				//		//let the client know about it
				//		WSMessage.AddCell(cell);
				//	}
				//}
				break;

			case Map::CELL_REMOVE_DYNAMIC:
				if ((NeedsUpdate = Map::EdgeInside(cell, TopLeft, BottomRight)))
				{
					DeleteDynamic(cell);
				}
				break;

			default:
				exit(-1);
			}
		}
	}
}

void Player::DeleteStatic(uint32_t ID)
{
	WSMessage.RemoveCell(ID);
}

void Player::DeleteDynamic(Cell * cell)
{
	int32_t x = 0;
	while (x < CellHistory.size())
	{
		Cell * history = CellHistory[x];
		//if we already know about this cell
		if (cell == history)
		{
			CellHistory.erase(CellHistory.begin() + x);
			WSMessage.RemoveCell(cell->ID);
			//TODO we do we get duplicates?
			break;
		}
		else
		{
			x++;
		}
	}
}

bool Player::CanSend(float CurrentTime)
{
	float diff = CurrentTime - SendTime;

	//if its been longer than the ACK timeout, send a new packet
	//of if the packet has been acknowledged and it has been more than the minimum send interval

	if ((diff > Game::TIMEOUT_ACK) || (WSMessage.ACK && (diff > Game::SEND_INTERVAL)))
	{
		return true;
	}
	return false;
}