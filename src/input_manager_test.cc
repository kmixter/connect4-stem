#include "input_manager.h"

#include <gtest/gtest.h>

class InputManagerTest : public testing::Test {
 protected:
  void SetUp() override {
  }

  InputManager in_;
};

TEST_F(InputManagerTest, Instantiate) {
}

TEST_F(InputManagerTest, GetWhenEmpty) {
	_testing_current_state = 0;
	EXPECT_FALSE(in_.Get(nullptr));
}

TEST_F(InputManagerTest, GetCurrentState) {
	_testing_current_state = (1 << kNoButtonKey);
	EXPECT_EQ(_testing_current_state, in_.GetCurrentState());
}

TEST_F(InputManagerTest, OneDownAndOnlyOne) {
	_testing_current_state = (1 << kNoButtonKey);
	in_.Poll();
	InputEvent e;
	ASSERT_TRUE(in_.Get(&e));
	EXPECT_EQ(kKeyDown, e.kind);
	EXPECT_EQ(kNoButtonKey, e.key);
	ASSERT_FALSE(in_.Get(&e));
}

TEST_F(InputManagerTest, DownAndUpEvent) {
	_testing_current_state = (1 << kColumn0Key);
	in_.Poll();
	_testing_current_state = 0;
	in_.Poll();
	InputEvent e;
	ASSERT_TRUE(in_.Get(&e));
	EXPECT_EQ(kKeyDown, e.kind);
	EXPECT_EQ(kColumn0Key, e.key);
	EXPECT_TRUE(e.IsKeyDown(kColumn0Key));
	ASSERT_TRUE(in_.Get(&e));	
	EXPECT_EQ(kKeyUp, e.kind);
	EXPECT_EQ(kColumn0Key, e.key);
	EXPECT_TRUE(e.IsKeyUp(kColumn0Key));
	ASSERT_FALSE(in_.Get(&e));	
}

TEST_F(InputManagerTest, TwoDownAtOnce) {
	_testing_current_state = (1 << kHomeSwitchKey) | (1 << kColumn5Key);
	in_.Poll();
	InputEvent e;
	ASSERT_TRUE(in_.Get(&e));
	EXPECT_EQ(kKeyDown, e.kind);
	EXPECT_EQ(kHomeSwitchKey, e.key);
	ASSERT_TRUE(in_.Get(&e));	
	EXPECT_EQ(kKeyDown, e.kind);
	EXPECT_EQ(kColumn5Key, e.key);
	ASSERT_FALSE(in_.Get(&e));	

	_testing_current_state = (1 << kColumn5Key);
	in_.Poll();

	ASSERT_TRUE(in_.Get(&e));
	EXPECT_EQ(kKeyUp, e.kind);
	EXPECT_EQ(kHomeSwitchKey, e.key);
	ASSERT_FALSE(in_.Get(&e));	
}

TEST_F(InputManagerTest, FullQueue) {

}