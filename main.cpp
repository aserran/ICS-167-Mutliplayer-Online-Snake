//Alex Deuling		94357349
//Jared Napoli		31666553
//Noel Biscocho		10062289
//Anthony Serrano	53934109

#include <stdlib.h>
#include <iostream>
#include "SnakeServer.h"


//defaults
int const ROWS = 50;
int const COLUMNS = 50;
int const TOKENS = 200;
int const AI_PLAYERS = 0;
bool const DEAD_SNAKES_TURN_TO_WALLS = false;
bool const LATENCY_ON = true;
int const MAX_LAT = 300;
int const MIN_LAT = 50;
bool const FUN = false;
int const PORT = 100;


int rows = ROWS;
int columns = COLUMNS;
int tokens = TOKENS;
int ai_players = AI_PLAYERS;
bool dead_snakes_to_walls = DEAD_SNAKES_TURN_TO_WALLS;
bool latency_on = LATENCY_ON;
int max_lat = MAX_LAT;
int min_lat = MIN_LAT;
bool fun = FUN;
int port = PORT;


void getInput();


int main(int argc, char *argv[])
{
	getInput();

	if (SnakeServer::setup(rows, columns, tokens, ai_players, dead_snakes_to_walls))
	{
		if (latency_on)
		{
			SnakeServer::setMaxLatency(max_lat);
			SnakeServer::setMinLatency(min_lat);
			SnakeServer::turnOnArtificialLatency();
		}
		if (fun)
			SnakeServer::turnOnTheLOLs();

		SnakeServer::startServer(port);
	}

	return 1;
}

void getInput()
{
	cout << "Please set rows(50): ";
	cin >> rows;
	cout << "Please set columns(50): ";
	cin >> columns;
	cout << "Please set tokens(200): ";
	cin >> tokens;
	//cout << "Please set ai players(0): ";
	//cin >> ai_players;
	cout << "Please set if dead snakes turn to walls(off) y/n: ";
	char temp;
	cin >> temp;
	switch (temp)
	{
		case 'y': dead_snakes_to_walls = true;  break;
		case 'n': dead_snakes_to_walls = false; break;
	}
	
	cout << "Please set if artificial latency is on(on) y/n: ";
	cin >> temp;
	switch (temp)
	{
		case 'y': latency_on = true;  break;
		case 'n': latency_on = false; break;
	}

	if (latency_on)
	{
		cout << "Please set max latency(300): ";
		cin >> max_lat;
		cout << "Please set min latency(50): ";
		cin >> min_lat;
	}

	cout << "Please set fun(off) y/n: ";
	cin >> temp;
	switch (temp)
	{
		case 'y': fun = true;  break;
		case 'n': fun = false; break;
	}
	cout << "Please set server port: ";
	cin >> port;
}