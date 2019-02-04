#include <memory>
#include "prng.h"
#include "roombabot.h"
#include <gtest/gtest.h>

class RoombaBotTest : public testing::Test {
 protected:
  void SetUp() override {
    prng_.reset(new NotAtAllRandom(0));
    bot_.reset(new RoombaBot(kRedDisc, prng_.get()));
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
  std::unique_ptr<RoombaBot> bot_;
  std::unique_ptr<PRNG> prng_;
};

TEST_F(RoombaBotTest, GetName) {
  EXPECT_STREQ("Roomba", bot_->GetName());
}


TEST_F(RoombaBotTest, FindNextMove) {
  SetTwoTowers();

  for (int i = 0; i < 20; ++i) {
    prng_.reset(new NotAtAllRandom(i));
    bot_->SetPRNG(prng_.get());
    int col;
    ASSERT_TRUE(bot_->FindNextMove(&b_, &col));
    int expected[] = {0, 1, 3, 5, 6};
    EXPECT_EQ(expected[i % 5], col);
  }
}