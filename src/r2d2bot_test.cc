#include <memory>
#include "r2d2bot.h"
#include <gtest/gtest.h>

class R2D2Test : public testing::Test {
 protected:
  void SetUp() override {
  	bot_.reset(new R2D2Bot(kRedDisc));
  }

  bool PlayMove() {
  	int col;
  	return bot_->FindNextMove(&b_, &col) && b_.Add(col, kRedDisc);
  }
  Board b_;
  std::unique_ptr<R2D2Bot> bot_;
};

TEST_F(R2D2Test, GetName) {
  EXPECT_STREQ("R2D2", bot_->GetName());
}

TEST_F(R2D2Test, OpeningMove) {
	ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "R _ _ _ _ _ _\n", b_.ToString());
}

TEST_F(R2D2Test, TakeTheWinEvenIfCanDefendImmediateLoss) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y Y Y _ _\n"
                               "_ _ R R R Y _\n"));
  int col;
  EXPECT_TRUE(bot_->FindNextMove(&b_, &col));
  EXPECT_EQ(1, col);
}

TEST_F(R2D2Test, DefendImmediateLossIfNoWin) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "R _ _ _ Y _ _\n"
                               "R R Y _ Y Y _\n"));
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "R _ _ _ Y _ _\n"
            "R R Y R Y Y _\n", b_.ToString());
}

TEST_F(R2D2Test, Defend3InARowEvenIfUncovered) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ R _ _ _\n"
                               "_ _ _ Y Y _ _\n"));
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ R _ _ _\n"
            "_ _ R Y Y _ _\n", b_.ToString());
}                   

TEST_F(R2D2Test, NoOtherRuleAppliesPickLeftMostWithMaxStreak) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R _ R _ _\n"
                               "_ _ R _ R R _\n"
                               "Y R Y _ Y R _\n"));
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ R _ _ _ _\n"
            "_ _ R _ R _ _\n"
            "_ _ R _ R R _\n"
            "Y R Y _ Y R _\n", b_.ToString());

}

TEST_F(R2D2Test, Draw) {
  ASSERT_TRUE(b_.SetFromString("R Y R Y R Y R\n"
                               "R Y R Y R Y R\n"
                               "R Y R Y R Y R\n"
                               "Y R Y R Y R Y\n"
                               "Y R Y R Y R Y\n"
                               "Y R Y R Y R Y\n"));
  int col;
  EXPECT_FALSE(bot_->FindNextMove(&b_, &col));
}

