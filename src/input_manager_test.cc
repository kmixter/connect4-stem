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

TEST_F(InputManagerTest, PeekEmpty) {
	_testing_current_state = 0;
	in_.Poll();
	InputEvent e;
	ASSERT_FALSE(in_.Peek(&e));
}

TEST_F(InputManagerTest, PeekTwo) {
	_testing_current_state = (1 << kColumn3Key) | (1 << kColumn5Key);
	in_.Poll();
	InputEvent e;
	ASSERT_TRUE(in_.Peek(&e));
	EXPECT_TRUE(e.IsKeyDown(kColumn3Key));
	ASSERT_TRUE(in_.Peek(&e));
	EXPECT_TRUE(e.IsKeyDown(kColumn3Key));
	ASSERT_TRUE(in_.Peek(&e));
	EXPECT_TRUE(e.IsKeyDown(kColumn3Key));
	ASSERT_TRUE(in_.Get(&e));	
	EXPECT_TRUE(e.IsKeyDown(kColumn3Key));
	ASSERT_TRUE(in_.Peek(&e));
	EXPECT_TRUE(e.IsKeyDown(kColumn5Key));
	ASSERT_TRUE(in_.Get(&e));	
	EXPECT_TRUE(e.IsKeyDown(kColumn5Key));
	ASSERT_FALSE(in_.Peek(&e));	
}

TEST_F(InputManagerTest, GetFilteredFirst) {
	_testing_current_state = (1 << kColumn5Key);
	in_.Poll();
	InputEvent e;
	ASSERT_FALSE(in_.GetFiltered(kColumn0Key, &e));
	ASSERT_TRUE(in_.GetFiltered(kColumn5Key, &e));
	EXPECT_TRUE(e.IsKeyDown(kColumn5Key));
	_testing_current_state = 0;
	in_.Poll();
	ASSERT_TRUE(in_.GetFiltered(kColumn5Key, &e));
	EXPECT_TRUE(e.IsKeyUp(kColumn5Key));
	ASSERT_FALSE(in_.GetFiltered(kColumn5Key, &e));
}

TEST_F(InputManagerTest, GetFilteredMiddle) {
	_testing_current_state = (1 << kColumn5Key);
	in_.Poll();

	_testing_current_state = (1 << kColumn3Key) | (1 << kColumn5Key);
	in_.Poll();

	_testing_current_state = (1 << kColumn5Key);
	in_.Poll();

	_testing_current_state = 0;
	in_.Poll();

	// Queue is now Down(5), Down(3), Up(3), Down(5)

	InputEvent e;
	ASSERT_TRUE(in_.Peek(&e));
	EXPECT_TRUE(e.IsKeyDown(kColumn5Key));

	ASSERT_TRUE(in_.GetFiltered(kColumn3Key, &e));
	EXPECT_TRUE(e.IsKeyDown(kColumn3Key));

	ASSERT_TRUE(in_.GetFiltered(kColumn3Key, &e));
	EXPECT_TRUE(e.IsKeyUp(kColumn3Key));

	ASSERT_FALSE(in_.GetFiltered(kColumn3Key, &e));

	ASSERT_TRUE(in_.Get(&e));
	EXPECT_TRUE(e.IsKeyDown(kColumn5Key));

	ASSERT_TRUE(in_.Get(&e));
	EXPECT_TRUE(e.IsKeyUp(kColumn5Key));	

	ASSERT_FALSE(in_.Get(&e));
}

TEST_F(InputManagerTest, FullQueue) {
	for (int i = 0; i < InputManager::kQueueSize / 2; ++i) {
		_testing_current_state = (1 << kHomeSwitchKey);
		in_.Poll();
		_testing_current_state = 0;
		in_.Poll();
	}

	_testing_current_state = (1 << kColumn3Key);
	in_.Poll();

	InputEvent e;
	for (int i = 0; i < InputManager::kQueueSize / 2; ++i) {
		ASSERT_TRUE(in_.Get(&e));
		EXPECT_TRUE(e.IsKeyDown(kHomeSwitchKey));
		ASSERT_TRUE(in_.Get(&e));
		EXPECT_TRUE(e.IsKeyUp(kHomeSwitchKey));
	}

	ASSERT_FALSE(in_.Get(&e));
}