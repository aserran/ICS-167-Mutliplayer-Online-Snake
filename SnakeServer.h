//Alex Deuling		94357349
//Jared Napoli		31666553
//Noel Biscocho		10062289
//Anthony Serrano	53934109

#ifndef SNAKE_SERVER_H
#define SNAKE_SERVER_H

#include "Snake.hpp"
#include "websocket.h"
#include <Windows.h>

class SnakeServer
{
public:
	~SnakeServer();
	static bool setup(u_int r = 45, u_int c = 45, u_int t = 200, u_int aip = 0, bool ds = false);
	static void startServer(u_int port = 100);

	static void turnOnArtificialLatency();
	static void turnOffArtificialLatency();
	static void turnOnTheLOLs();
	static void turnOffTheLOLs();

	static void setMaxLatency(u_int ml);
	static void setMinLatency(u_int ml);

	static void addWalls(std::list<Snake::Pos> posList);

private:


	struct Message
	{
		clock_t handleAtThisTime;
		int clientID;
		std::string msg;
		bool operator < (const Message& msg1) const { return this->handleAtThisTime > msg1.handleAtThisTime; };
	};

	struct PlayerInfo
	{
		std::string name;
		bool ready;
		int snakeNum;
	};

	static webSocket server;
	static Snake* game;
	static bool setupDone;
	static bool artificialLatencyOn;
	static int max_latency;
	static int min_latency;
	static int playersReady;
	static bool startGame;
	static int forTheLols;
	static bool LOL;
	static u_int frame;

	const static bool AI_PLAYER = true;
	const static bool HUMAN_PLAYER = false;

	static map<int, PlayerInfo> players;
	static priority_queue<Message> inputQueue;
	static priority_queue<Message> outputQueue;

	//private helper functions
	static int randomLatency();
	static bool allReady();
	static void resetAllReady();

	static void openHandler(int);
	static void closeHandler(int);
	static void messageHandler(int, std::string);
	static void periodicHandler();


	static void handleInputMessages();
	static void handleOutputMessages();

	static void handleLogMessageFromClient(Message);
	static void handleDirectionFromClient(Message);
	static void handleNewPlayerFromClient(Message);
	static void handlePlayerIsReadyFromClient(Message);
	static void handlePingingServerFromClient(Message);

	static void gotoXY(u_int, u_int);

	//no public constructors (singleton class)
	SnakeServer();
	SnakeServer(const SnakeServer&);
	const SnakeServer& operator=(const SnakeServer&);
};

#endif