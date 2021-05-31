#pragma once
#include "Player.h"
#include <uv.h>
#include "lws_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>
#include "libwebsockets.h"
#include "Map.h"
#include "cJSON.h"

#define MAX_ECHO_PAYLOAD 1024
#define PLAYER_NAME_LEN 20

#define SEQ_LEN 100

// alternate player cell constuctor
#define FREE_PLAY
// 
#define FREE_MAP
// ball that split / was popped changes its combine time
#define NEW_COMBINE
// slow down when mouse inside cell
#define CELL_PRECISION


// bigger cells are moved less during collisions
#define CELL_FORCE
// calculate a cells collision force based on size instead of radius. MUST ENABLE CELL_FORCE
//#define MOVE_SIZE

// ==================== GAME IDEAS ====================
//
// collision with olther player cells if they are same size
//  eliminates overlap-feed teaming
//  completely breaks overlap virusing 
//  naw
//
// 


class Game
{
public:

	static constexpr float TIMEOUT_ACK = 5.f;
	static constexpr float SEND_INTERVAL = 1.f / 60.f;

	static constexpr uint32_t MAX_VIRUS = 100;
	static constexpr uint32_t MIN_VIRUS = 10;

	static constexpr uint32_t MAX_FOOD = 1000;
	static constexpr uint32_t MIN_FOOD = 1000;

	static constexpr float FOOD_RADIUS = 32;

	//how much size is lost on pressing W
	static constexpr float FEED_SIZE = 20;
	//what percentage of FEED_SIZE is left after ejecting feed
	static constexpr float FEED_MULT = 0.75;

	static constexpr float VIRUS_MIN_RADIUS = 100;
	static constexpr float VIRUS_MAX_RADIUS = 200;


	static constexpr uint32_t PLAYER_MAX_CELLS = 16;

	static constexpr float PLAYER_MIN_RADIUS = 42;
	static constexpr float PLAYER_MIN_SIZE = Cell::GetSize(PLAYER_MIN_RADIUS);

	static constexpr float PLAYER_START_RADIUS = 150;
	static constexpr float PLAYER_MAX_RADIUS = 1500;

	static constexpr float SIZE_DAMPING = 2.2;
	static constexpr float MINIMUM_DAMPING = 0.01;
	static constexpr float MAXIMUM_DAMPING = 0.05;
	static constexpr float FEED_DAMPING = 0.02;
	
	static constexpr float SPLIT_VELOCITY = 5000;// 25'000'000 * VELOCITY_DAMPING;
	static constexpr float FEED_FORCE = 300'000;// *VELOCITY_DAMPING;
	static constexpr float MOUSE_FORCE = 1'200;// *VELOCITY_DAMPING;
	static constexpr float VIRUS_VELOCITY = 3000;// *VELOCITY_DAMPING;

	// 1 = 45 degrees
	static constexpr float FEED_DEVIATION = 0.2;

	static float CurrentTime;

	Game();
	~Game();

	static void game_loop_cb(uv_timer_t* handle);
	static int callback_wsapi(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len);
	static std::vector<Player> Players;

private:
	static void ParseEvents();
};

