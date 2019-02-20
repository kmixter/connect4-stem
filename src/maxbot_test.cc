#include <list>
#include <memory>
#include "prng.h"
#include "maxbot.h"
#include <gtest/gtest.h>

#define M3 MaxBot::kBonusCount[2]
#define M2 MaxBot::kBonusCount[1]

class MaxBotTest : public testing::Test {
 protected:
  void SetUp() override {
    prng_.reset(new NotAtAllRandom(0));
    ResetBot(kRedDisc, 4, false);
  }

  void ResetBot(CellContents disc, int lookahead,
                bool use_lookahead) {
    bot_.reset(new MaxBot(disc, lookahead, prng_.get(),
                          use_lookahead));
  }

  Board b_;
  std::unique_ptr<MaxBot> bot_;
  std::unique_ptr<PRNG> prng_;
};

TEST_F(MaxBotTest, GetName) {
  EXPECT_STREQ("Superbot", bot_->GetName());
}

TEST_F(MaxBotTest, HeuristicNewBoard) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ R _ _ _\n"));
  EXPECT_EQ(4 + 3, bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R _ _ _ _\n"));
  EXPECT_EQ(3 + 2, bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "Y _ _ _ _ _ _\n"));
  EXPECT_EQ(-3, bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ Y _\n"));
  EXPECT_EQ(-2 - 2, bot_->ComputeHeuristic(&b_));
}

TEST_F(MaxBotTest, HeuristicSecondMove) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ Y _ _ _\n"
                               "_ _ _ R _ _ _\n"));
  EXPECT_EQ((4 + 1 + 1) - (4 + 1 + 2 + 2), bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y R _ _ _\n"));
  EXPECT_EQ((1 + 1 + 1 + 1) - (2), bot_->ComputeHeuristic(&b_));  
}

TEST_F(MaxBotTest, HeuristicSoonLosses) {
  // Yellow is guaranteed loss in 6 moves.
  ASSERT_TRUE(b_.SetFromString("_ _ _ Y _ _ _\n"
                               "_ _ _ R _ _ _\n"
                               "_ _ _ R _ _ _\n"
                               "_ _ _ R R _ _\n"
                               "_ _ R Y Y Y _\n"
                               "_ _ Y R Y R _\n"));
  EXPECT_EQ((/*hrz*/(4 + 4 + 1 + 3*M2) + /*vert*/(2) + /*bs*/(3+2*M2) + /*slash*/(2+3+2*M2+1)) -
            (/*hrz*/(4 + M3) + /*vert*/(1) + /*bs*/(2+M2) + /*slash*/(1)),
            bot_->ComputeHeuristic(&b_)); 

  // Yellow is guaranteed a loss in 3 moves. In this case, heuristic
  // fails us.
  ASSERT_TRUE(b_.SetFromString("_ _ _ Y _ _ Y\n"
                               "R _ R R Y R R\n"
                               "R _ Y Y R Y Y\n"
                               "Y R R R Y Y R\n"
                               "R Y Y Y R Y R\n"
                               "Y Y R R Y R R\n"));
  EXPECT_EQ((/*hrz*/(M3) + /*vert*/(1) + /*bs*/(0) + /*sl*/(0)) -
            (/*hrz*/(3+M2) + /*vert*/(0) + /*bs*/(0) + /*sl*/(0)),
            bot_->ComputeHeuristic(&b_));

  // Yellow is about to lose.
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ Y _ _ _\n"
                               "_ _ _ R _ _ _\n"
                               "_ _ R R R _ _\n"
                               "_ R Y R Y Y Y\n"));
  EXPECT_EQ((/*hrz*/(4 + 2*M2+2*M3) + /*vert*/(3) + /*bs*/(2*M2+2) + /*slash*/(M3+2*M2+2)) -
            (/*hrz*/(4) + /*vert*/(2) + /*bs*/(2+M2) + /*slash*/(3)),
            bot_->ComputeHeuristic(&b_));
}

TEST_F(MaxBotTest, TestHeuristicsForOneBoard) {
  /* Heuristics for red to column 0 */
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "Y _ Y _ _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(M2+1) + /*vert*/(1) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(M2+2) + /*vert*/(3) + /*bs*/(0) + /*slash*/(2+M2)),
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ Y Y _ _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(M2+1) + /*vert*/(2) + /*bs*/(0) + /*slash*/(2)) -
            (/*hrz*/(2*M2+1) + /*vert*/(2+M2) + /*bs*/(0) + /*slash*/(2+M2)),
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(M2+1) + /*vert*/(2) + /*bs*/(0) + /*slash*/(2)) -
            (/*hrz*/(6) + /*vert*/(2+M2) + /*bs*/(2) + /*slash*/(3+M2)),
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y Y _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(M2+1) + /*vert*/(1) + /*bs*/(0) + /*slash*/(2)) -
            (/*hrz*/(3*M2+1) + /*vert*/(3) + /*bs*/(2) + /*slash*/(2+M2)),
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "R Y R R Y _ _\n"));
  EXPECT_EQ((/*hrz*/(0) + /*vert*/(2) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(3) + /*vert*/(3) + /*bs*/(1) + /*slash*/(M2+1)),
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "R Y R R _ Y _\n"));
  EXPECT_EQ((/*hrz*/(0) + /*vert*/(2) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(3) + /*vert*/(3) + /*bs*/(1) + /*slash*/(M2+1)),
            bot_->ComputeHeuristic(&b_)); 

  /* Heuristics for red to column 2 */
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "Y Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(4+M2) + /*vert*/(2) + /*bs*/(2) + /*slash*/(4)) -
            (/*hrz*/(3) + /*vert*/(2) + /*bs*/(0) + /*slash*/(M2+1)),
            bot_->ComputeHeuristic(&b_)); 
}

class RecordingObserver : public SimpleObserver {
 public:
  bool Observe(PlayerBot::Observer::State* s) {
    states.push_back(*s);
    states.back().moves = new uint8_t[MaxBot::kMaxLookahead];
    memcpy(states.back().moves, s->moves, MaxBot::kMaxLookahead);
    if (s->kind == kMoveDone) {
      heuristic = s->heuristic;
    }
    return SimpleObserver::Observe(s);
  }

  PlayerBot::Observer::State* Find(int length, int* needle) {
    for (auto i = states.begin(); i != states.end(); ++i) {
      if (i->depth == length) {
        int j;
        for (j = 0; j < length; ++j) {
          if (i->moves[j] != needle[j]) break;
        }
        if (j == length) return &*i;
      }
    }
    return nullptr;
  }

  void Dump() {
    for (auto i : states) {
      for (int j = i.depth - 1; j >= 0; --j)
        printf("%d -> ", i.moves[j]);
      switch (i.kind) {
       case kHeuristicDone:
        printf("HeuristicDone ");
        break;
       case kAlphaBetaPruneDone:
        printf("AlphaBetaPruneDone ");
        break;
       case kMoveDone:
        printf("MoveDone ");
        break;
       case kNoMovePossible:
        printf("NoMovePossible ");
        break;
      }
      printf("%ld\n", i.heuristic);
    }      
  }

  int heuristic;
  std::list<PlayerBot::Observer::State> states;
};

TEST_F(MaxBotTest, TestFindNextMoveOnOneBoard) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "_ Y R R _ _ _\n"));

  RecordingObserver o;
  ResetBot(kRedDisc, 2, false);
  bot_->FindNextMove(&b_, &o);
  ASSERT_TRUE(o.success);
  EXPECT_EQ(4, o.column);
  EXPECT_EQ(7 * 7U + 1, o.states.size());
  b_.Add(4, kRedDisc);
  b_.Add(5, kYellowDisc);
  int expected = bot_->ComputeHeuristic(&b_);
  EXPECT_EQ(expected, o.heuristic);
  int search[] = {4, 5};
  auto s = o.Find(2, search);
  ASSERT_TRUE(s != nullptr);
  EXPECT_EQ(expected, s->heuristic);
}

TEST_F(MaxBotTest, TestFindNextMoveOnOneBoardWithAlphaBeta) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "_ Y R R _ _ _\n"));

  RecordingObserver o;
  ResetBot(kRedDisc, 2, true);
  bot_->FindNextMove(&b_, &o);
  ASSERT_TRUE(o.success);
  EXPECT_EQ(4, o.column);
  // There are 3 alpha-beta prunings:
  // 1) column 2 is worse than 1, discovered at 1 (prune 5)
  // 2) column 3 is worse than 1, discovered at 3 (prune 3)
  // 3) column 6 is worse than 5, discovered at 1 (prune 5)
  EXPECT_EQ(7 * 7U + 1 - 5 - 3 - 5 + 3, o.states.size());
  b_.Add(4, kRedDisc);
  b_.Add(5, kYellowDisc);
  int expected = bot_->ComputeHeuristic(&b_);
  EXPECT_EQ(expected, o.heuristic);
  int search[] = {4, 5};
  auto s = o.Find(2, search);
  ASSERT_TRUE(s != nullptr);
  EXPECT_EQ(expected, s->heuristic);
}
