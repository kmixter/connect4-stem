#ifndef _PLAYERBOT_H
#define _PLAYERBOT_H

#include "board.h"

class PlayerBot {
 public:
  PlayerBot(const char* name, CellContents my_disc) {
    my_disc_ = my_disc;
    opponent_disc_ = my_disc == kRedDisc ? kYellowDisc : kRedDisc;
    name_ = name;
  }

  virtual bool FindNextMove(Board* board, int* column) = 0;

  const char* GetName() const {
    return name_;
  }

 protected:
  const char* name_;
  CellContents my_disc_;
  CellContents opponent_disc_;
};


#endif  // _PLAYERBOT_H
