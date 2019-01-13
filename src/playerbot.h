#ifndef _PLAYERBOT_H
#define _PLAYERBOT_H

#include "board.h"

class PlayerBot {
 public:
 	PlayerBot(CellContents my_disc);
  virtual bool FindNextMove(Board* board, int* column) = 0;

 protected:
 	CellContents my_disc_;
 	CellContents opponent_disc_;
};

inline PlayerBot::PlayerBot(CellContents my_disc) {
	my_disc_ = my_disc;
	opponent_disc_ = my_disc == kRedDisc ? kYellowDisc : kRedDisc;
}

#endif  // _PLAYERBOT_H
