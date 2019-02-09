#ifndef _ROOMBABOT_H
#define _ROOMBABOT_H

#include "playerbot.h"

class RoombaBot : public PlayerBot {
 public:
  RoombaBot(CellContents disc, PRNG* prng) : PlayerBot("Roomba", disc, prng) {}

  void FindNextMove(Board* board, Observer* o) override;
};

#endif  // _ROOMBABOT_H10
