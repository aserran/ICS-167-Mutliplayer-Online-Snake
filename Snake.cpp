#include "Snake.hpp"

Snake::Snake(u_int rows, u_int cols, u_int toks, u_int aip, bool dstw)
	: ROWS(rows == 0 ? 30 : rows), COLS(cols == 0 ? 30 : cols), TOKENS(toks >= rows*cols ? 1 : toks), AI_PLAYERS(aip), DEAD_SNAKES_TO_WALLS(dstw), tokensToAdd(TOKENS), playerIDs(5), changes("")
{
	srand(static_cast<u_int>(time(NULL)));

	//initilize the whole board to 0;
	board = new u_int*[ROWS];
	for (u_int i = 0; i < ROWS; ++i)
	{
		board[i] = new u_int[COLS];
		for (u_int j = 0; j < COLS; ++j)
		{
			board[i][j] = EMPTY;
			emptySpaces.insert(Pos{i,j});
		}
	}
	for (u_int i = 0; i < AI_PLAYERS; ++i)
	{
		addPlayer(true);
	}
	addTokens();
}

Snake::~Snake()
{
	for (u_int i = 0; i < ROWS; ++i)
		delete[] board[i];
	delete[] board;
}

std::string Snake::boardToString() const
{
	std::string output = "";

	for (u_int i = 0; i < ROWS; ++i)
	{
		for (u_int j = 0; j < COLS; ++j)
		{
			switch (board[i][j])
			{
			case WALL:	output += "W@" + std::to_string(i) + "," + std::to_string(j) + "$"; break;
			case TOKEN:	output += "T@" + std::to_string(i) + "," + std::to_string(j) + "$"; break;
			case EMPTY: output += "E@" + std::to_string(i) + "," + std::to_string(j) + "$"; break;
			default:	output += std::to_string(board[i][j]) + "@" + std::to_string(i) + "," + std::to_string(j) + "$"; break;
			}
		}
	}

	for (auto it = players.begin(); it != players.end(); ++it)
		output += "S@" + std::to_string(it->first) + "," + std::to_string(it->second.snake.size()) + "$";

	return output.substr(0, output.length() - 1);
}

std::string Snake::tokensToString() const
{
	std::string output = "";

	for (auto i : tokens)
	{
		output += "T@" + std::to_string(i.first) + "," + std::to_string(i.second) + "$";
	}

	return output.substr(0, output.length() - 1);
}

//returns the number of walls added
int Snake::addWall(Pos pos)
{
	if (pos.first >= ROWS || pos.second >= COLS)
		return 0;

	if(board[pos.first][pos.second] == EMPTY)
	{
		board[pos.first][pos.second] = WALL;
		emptySpaces.erase(Pos{ pos.first, pos.second });
		changes += "W@" + std::to_string(pos.first) + "," + std::to_string(pos.second) + "$";
		return 1;
	}
	else if (board[pos.first][pos.second] == TOKEN)
	{
		++tokensToAdd;
		board[pos.first][pos.second] = WALL;
		tokens.erase(Pos{ pos.first, pos.second });
		changes += "W@" + std::to_string(pos.first) + "," + std::to_string(pos.second) + "$";
		return 1;
	}
	return 0;
}

//returns the number of walls added
int Snake::addWall(u_int row, u_int col)
{
	return addWall(Pos{ row, col });
}

//returns the number of walls added
int Snake::addWall(std::list<Pos> walls)
{
	int added = 0;
	for (std::list<Pos>::iterator it = walls.begin(); it != walls.end(); ++it)
		added += addWall(*it);
	return added;
}

//ADD PLAYERS FIRST THING AFTER UPDATE
//returns the player number
int Snake::addPlayer(bool isAI)
{
	int r = rand() % emptySpaces.size();

	auto it = emptySpaces.begin();
	for (; r != 0; --r) ++it;

	board[it->first][it->second] = playerIDs;

    players[playerIDs] = {*it, {}, false, false, isAI, RIGHT, RIGHT };
    players[playerIDs].snake.push(*it);

	changes += std::to_string(playerIDs) + "@" + std::to_string(it->first) + "," + std::to_string(it->second) + "$";
	emptySpaces.erase(*it);

	return playerIDs++;
}

void Snake::removePlayer(u_int pNum)
{
	if (pNum >= players.size())
		return;
	players[pNum].dead = true;
	removeDeadSnakes();
}

void Snake::setPlayerDirection(const u_int pNum, const char chDir)
{
	auto it = players.find(pNum);

	if (it == players.end())
		return;

	switch (chDir)
	{
	case 'W':
	case 'w':   it->second.dir = UP;    break;
	case 'S':
	case 's':   it->second.dir = DOWN;  break;
	case 'A':
	case 'a':   it->second.dir = LEFT;  break;
	case 'D':
	case 'd':   it->second.dir = RIGHT; break;
	default:    return;
	}
}

void Snake::setPlayerToAI(const u_int pNum)
{
	players[pNum].isAI = true;
}

void Snake::update()
{
	changes = "";
    getAIMoves();
	setPlayersDirLastFrame();
    updateNewHeads();
    checkForNewHeadCollisions();
    removeDeadSnakes();
    checkNewHeadForToken();
    updateSnakeTails();
    checkOtherCollisions();
    updateHeads();
    removeDeadSnakes();


    addTokens();
	changes = changes.substr(0, changes.length() - 1);
}

void Snake::reset()
{
	tokensToAdd = TOKENS;
	playerIDs = 5;
	changes = ""; // used for exporting;
	tokens.clear();
	emptySpaces.clear();
	players.clear();
	for (u_int i = 0; i < ROWS; ++i)
	{
		board[i] = new u_int[COLS];
		for (u_int j = 0; j < COLS; ++j)
		{
			board[i][j] = EMPTY;
			emptySpaces.insert(Pos{ i, j });
		}
	}
	for (u_int i = 0; i < AI_PLAYERS; ++i)
	{
		addPlayer(true);
	}
	addTokens();
}

std::ostream& operator<<(std::ostream& outs, const Snake& sn)
{
	typedef unsigned short u_int;
	std::string toWrite = "";

	for (u_int i = 0; i < (sn.COLS * 2) + 3; ++i)
		outs << "=";

	outs << std::endl;

	for (u_int i = 0; i < sn.ROWS; ++i)
	{
		outs << "||";
		for (u_int j = 0; j < sn.COLS; ++j)
		{
			int type = sn.board[i][j];
			switch (type)
			{
				case Snake::SpaceType::EMPTY:       toWrite = " ";		break; //space
				case Snake::SpaceType::TOKEN:       toWrite = "*";		break; //token
				case Snake::SpaceType::WALL:        toWrite = "W";		break; //wall
				default:							toWrite = "P";//player
			}
			outs << toWrite;
			if (j != sn.ROWS - 1)
				outs << " ";
		}

		outs << "||";
		outs << std::endl;
	}

	for (u_int i = 0; i < (sn.COLS * 2) + 3; ++i)
		outs << "=";

	outs << std::endl;

	return outs;
}

void Snake::setPlayersDirLastFrame()
{
	for (auto it = players.begin(); it != players.end(); ++it)
	{
		if ((it->second.dirLastFrame == UP    && it->second.dir == DOWN) ||
			(it->second.dirLastFrame == DOWN  && it->second.dir == UP) ||
			(it->second.dirLastFrame == LEFT  && it->second.dir == RIGHT) ||
			(it->second.dirLastFrame == RIGHT && it->second.dir == LEFT))
		{
			it->second.dir = it->second.dirLastFrame;
		}
		else
		{
			it->second.dirLastFrame = it->second.dir;
		}
	}
}

//returns the number of tokens added
int Snake::addTokens()
{
	if (tokens.size() == 0 && tokensToAdd == 0)
		tokensToAdd = TOKENS;


	int r = rand() % emptySpaces.size();
	int added = 0;

	while (tokensToAdd > 0 && emptySpaces.size() != 0)
	{
		auto it = emptySpaces.begin();
		for (; r != 0; --r) ++it;


		board[it->first][it->second] = TOKEN;
		tokens.insert({ it->first, it->second });
		//removed for pulsating tokens
		//changes += "T@" + std::to_string(it->first) + "," + std::to_string(it->second) + "$";
		--tokensToAdd;
		++added;
		emptySpaces.erase(*it);
		r = rand() % emptySpaces.size();
	}
	return added;
}

void Snake::updateNewHeads()
{
	for (auto it = players.begin(); it != players.end(); ++it)
	{
		switch (it->second.dir)
		{
		case UP:
		{
			if (it->second.newHead.first - 1 < 0)
				it->second.newHead.first = ROWS - 1;
			else
				--it->second.newHead.first;
			break;
		}
		case DOWN:
		{
			if (it->second.newHead.first + 1 == ROWS)
				it->second.newHead.first = 0;
			else
				++it->second.newHead.first;
			break;
		}
		case LEFT:
		{
			if (it->second.newHead.second - 1 < 0)
				it->second.newHead.second = COLS - 1;
			else
				--it->second.newHead.second;
			break;
		}
		case RIGHT:
		{
			if (it->second.newHead.second + 1 == COLS)
				it->second.newHead.second = 0;
			else
				++it->second.newHead.second;
			break;
		}
		}
	}	
}

void Snake::checkForNewHeadCollisions()
{
    //check for collisions with other snake heads
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        //if dead, continue
        if (it->second.dead)
            continue;
        
        
        for (auto it2 = players.begin(); it2 != players.end(); ++it2)
        {
            if (it2->second.dead)
                continue;
            
            //if i's and j's heads collide, kill i if score is less than or equal to j
            if (it->second.newHead == it2->second.newHead &&
                it->first != it2->first)
                if (it->second.snake.size() <= it2->second.snake.size())
                    it->second.dead = true;
        }
    }
}

void Snake::removeDeadSnakes()
{
    for (auto it = players.begin(); it != players.end();)
    {
        //if dead, remove him
        if (it->second.dead)
        {
			changes += "D@" + std::to_string(it->first) + "$";
            Pos tail;
            while(it->second.snake.size() > 0)
            {
                tail = it->second.snake.front();
                it->second.snake.pop();
				if (DEAD_SNAKES_TO_WALLS)
				{
					changes += "W@" + std::to_string(tail.first) + "," + std::to_string(tail.second) + "$";
					board[tail.first][tail.second] = WALL;
				}
				else
				{
					changes += "E@" + std::to_string(tail.first) + "," + std::to_string(tail.second) + "$";
					board[tail.first][tail.second] = EMPTY;
					emptySpaces.insert(tail);
				}
            }
            players.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void Snake::checkNewHeadForToken()
{
    //update snakes that will grow
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        //if dead, continue
        if (it->second.dead)
            continue;
        
        if (board[it->second.newHead.first][it->second.newHead.second] == TOKEN)
        {
            it->second.grow = true;
            ++tokensToAdd;
            tokens.erase(it->second.newHead);
        }
    }
}

void Snake::updateSnakeTails()
{
    //update everyones tail
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        //if dead, continue
        if (it->second.dead)
            continue;
        
        
        if (it->second.grow)
        {
            //it->second.grow = false;
        }
        else
        {
            Pos tail = it->second.snake.front();
            it->second.snake.pop();
            changes += "E@" + std::to_string(tail.first) + "," + std::to_string(tail.second) + "$";
            emptySpaces.insert(tail);
            board[tail.first][tail.second] = 0;
        }
    }
}

void Snake::checkOtherCollisions()
{
    //check for collisions with walls and snakes
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        switch (board[it->second.newHead.first][it->second.newHead.second])
        {
            case SpaceType::EMPTY:
            case SpaceType::TOKEN:
                break;
            default:
                it->second.dead = true;
        }
    }
}

void Snake::updateHeads()
{
    //update snake heads that are not dead
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        if (it->second.dead)
            continue;
        
        Pos head = it->second.newHead;
        
        board[head.first][head.second] = it->first;
		emptySpaces.erase(head);
        changes += std::to_string(it->first) + "@" + std::to_string(head.first) + "," + std::to_string(head.second) + "$";
            
        it->second.snake.push(head);
        if(it->second.grow)
        {
            it->second.grow = false;
			changes += "S@" + std::to_string(it->first) + "," + std::to_string(it->second.snake.size()) + "$";
        }
    }
}

void Snake::getAIMoves()
{
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        if (it->second.isAI)
        {
            Pos myHead = it->second.snake.back();
            u_int down = board[(myHead.first + 1 >= ROWS ? 0 : myHead.first + 1)][myHead.second];
            u_int up = board[(myHead.first - 1 < 0 ? ROWS - 1 : myHead.first - 1)][myHead.second];
            u_int left = board[myHead.first][(myHead.second - 1 < 0 ? COLS - 1 : myHead.second - 1)];
            u_int right = board[myHead.first][(myHead.second + 1 >= COLS ? 0 : myHead.second + 1)];
            
            if (down == TOKEN)
            {
                it->second.dir =  DOWN;
                return;
            }
            if (up == TOKEN)
            {
                it->second.dir = UP;
                return;
            }
            if (left == TOKEN)
            {
                it->second.dir = LEFT;
                return;
            }
            if (right == TOKEN)
            {
                it->second.dir = RIGHT;
                return;
            }
            
            
            
            std::set<Direction> randomDir = { UP, DOWN, RIGHT, LEFT };
            
            if (down != EMPTY)
                randomDir.erase(DOWN);
            if (up != EMPTY)
                randomDir.erase(UP);
            if (left != EMPTY)
                randomDir.erase(LEFT);
            if (right != EMPTY)
                randomDir.erase(RIGHT);
            
            if (randomDir.size() != 0)
                it->second.dir = *randomDir.begin();
        }
    }
}