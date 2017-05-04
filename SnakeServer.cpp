//Alex Deuling		94357349
//Jared Napoli		31666553
//Noel Biscocho		10062289
//Anthony Serrano	53934109

#include "SnakeServer.h"

bool SnakeServer::setupDone = false;
webSocket SnakeServer::server;
Snake* SnakeServer::game = nullptr;
bool SnakeServer::artificialLatencyOn = false;
int SnakeServer::max_latency = 300;
int SnakeServer::min_latency = 50;
int SnakeServer::playersReady = 0;
bool SnakeServer::startGame = false;
int SnakeServer::forTheLols = 0;
bool SnakeServer::LOL = false;
u_int SnakeServer::frame = 0;

std::map<int, SnakeServer::PlayerInfo> SnakeServer::players;
std::priority_queue<SnakeServer::Message> SnakeServer::inputQueue;
std::priority_queue<SnakeServer::Message> SnakeServer::outputQueue;

SnakeServer::SnakeServer()
{}

SnakeServer::~SnakeServer()
{
	delete game;
	server.stopServer();
}

bool SnakeServer::setup(u_int r, u_int c, u_int t, u_int aip, bool ds)
{
	//if setup has already been done, return
	if (setupDone)
		return false;

	try
	{
		srand(static_cast<u_int>(time(NULL)));
		game = new Snake(r, c, t, aip, ds);
		server.setOpenHandler(openHandler);
		server.setCloseHandler(closeHandler);
		server.setMessageHandler(messageHandler);
		server.setPeriodicHandler(periodicHandler);
		setupDone = true;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

void SnakeServer::startServer(u_int port)
{
	server.startServer(port);
}

void SnakeServer::turnOnArtificialLatency()
{
	artificialLatencyOn = true;
}

void SnakeServer::turnOffArtificialLatency()
{
	artificialLatencyOn = false;
}

void SnakeServer::turnOnTheLOLs()
{
	LOL = true;
	forTheLols = 0;
}

void SnakeServer::turnOffTheLOLs()
{
	LOL = false;
}

void SnakeServer::setMaxLatency(u_int ml)
{
	if (ml <= min_latency)
		return;
	max_latency = ml;
}

void SnakeServer::setMinLatency(u_int ml)
{
	if (ml >= max_latency)
		return;
	min_latency = ml;
}

void SnakeServer::addWalls(std::list<Snake::Pos> posList)
{
	game->addWall(posList);
}

int SnakeServer::randomLatency()
{
	if (artificialLatencyOn)
		return (rand() % (max_latency - min_latency)) + min_latency;
	else
		return 0;
}

bool SnakeServer::allReady()
{
	if (players.size() == 0)
		return false;

	for (auto i : players)
		if (!i.second.ready)
			return false;
	return true;
}

void SnakeServer::resetAllReady()
{
	for (auto i : players)
		i.second.ready = false;
}

void SnakeServer::openHandler(int clientID)
{
	//tell all the current clients that someone has joined
	vector<int> clientIDs = server.getClientIDs();
	for (int i : clientIDs)
		if (clientIDs[i] != clientID)
		{
			if (artificialLatencyOn)
				outputQueue.push({ randomLatency(), clientID, "I#M@Someone has joined." });
			else
				server.wsSend(clientID, "I#M@Someone has joined.");
		}


	std::string currentPlayers = "";

	for (auto i : players)
		currentPlayers += "NP@" + i.second.name + "," + std::to_string(i.second.snakeNum) + "$";


	if (artificialLatencyOn)
		outputQueue.push({ randomLatency() + clock(), clientID, "I#" + currentPlayers + "M@Welcome!$DW@" + std::to_string(game->getCols()) + "$DH@" + std::to_string(game->getRows()) + "$" + game->boardToString() });
	else
		server.wsSend(clientID, "I#" + currentPlayers + "M@Welcome!$DW@" + std::to_string(game->getCols()) + "$DH@" + std::to_string(game->getRows()) + "$" + game->boardToString());

}

/* called when a client disconnects */
void SnakeServer::closeHandler(int clientID)
{
	std::string temp = "I#M@" + players[clientID].name + " has left.";

	vector<int> clientIDs = server.getClientIDs();
	for (int i : clientIDs)
		if (clientIDs[i] != clientID)
		{
			if (artificialLatencyOn)
				outputQueue.push({ randomLatency() + clock(), clientIDs[i], temp });
			else
				server.wsSend(clientIDs[i], temp);
		}

	//if a player disconnects, remove them from the game
	game->removePlayer(players[clientID].snakeNum);
	players.erase(clientID);
}

/* called when a client sends a message to the server */
void SnakeServer::messageHandler(int clientID, string message)
{
	if (artificialLatencyOn)
	{
		inputQueue.push({ randomLatency() + clock(), clientID, message });
	}
	else
	{
		Message msg = {clock(), clientID, message };
		switch (msg.msg[0])
		{
		case 'm':	handleLogMessageFromClient(msg);	break;
		case 'd':	handleDirectionFromClient(msg);		break;
		case 'n':	handleNewPlayerFromClient(msg);		break;
		case 's':	handlePlayerIsReadyFromClient(msg);	break;
		case 'p':	handlePingingServerFromClient(msg);	break;
		default:	break;
		}
	}
}

/* called once per select() loop */
void SnakeServer::periodicHandler()
{
	clock_t start = clock();
	if (artificialLatencyOn)
	{
		handleInputMessages();
		handleOutputMessages();
	}

	vector<int> clientIDs = server.getClientIDs();

	if (!startGame && playersReady > 1 && playersReady == clientIDs.size())
		startGame = true;

	if (startGame && clientIDs.size() > 1)
	{
		frame++;
		if (game->getPlayers() <= 0)
		{
			resetAllReady();
			startGame = false;
			game->reset();
			playersReady = 0;
			for (int i : clientIDs)
			{
				//multiple RS messages need to be sent in order to overflow the buffer and register the reset
				if (artificialLatencyOn)
				{
					outputQueue.push({ randomLatency() + clock(), clientIDs[i], "B" + std::to_string(frame) + "#RS@:)" });
					outputQueue.push({ randomLatency() + clock(), clientIDs[i], "B" + std::to_string(frame) + "#RS@:)" });
					outputQueue.push({ randomLatency() + clock(), clientIDs[i], "B" + std::to_string(frame) + "#RS@:)" });
				}
				else
				{
					server.wsSend(clientIDs[i], "B" + std::to_string(frame) + "#RS@:)");
					server.wsSend(clientIDs[i], "B" + std::to_string(frame) + "#RS@:)");
					server.wsSend(clientIDs[i], "B" + std::to_string(frame) + "#RS@:)");
				}

				players[i].snakeNum = game->addPlayer(HUMAN_PLAYER);
			}
		}
		else
		{
			game->update();
			
			//gotoXY(0, 0);
			//std::cout << *game;

			for (int i : clientIDs)
			{
				if (artificialLatencyOn)
				{
					outputQueue.push({ randomLatency() + clock(), clientIDs[i], "B" + std::to_string(frame) + "#" + game->getChanges() + "$" + game->tokensToString() });
				}
				else
				{
					server.wsSend(clientIDs[i], "B" + std::to_string(frame) + "#" + game->getChanges() + "$" + game->tokensToString());
				}
			}

			//LOLOLOLOLOLOLOLOLLOOLOLOLOLOLOLLLOOOOLLLOLOOLOOL
			//THE LOL LOOP
			if (LOL)
			{
				++forTheLols;
				if (forTheLols >= 20)
				{
					forTheLols = 0;
					int newSnakeNum = game->addPlayer(AI_PLAYER);
					std::string temp = "I#NP@AI_Snake" + std::to_string(newSnakeNum) + "," + std::to_string(newSnakeNum);
						for (int i : clientIDs)
							server.wsSend(clientIDs[i], temp);
				}
			}
			//END OF THE LOL LOOP
			//LOLOLOLOLOLOLOLOLLOOLOLOLOLOLOLLLOOOOLLLOLOOLOOL

			clock_t end = clock();
			auto temp = end - start;
			if (100-temp > 0)
				Sleep(100 - temp);
		}
	}
}

void SnakeServer::handleInputMessages()
{
	vector<int> clientIDs = server.getClientIDs();
	clock_t start = clock();
	while (!inputQueue.empty() && inputQueue.top().handleAtThisTime <= start)
	{
		Message message = inputQueue.top();
		inputQueue.pop();

		//if the player is no longer connected, skip this iteration
		bool found = false;
		for (int i : clientIDs)
		{
			if (i == message.clientID)
			{
				found = true;
				break;
			}
		}
		if (!found)
			continue;

		switch (message.msg[0])
		{
		case 'm':	handleLogMessageFromClient(message);	break;
		case 'd':	handleDirectionFromClient(message);		break;
		case 'n':	handleNewPlayerFromClient(message);		break;
		case 's':	handlePlayerIsReadyFromClient(message);	break;
		case 'p':	handlePingingServerFromClient(message);	break;
		default:	break;
		}
	}
}

void SnakeServer::handleOutputMessages()
{
	vector<int> clientIDs = server.getClientIDs();
	clock_t start = clock();
	while (!outputQueue.empty() && outputQueue.top().handleAtThisTime <= start)
	{
		Message message = outputQueue.top();
		outputQueue.pop();

		//if the player is no longer connected, skip this iteration
		bool found = false;
		for (int i : clientIDs)
		{
			if (i == message.clientID)
			{
				found = true;
				break;
			}
		}
		if (!found)
			continue;


		server.wsSend(message.clientID, message.msg);
	}

}

void SnakeServer::handleLogMessageFromClient(Message message)
{
	std::string temp = "I#M@" + players[message.clientID].name + " says: " + message.msg.substr(1, message.msg.size());
	
	vector<int> clientIDs = server.getClientIDs();
	for (int i : clientIDs)
		if (clientIDs[i] != message.clientID)
		{
			if (artificialLatencyOn)
				outputQueue.push({ randomLatency() + clock(), clientIDs[i], temp });
			else
				server.wsSend(clientIDs[i], temp);;
		}
}

void SnakeServer::handleDirectionFromClient(Message message)
{
	game->setPlayerDirection(players[message.clientID].snakeNum, message.msg[1]);
}

void SnakeServer::handleNewPlayerFromClient(Message message)
{
	players[message.clientID].name = message.msg.substr(1, message.msg.size());
	players[message.clientID].ready = false;
	players[message.clientID].snakeNum = game->addPlayer(HUMAN_PLAYER);
	vector<int> clientIDs = server.getClientIDs();
	for (int i : clientIDs)
	{
		if (artificialLatencyOn)
			outputQueue.push({ randomLatency() + clock(), clientIDs[i], "I#NP@" + players[message.clientID].name + "," + to_string(players[message.clientID].snakeNum)});
		else
			server.wsSend(clientIDs[i], "I#NP@" + players[message.clientID].name + "," + to_string(players[message.clientID].snakeNum));
	}
}

void SnakeServer::handlePlayerIsReadyFromClient(Message message)
{
	players[message.clientID].ready = true;
	++playersReady;
	vector<int> clientIDs = server.getClientIDs();
	for (int i : clientIDs)
	{
		if (artificialLatencyOn)
			outputQueue.push({ randomLatency() + clock(), clientIDs[i], "I#M@" + players[message.clientID].name + " is ready." });
		else
			server.wsSend(clientIDs[i], "I#M@" + players[message.clientID].name + " is ready.");
	}
}

void SnakeServer::handlePingingServerFromClient(Message message)
{
	clock_t temp = randomLatency() + clock();
	if (artificialLatencyOn)
		outputQueue.push({ temp, message.clientID, "I#P@" + to_string(temp - message.handleAtThisTime) });
	else
		server.wsSend(message.clientID, "I#P@" + to_string(temp - message.handleAtThisTime));
}

void SnakeServer::gotoXY(u_int x, u_int y)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(hStdOut, coord);
}