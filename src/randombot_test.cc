#include <memory>
#include "prng.h"
#include "randombot.h"
#include <gtest/gtest.h>

class RandomBotTest : public testing::Test {
 protected:
  void SetUp() override {
    prng_.reset(new NotAtAllRandom(0));
    bot_.reset(new RandomBot(kRedDisc, prng_.get()));
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
  std::unique_ptr<RandomBot> bot_;
  std::unique_ptr<PRNG> prng_;
};

TEST_F(RandomBotTest, GetName) {
  EXPECT_STREQ("Cupcake", bot_->GetName());
}


TEST_F(RandomBotTest, FindNextMove) {
  SetTwoTowers();

  for (int i = 0; i < 20; ++i) {
    prng_.reset(new NotAtAllRandom(i));
    bot_->SetPRNG(prng_.get());
    SimpleObserver o;
    bot_->FindNextMove(&b_, &o);
    ASSERT_TRUE(o.success);
    int expected[] = {0, 1, 3, 5, 6};
    EXPECT_EQ(expected[i % 5], o.column);
  }
}