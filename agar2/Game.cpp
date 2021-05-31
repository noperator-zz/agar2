#include "Game.h"
#include <vector>
#include "Cell.h"
#include "Vec.h"
#include "Color.h"
#include "Player.h"
#include "Map.h"
#include <time.h>
#include <queue>
#include <deque>
#include <memory>
#include "Message.h"

#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
    	    }                                                 \
      } while (0)

std::vector<Player> Game::Players;
float Game::CurrentTime = 0;




Game::Game()
{
	
}


Game::~Game()
{

}


float vel = 1;
//struct Game::per_session_data__apigataway *pss;
void Game::game_loop_cb(uv_timer_t* handle)
{
	float OldTime = CurrentTime;
	CurrentTime = (float)clock() / (float)CLOCKS_PER_SEC;
	float dt = CurrentTime - OldTime;

	ASSERT(handle != NULL);
	ASSERT(1 == uv_is_active((uv_handle_t*)handle));
	//printf("loop\n");

	Map::CellEvents[Map::CELL_ADD_STATIC].clear();
	Map::CellEvents[Map::CELL_REMOVE_STATIC].clear();
	Map::CellEvents[Map::CELL_ADD_DYNAMIC].clear();
	Map::CellEvents[Map::CELL_REMOVE_DYNAMIC].clear();
	Map::CellEvents[Map::CELL_MOVE].clear();
	/////////////////////////////////////////////////////////////////////////////////////////////// STATIC CELL UPDATES
	//game logic
	//for (auto& player : Players)
	//{
	//	for (auto cell : player.Cells)
	//	{
	//		cell->Color = 0xFF0000;
	//	}
	//}

	for (auto cell : Map::DynamicMap)
	{
		cell->Update(dt);
	}

	int32_t x = 0;
	for (x = 0; x < Players.size(); x++)
	{
		if (!Players[x].Connected())
		{
			printf("Player overflow: %u, %u", x, Players[x].wsi);
			Players.erase(Players.begin() + x);
			x--;
		}
	}

	for (auto& player : Players)
	{
		player.CollideStatic();
		player.CollideDynamic();
		player.CollideSelf();
	}

	uint32_t i = 0;
	//if there are 3 players:
	//all of player 0's cells are checked against all of player 1 and player 2's cells
	//then all of player 1's cells are check against all of ONLY player 2's cells
	//don't need to check against player 0 because player 0 already checked against player 1
	//therefore, the CollideDynamic function needs to know which player to start colliding with
	for (i = 0; i < Players.size(); i++)
	{
		Players[i].CollideOtherPlayer(i+1);
	}


	/////////////////////////////////////////////////////////////////////////////////////////////// DYNAMIC CELL UPDATES
	for (auto& player : Players)
	{
		//printf("Player ")
		for (auto cell : player.Cells)
		{
			cell->Update(dt);

			vec2 Direction = cell->MouseVector.Normalize();
			float len = cell->MouseVector.Length();
			Direction *= Game::MOUSE_FORCE * cell->Size();// *pow(cell->Radius, -0.22);

			float alpha = len / cell->Radius;

#ifdef CELL_PRECISION
			if (alpha < 1)
			{
				Direction *= alpha;
			}
#else

			if (alpha < 0.5) {
				//Direction *= 1 + (4 * alpha);
			}
#endif

#ifndef FREE_PLAY
			cell->MoveRel(vec2(0, 1));
#else
			if (alpha > 0.1) {
				cell->AddForce(Direction);
				//cell->MoveRel(Direction * dt);
			}
#endif
			Map::CellEvents[Map::CELL_MOVE].push_back(cell);
		}
	}

	for (auto cell : Map::DynamicMap)
	{
		//TODO update position
		//probly do this at top

		Map::CellEvents[Map::CELL_MOVE].push_back(cell);
		
	}

	for (auto& player : Players)
	{
		/////////////////////////////////////////////////////////////////////////////////////////// PLAYER UPDATES
		//calculate new view and create WS message based on changes
		player.Update();

		/////////////////////////////////////////////////////////////////////////////////////////// WEBSOCKET
		lws_callback_on_writable(player.wsi);
	}

	ParseEvents();

}

void Game::ParseEvents()
{
	//determine which blocks are currently visible

	Map::Block * NeedsUpdate;

	for (const auto& EventVector : Map::CellEvents)
	{
		uint32_t EventType = &EventVector - &Map::CellEvents[0];
		if (EventType == Map::CELL_MOVE)
		{
			continue;
		}

		for (const auto cell : EventVector)
		{
			switch (EventType)
			{
			case Map::CELL_MOVE:
				break;

			case Map::CELL_ADD_STATIC:
				Map::AddStatic(cell);
				break;

			case Map::CELL_REMOVE_STATIC:
				Map::RemoveStatic(cell);
				break;

			case Map::CELL_ADD_DYNAMIC:
				Map::AddDynamic(cell);
				break;

			case Map::CELL_REMOVE_DYNAMIC:
				Map::RemoveDynamic(cell);
				break;

			default:
				exit(-1);
			}
		}
	}
}
