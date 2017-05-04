#ifndef SNAKE_HPP
#define SNAKE_HPP

#include <time.h>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <list>
#include <queue>

class Snake
{
public:
	typedef unsigned short u_int;
	typedef std::pair<u_int,u_int> Pos;

	explicit Snake(u_int rows, u_int cols, u_int toks, u_int aip = 0, bool dstw = false);
	~Snake();

	u_int getRows()	   const { return ROWS; };
	u_int getCols()	   const { return COLS; };
	u_int getPlayers() const { return players.size(); };
	std::string getChanges() const { return changes; };

	std::string boardToString() const;
	std::string tokensToString() const;

	int addWall(Pos posOfWall);
	int addWall(u_int row, u_int col);
	int addWall(std::list<Pos> posList);
	int addPlayer(bool isAI);
	void removePlayer(u_int pNum);

	void setPlayerDirection(const u_int playerNumber, const char wasd);
	void setPlayerToAI(const u_int);
	void update();
	void reset();

	friend std::ostream& operator<<(std::ostream& outs, const Snake&);

private:

	enum Direction { UP, DOWN, LEFT, RIGHT };
	enum SpaceType {EMPTY = 0, TOKEN = 1, WALL = 2};

	struct Player
	{
		Pos newHead;
        std::queue<Pos> snake;
		bool dead;
        bool grow;
        bool isAI;
		Direction dir;
		Direction dirLastFrame;
	};

	const u_int ROWS;
	const u_int COLS;
	const u_int TOKENS;
	const u_int AI_PLAYERS;
	const bool DEAD_SNAKES_TO_WALLS;

	u_int** board;

	u_int tokensToAdd;
	u_int playerIDs;
	std::string changes; // used for exporting;
	std::set<Pos> tokens;
	std::set<Pos> emptySpaces;
	std::map<u_int,Player> players;




	//helper functions
	void setPlayersDirLastFrame();
	int addTokens();
	void updateNewHeads();
    void checkForNewHeadCollisions();
    void removeDeadSnakes();
    void checkNewHeadForToken();
    void updateSnakeTails();
    void checkOtherCollisions();
    void updateHeads();
    
    void getAIMoves();

	//to prevent use
	Snake();
	Snake(const Snake&);
	Snake& operator=(const Snake&);
};






















#endif //SNAKE_HPP