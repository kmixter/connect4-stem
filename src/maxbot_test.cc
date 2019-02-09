#include <memory>
#include "prng.h"
#include "maxbot.h"
#include <gtest/gtest.h>

class MaxBotTest : public testing::Test {
 protected:
  void SetUp() override {
    prng_.reset(new NotAtAllRandom(0));
    ResetBot(kRedDisc, 4);
  }

  void ResetBot(CellContents disc, int lookahead) {
    bot_.reset(new MaxBot(disc, lookahead, prng_.get()));    
  }

  Board b_;
  std::unique_ptr<MaxBot> bot_;
  std::unique_ptr<PRNG> prng_;
};

TEST_F(MaxBotTest, GetName) {
  EXPECT_STREQ("Max", bot_->GetName());
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
  EXPECT_EQ((/*hrz*/(4 + 4 + 4) + /*vert*/(2) + /*bs*/(5) + /*slash*/(2+3+3)) -
            (/*hrz*/(4 + 1) + /*vert*/(1) + /*bs*/(3) + /*slash*/(1)),
            bot_->ComputeHeuristic(&b_)); 

  // Yellow is guaranteed a loss in 3 moves. In this case, heuristic
  // fails us.
  ASSERT_TRUE(b_.SetFromString("_ _ _ Y _ _ Y\n"
                               "R _ R R Y R R\n"
                               "R _ Y Y R Y Y\n"
                               "Y R R R Y Y R\n"
                               "R Y Y Y R Y R\n"
                               "Y Y R R Y R R\n"));
  EXPECT_EQ((/*hrz*/(1) + /*vert*/(1) + /*bs*/(0) + /*sl*/(0)) -
            (/*hrz*/(4) + /*vert*/(0) + /*bs*/(0) + /*sl*/(0)),
            bot_->ComputeHeuristic(&b_));

  // Yellow is about to lose.
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ Y _ _ _\n"
                               "_ _ _ R _ _ _\n"
                               "_ _ R R R _ _\n"
                               "_ R Y R Y Y Y\n"));
  EXPECT_EQ((/*hrz*/(4 + 4) + /*vert*/(3) + /*bs*/(4) + /*slash*/(3+1+1)) -
            (/*hrz*/(4) + /*vert*/(2) + /*bs*/(3) + /*slash*/(3)),
            bot_->ComputeHeuristic(&b_));
}

TEST_F(MaxBotTest, FindNextMoveOneLookaheadEveryStep) {
  /* Heuristics for red to column 0 */
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "Y _ Y _ _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(1) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(3) + /*vert*/(3) + /*bs*/(0) + /*slash*/(3)), /* -3 */
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ Y Y _ _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(2) + /*bs*/(0) + /*slash*/(2)) -
            (/*hrz*/(3) + /*vert*/(3) + /*bs*/(0) + /*slash*/(3)), /* -3 */
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(2) + /*bs*/(0) + /*slash*/(2)) -
            (/*hrz*/(6) + /*vert*/(3) + /*bs*/(2) + /*slash*/(4)), /* -6 */
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y Y _ _ _\n"
                               "R Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(1) + /*bs*/(0) + /*slash*/(2)) -
            (/*hrz*/(4) + /*vert*/(3) + /*bs*/(2) + /*slash*/(3)), /* -6 */
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "R Y R R Y _ _\n"));
  EXPECT_EQ((/*hrz*/(0) + /*vert*/(2) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(3) + /*vert*/(3) + /*bs*/(1) + /*slash*/(2)), /* -4 */
            bot_->ComputeHeuristic(&b_));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "R Y R R _ Y _\n"));
  EXPECT_EQ((/*hrz*/(0) + /*vert*/(2) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(3) + /*vert*/(3) + /*bs*/(1) + /*slash*/(2)), /* -4 */
            bot_->ComputeHeuristic(&b_)); 
  /* Yellow will pick 2-3 for -6. */

  /* Heuristics for red to column 1 */
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ R Y _ _ _ _\n"
                               "Y Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(2) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(1) + /*vert*/(2) + /*bs*/(0) + /*slash*/(2)), /* 1 */
            bot_->ComputeHeuristic(&b_)); 

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ Y _ _ _ _ _\n"
                               "_ R Y _ _ _ _\n"
                               "_ Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(1) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(3) + /*vert*/(2) + /*bs*/(0) + /*slash*/(3)), /* -2 */
            bot_->ComputeHeuristic(&b_)); 

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "_ R Y _ _ _ _\n"
                               "_ Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(1) + /*bs*/(0) + /*slash*/(2)) -
            (/*hrz*/(3) + /*vert*/(2) + /*bs*/(2) + /*slash*/(3)), /* -3 */
            bot_->ComputeHeuristic(&b_)); 

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ R Y Y _ _ _\n"
                               "_ Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(2) + /*vert*/(1) + /*bs*/(0) + /*slash*/(3)) -
            (/*hrz*/(2) + /*vert*/(2) + /*bs*/(2) + /*slash*/(3)), /* -4 */
            bot_->ComputeHeuristic(&b_)); 

  /* Yellow will pick 2-3 for -6. */

  /* Heuristics for red to column 2 */
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "Y Y R R _ _ _\n"));
  EXPECT_EQ((/*hrz*/(5) + /*vert*/(2) + /*bs*/(2) + /*slash*/(4)) -
            (/*hrz*/(3) + /*vert*/(2) + /*bs*/(0) + /*slash*/(2)), /* 0 */
            bot_->ComputeHeuristic(&b_)); 
#if 0
  int column;

  ResetBot(kRedDisc, 1);

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "_ Y R R _ _ _\n"));
  ASSERT_TRUE(bot_->FindNextMove(&b_, &column));
  EXPECT_EQ(2, column); 
}

TEST_F(MaxBotTest, FindNextMoveNoLookaheadPickBestYellowHeuristic) {
  int column;

  ResetBot(kYellowDisc, 1);

  ASSERT_TRUE(bot_->FindNextMove(&b_, &column));
  EXPECT_EQ(3, column);

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y _ _ _ _\n"
                               "_ Y R R _ _ _\n"));
  ASSERT_TRUE(bot_->FindNextMove(&b_, &column));
  EXPECT_EQ(2, column);
#endif
}
