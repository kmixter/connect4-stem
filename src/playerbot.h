#ifndef _PLAYERBOT_H
#define _PLAYERBOT_H

#include "board.h"

class PRNG;

class PlayerBot {
 public:
  PlayerBot(const char* name, CellContents my_disc, PRNG* prng) : prng_(prng) {
    SetDisc(my_disc);
    name_ = name;
  }

  virtual bool FindNextMove(Board* board, int* column) = 0;

  const char* GetName() const {
    return name_;
  }

  void SetDisc(CellContents my_disc) {
    my_disc_ = my_disc;
    opponent_disc_ = my_disc == kRedDisc ? kYellowDisc : kRedDisc;
  }

  CellContents my_disc() const {
    return my_disc_;
  }

  void SetPRNG(PRNG* prng) {
    prng_ = prng;
  }

  PRNG* prng() { return prng_; }


 protected:
  const char* name_;
  CellContents my_disc_;
  CellContents opponent_disc_;
  PRNG* prng_;
};


#endif  // _PLAYERBOT_H
