#include <memory>
#include "prng.h"
#include "r2d2bot.h"
#include <gtest/gtest.h>

class R2D2Test : public testing::Test {
 protected:
  void SetUp() override {
    prng_.reset(new NotAtAllRandom(0));
    bot_.reset(new R2D2Bot(kRedDisc, prng_.get()));
  }

  bool PlayMove() {
    int col;
    SimpleObserver o;
    bot_->FindNextMove(&b_, &o);
    return o.success && b_.Add(o.column, kRedDisc);
  }

  void SetTwoTowers() {
    ASSERT_TRUE(b_.SetFromString("_ _ Y _ Y _ _\n"
                                 "_ _ R _ R _ _\n"
                                 "_ _ Y _ Y _ _\n"
                                 "_ _ R _ R _ _\n"
                                 "_ _ R _ R R _\n"
                                 "Y R Y _ Y R _\n"));
  }

  Board b_;
  std::unique_ptr<R2D2Bot> bot_;
  std::unique_ptr<PRNG> prng_;
};

TEST_F(R2D2Test, GetName) {
  EXPECT_STREQ("R2D2", bot_->GetName());
}

TEST_F(R2D2Test, OpeningMove) {
  ASSERT_TRUE(PlayMove());
  ASSERT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "R _ _ _ _ _ _\n", b_.ToString());
  b_.UnAdd(0);

  prng_.reset(new NotAtAllRandom(1));
  bot_->SetPRNG(prng_.get());
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ R _ _ _ _ _\n", b_.ToString());
  b_.UnAdd(1);

  prng_.reset(new NotAtAllRandom(6));
  bot_->SetPRNG(prng_.get());
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ R\n", b_.ToString());
  b_.UnAdd(6);

  prng_.reset(new NotAtAllRandom(10));
  bot_->SetPRNG(prng_.get());
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ R _ _ _\n", b_.ToString());
}

TEST_F(R2D2Test, TakeTheWinEvenIfCanDefendImmediateLoss) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y Y Y _ _\n"
                               "_ _ R R R Y _\n"));
  SimpleObserver o;
  bot_->FindNextMove(&b_, &o);
  EXPECT_TRUE(o.success);
  EXPECT_EQ(1, o.column);
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

TEST_F(R2D2Test, PickMaxStreak) {
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
  b_.UnAdd(2);

  prng_.reset(new NotAtAllRandom(1));
  bot_->SetPRNG(prng_.get());
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ R _ _\n"
            "_ _ R _ R _ _\n"
            "_ _ R _ R R _\n"
            "Y R Y _ Y R _\n", b_.ToString());
  b_.UnAdd(4);

  prng_.reset(new NotAtAllRandom(2));
  bot_->SetPRNG(prng_.get());
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ R _ R R _\n"
            "_ _ R _ R R _\n"
            "Y R Y _ Y R _\n", b_.ToString());
  b_.UnAdd(5);

  prng_.reset(new NotAtAllRandom(3));
  bot_->SetPRNG(prng_.get());
  ASSERT_TRUE(PlayMove());
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ R _ R _ _\n"
            "_ _ R _ R R _\n"
            "Y R Y _ Y R R\n", b_.ToString());
  b_.UnAdd(6);

  prng_.reset(new NotAtAllRandom(4));
  bot_->SetPRNG(prng_.get());
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
  SimpleObserver o;
  bot_->FindNextMove(&b_, &o);
  EXPECT_FALSE(o.success);
}