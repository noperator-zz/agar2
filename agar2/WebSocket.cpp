#include "Game.h"
#include <time.h>

struct ws_message_t
{
public:
	ws_message_t()
	{
		_data = new uint8_t[LWS_PRE + 10000000]();
		Data = _data + LWS_PRE;
		Ptr = Data;
	}

	~ws_message_t()
	{
		delete (_data);
	}

	void clear()
	{
		Ptr = Data;
	}

	uint32_t size()
	{
		return Ptr - Data;
	}

	uint8_t * _data;
	uint8_t * Data;
	uint8_t * Ptr;
};

uv_timer_t timeout_watcher;

//main ws api function , which is called each time client invoke API 
int Game::callback_wsapi(struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len)
{
	static ws_message_t * Message = nullptr;
	unsigned char request_from_client_buf[LWS_PRE + 1024];
	int n;
	float CurrentTime = (float)clock() / (float)CLOCKS_PER_SEC;
	char buf[10];



	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
	{
		srand((unsigned int)time(NULL));
		uv_timer_init(lws_uv_getloop(lws_get_context(wsi), 0), &timeout_watcher);
		//every 100ms
		uv_timer_start(&timeout_watcher, game_loop_cb, 1000, 10);

		Map::Init();

		break;
	}
	case LWS_CALLBACK_ESTABLISHED:
	{

		break;
	}
	case LWS_CALLBACK_SERVER_WRITEABLE:
	{
		//printf("Writable: %X\n", (uint32_t)wsi);
		bool found = false;
		for (auto& player : Players)
		{
			if (wsi == player.wsi)
			{
				found = true;
				//break;
			}

			player.Send(CurrentTime);
		}

		if (!found)
		{
			printf("New player: %p\n", wsi);
			Players.emplace_back(std::move(Player(wsi)));
		}

		break;
	}
	case LWS_CALLBACK_RECEIVE:
	{
		if (len < 1)
		{
			break;
		}

		uint8_t * DataPtr = (uint8_t*)in;
		Message::Mouse Mouse;

		for (auto& player : Players)
		{
			if (wsi == player.wsi)
			{
				player.Acknowledge();

				switch (DataPtr[0])
				{
					//case Message::MSG_ACK:
					//	player.Acknowledge();
					//	break;
				case Message::MSG_MOUSE:
					Mouse = Message::Mouse(DataPtr);
					player.Mouse(vec2(Mouse.x, Mouse.y));
					break;

				case Message::MSG_SPLIT:
					player.DoSplit = true;
					break;

				case Message::MSG_FEED:
					player.DoFeed = true;
					break;

				default:
					break;
				}
			}
		}

		//Only invoke callback back to client when baby client is ready to eat 
		lws_callback_on_writable(wsi);
		break;
	}
	/*
	* this just demonstrates how to use the protocol filter. If you won't
	* study and reject connections based on header content, you don't need
	* to handle this callback
	*/
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		//dump_handshake_info(wsi);
		/* you could return non-zero here and kill the connection */
		break;

		/*
		* this just demonstrates how to handle
		* LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
		* code and auxiliary data.  You can just not handle it if you don't
		* have a use for this.
		*/
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
	{
		uint32_t x = 0;
		for (x = 0; x < Players.size(); x++)
		{
			if (wsi == Players[x].wsi)
			{
				printf("Bye player: %p\n", wsi);
				Players.erase(Players.begin() + x);
				break;
			}
		}
		lwsl_notice("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: len %d\n", len);
		for (n = 0; n < (int)len; n++)
		{
			lwsl_notice(" %d: 0x%02X\n", n, ((unsigned char *)in)[n]);
		}
	}

	default:
		buf[0] = 0;
		break;
	}
	//printf(buf);
	return 0;
}