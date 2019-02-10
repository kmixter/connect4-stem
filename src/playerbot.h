#ifndef _PLAYERBOT_H
#define _PLAYERBOT_H

#include <stdint.h>

#include "board.h"

class PRNG;

class PlayerBot {
 public:
  class Observer {
   public:
    enum Kind {
      kHeuristicDone,
      kMoveDone,
      kNoMovePossible
    };
    struct State {
      State() : kind(kNoMovePossible) {}
      Kind kind;
      // only set for kHeuristicDone
      int depth;
      uint8_t* moves;
      long heuristic;
      // only set for kMoveDone
      uint8_t column;
    };
    Observer() {}
    virtual ~Observer() {}

    // Return false to interrupt FindNextMove call and return prematurely.
    virtual bool Observe(State* s) = 0;
  };

  PlayerBot(const char* name, CellContents my_disc, PRNG* prng) : prng_(prng) {
    SetDisc(my_disc);
    name_ = name;
  }

  // Find next move for given board b. Observer may be called multiple times
  // for status updates (and to give user ability interrupt). If kMoveDone
  // is not called before returning, assume the function was interrupted.
  virtual void FindNextMove(Board* b, Observer* o) = 0;

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


class SimpleObserver : public PlayerBot::Observer {
 public:
  SimpleObserver() : column(9), success(false) {}
  bool Observe(PlayerBot::Observer::State* s) {
    if (s->kind == kMoveDone) {
      column = s->column;
      success = true;
    }
    return true;
  }
  bool success;
  int column;
};

#endif  // _PLAYERBOT_H
