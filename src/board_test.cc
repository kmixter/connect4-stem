#include "board.h"
#include <gtest/gtest.h>

class BoardTest : public testing::Test {
 protected:
  void SetUp() override {
  }

  Board b_;
};

TEST_F(BoardTest, Instantiate) {
}

TEST_F(BoardTest, InitialState) {
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n", b_.ToString());
}

TEST_F(BoardTest, GetCountEmpty) {
  EXPECT_EQ(0, b_.GetCount());
}

TEST_F(BoardTest, ErrorRequests) {
  EXPECT_EQ((int)kError, (int)b_.Get(-1, 0));
  EXPECT_EQ((int)kError, (int)b_.Get(0, -1));
  EXPECT_EQ((int)kError, (int)b_.Get(6, 0));
  EXPECT_EQ((int)kError, (int)b_.Get(0, 7));
}

TEST_F(BoardTest, GetCellLocator) {
  EXPECT_STREQ("A#", b_.GetCellLocator(-1, 0));
  EXPECT_STREQ("A#", b_.GetCellLocator(6, 0));
  EXPECT_STREQ("X1", b_.GetCellLocator(0, -1));
  EXPECT_STREQ("X6", b_.GetCellLocator(5, 7));
  EXPECT_STREQ("C3", b_.GetCellLocator(2, 2));
  EXPECT_STREQ("G1", b_.GetCellLocator(0, 6));
  EXPECT_STREQ("A6", b_.GetCellLocator(5, 0));
  EXPECT_STREQ("G6", b_.GetCellLocator(5, 6));
}

TEST_F(BoardTest, AddOneRedOkWithNoOutRow) {
  EXPECT_TRUE(b_.Add(2, kRedDisc));
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ R _ _ _ _\n", b_.ToString());
}

TEST_F(BoardTest, AddOneYellowOkWithOutRow) {
  int row;
  EXPECT_TRUE(b_.Add(3, kYellowDisc, &row));
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ Y _ _ _\n", b_.ToString());
  EXPECT_EQ(0, row);
  EXPECT_EQ(1, b_.GetCount());
}

TEST_F(BoardTest, StackTest) {
  int row;
  EXPECT_TRUE(b_.Add(4, kRedDisc));
  EXPECT_TRUE(b_.Add(4, kYellowDisc));
  EXPECT_TRUE(b_.Add(4, kRedDisc, &row));

  EXPECT_EQ(2, row);

  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ R _ _\n"
            "_ _ _ _ Y _ _\n"
            "_ _ _ _ R _ _\n", b_.ToString());

  EXPECT_TRUE(b_.Add(4, kYellowDisc));
  EXPECT_TRUE(b_.Add(4, kRedDisc));
  EXPECT_TRUE(b_.Add(4, kYellowDisc));

  EXPECT_EQ("_ _ _ _ Y _ _\n"
            "_ _ _ _ R _ _\n"
            "_ _ _ _ Y _ _\n"
            "_ _ _ _ R _ _\n"
            "_ _ _ _ Y _ _\n"
            "_ _ _ _ R _ _\n", b_.ToString());

  EXPECT_FALSE(b_.Add(4, kRedDisc));

  EXPECT_EQ("_ _ _ _ Y _ _\n"
            "_ _ _ _ R _ _\n"
            "_ _ _ _ Y _ _\n"
            "_ _ _ _ R _ _\n"
            "_ _ _ _ Y _ _\n"
            "_ _ _ _ R _ _\n", b_.ToString());
}

TEST_F(BoardTest, SequenceOfAddsAlongBottom) {
  EXPECT_TRUE(b_.Add(0, kRedDisc));
  EXPECT_TRUE(b_.Add(1, kYellowDisc));
  EXPECT_TRUE(b_.Add(2, kRedDisc));
  EXPECT_TRUE(b_.Add(3, kYellowDisc));
  EXPECT_TRUE(b_.Add(4, kRedDisc));
  EXPECT_TRUE(b_.Add(5, kYellowDisc));
  EXPECT_TRUE(b_.Add(6, kRedDisc));
  EXPECT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "R Y R Y R Y R\n", b_.ToString());
}

TEST_F(BoardTest, UnAdd) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R Y _ _ _\n"
                               "_ _ Y R _ _ _\n"
                               "_ _ R Y _ _ _\n"));
  EXPECT_FALSE(b_.UnAdd(0));
  EXPECT_FALSE(b_.UnAdd(1));
  ASSERT_TRUE(b_.UnAdd(2));
  ASSERT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ Y _ _ _\n"
            "_ _ Y R _ _ _\n"
            "_ _ R Y _ _ _\n", b_.ToString());

  ASSERT_TRUE(b_.UnAdd(3));
  ASSERT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ Y R _ _ _\n"
            "_ _ R Y _ _ _\n", b_.ToString());

  ASSERT_TRUE(b_.UnAdd(2));
  ASSERT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ R _ _ _\n"
            "_ _ R Y _ _ _\n", b_.ToString());

  ASSERT_TRUE(b_.UnAdd(2));
  ASSERT_EQ("_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ _ _ _ _\n"
            "_ _ _ R _ _ _\n"
            "_ _ _ Y _ _ _\n", b_.ToString());

  EXPECT_FALSE(b_.UnAdd(2));
}

TEST_F(BoardTest, FirstWin) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R Y _ _ _\n"
                               "_ _ R Y _ _ _\n"
                               "_ _ R Y _ _ _\n"));
  EXPECT_TRUE(b_.Add(2, kRedDisc));
  int win_row, win_col, win_delta_row, win_delta_col;
  ASSERT_TRUE(b_.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col));
  EXPECT_EQ(0, win_row);
  EXPECT_EQ(2, win_col);
  EXPECT_EQ(1, win_delta_row);
  EXPECT_EQ(0, win_delta_col);
  EXPECT_STREQ("C1-C4", b_.GetWinLocator());
  bool is_draw = true;
  ASSERT_TRUE(b_.IsTerminal(&is_draw));
  EXPECT_FALSE(is_draw);
}

TEST_F(BoardTest, HorizontalWin) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ Y Y Y _ _ R\n"
                               "_ R R R Y _ R\n"));
  EXPECT_TRUE(b_.Add(4, kYellowDisc));
  int win_row, win_col, win_delta_row, win_delta_col;
  ASSERT_TRUE(b_.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col));
  EXPECT_EQ(1, win_row);
  EXPECT_EQ(1, win_col);
  EXPECT_EQ(0, win_delta_row);
  EXPECT_EQ(1, win_delta_col);
  EXPECT_STREQ("B2-E2", b_.GetWinLocator());
}

TEST_F(BoardTest, SlashWin) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ R Y _ _\n"
                               "_ _ R R R _ _\n"
                               "_ R Y Y Y _ Y\n"));
  EXPECT_TRUE(b_.Add(4, kRedDisc));
  int win_row, win_col, win_delta_row, win_delta_col;
  ASSERT_TRUE(b_.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col));
  EXPECT_EQ(0, win_row);
  EXPECT_EQ(1, win_col);
  EXPECT_EQ(1, win_delta_row);
  EXPECT_EQ(1, win_delta_col);
  EXPECT_STREQ("B1-E4", b_.GetWinLocator());
}

TEST_F(BoardTest, BackslashWin) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ Y R Y _ _\n"
                               "_ _ R R R _ _\n"
                               "_ R Y Y Y R Y\n"));
  EXPECT_TRUE(b_.Add(2, kRedDisc));
  int win_row, win_col, win_delta_row, win_delta_col;
  ASSERT_TRUE(b_.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col));
  EXPECT_EQ(0, win_row);
  EXPECT_EQ(5, win_col);
  EXPECT_EQ(1, win_delta_row);
  EXPECT_EQ(-1, win_delta_col);
  EXPECT_STREQ("F1-C4", b_.GetWinLocator());
}

TEST_F(BoardTest, MiddleOf3DoesntWin) {
  EXPECT_TRUE(b_.Add(3, kRedDisc));
  EXPECT_TRUE(b_.Add(1, kRedDisc));
  EXPECT_TRUE(b_.Add(2, kRedDisc));
  int win_row, win_col, win_delta_row, win_delta_col;
  EXPECT_FALSE(b_.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col));
  EXPECT_STREQ("None", b_.GetWinLocator());
}

TEST_F(BoardTest, MiddleWin) {
  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R _ _ _ _\n"
                               "_ _ Y R _ _ _\n"
                               "_ _ R Y _ _ _\n"
                               "_ Y Y R Y R R\n"));
  EXPECT_TRUE(b_.Add(4, kRedDisc));
  int win_row, win_col, win_delta_row, win_delta_col;
  ASSERT_TRUE(b_.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col));
  EXPECT_EQ(0, win_row);
  EXPECT_EQ(5, win_col);
  EXPECT_EQ(1, win_delta_row);
  EXPECT_EQ(-1, win_delta_col);
}

TEST_F(BoardTest, IsTerminalDraw) {
  bool is_draw;
  ASSERT_FALSE(b_.IsTerminal(&is_draw));

  ASSERT_TRUE(b_.SetFromString("R Y R Y R Y _\n"
                               "R Y R Y R Y R\n"
                               "R Y R Y R Y R\n"
                               "Y R Y R Y R Y\n"
                               "Y R Y R Y R Y\n"
                               "Y R Y R Y R Y\n"));
  EXPECT_EQ(41, b_.GetCount());

  ASSERT_FALSE(b_.IsTerminal(&is_draw));

  ASSERT_TRUE(b_.Add(6, kRedDisc));

  ASSERT_TRUE(b_.IsTerminal(&is_draw));
  EXPECT_TRUE(is_draw);
}

const char* ExpandBitmap(const uint8_t* bits) {
  static char result[28 * 16 + 1] = {0};
  char* p = result;

  for (int choff = 4; choff >= 0; choff -= 4) {
    for (int line = 0; line < 8; ++line) {
      for (int lch = 0; lch < 4; ++lch) {
        int ch = choff + lch;
        for (int b = 4; b >= 0; --b) {
          *p = bits[8 * ch + line] & (1 << b) ? '.' : ' ';
          ++p;
        }
        if (lch % 4 != 3) {
          *p++ = ' ';
          *p++ = ' ';
        } else {
          *p++ = '\n';
        }
      }
    }
  }
  *p++ = 0;
  return result;
}

TEST_F(BoardTest, GetBitmap) {
  EXPECT_STREQ("                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"

               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n",
               ExpandBitmap(b_.GetBitmap()));

  ASSERT_TRUE(b_.SetFromString("R R R R R R R\n"
                               "R R R R R R R\n"
                               "R R R R R R R\n"
                               "R R R R R R R\n"
                               "R R R R R R R\n"
                               "R R R R R R R\n"));

  EXPECT_STREQ(".. ..  .. ..  .. ..  ..   \n"
               ".. ..  .. ..  .. ..  ..   \n"
               "                          \n"
               ".. ..  .. ..  .. ..  ..   \n"
               ".. ..  .. ..  .. ..  ..   \n"
               "                          \n"
               ".. ..  .. ..  .. ..  ..   \n"
               ".. ..  .. ..  .. ..  ..   \n"

               ".. ..  .. ..  .. ..  ..   \n"
               ".. ..  .. ..  .. ..  ..   \n"
               "                          \n"
               ".. ..  .. ..  .. ..  ..   \n"
               ".. ..  .. ..  .. ..  ..   \n"
               "                          \n"
               ".. ..  .. ..  .. ..  ..   \n"
               ".. ..  .. ..  .. ..  ..   \n",
               ExpandBitmap(b_.GetBitmap()));

  ASSERT_TRUE(b_.SetFromString("_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ _ _ _ _ _\n"
                               "_ _ R Y _ _ _\n"
                               "_ _ R Y _ _ _\n"
                               "_ _ R Y _ _ _\n"));

  EXPECT_STREQ("                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"
               "                          \n"

               "       .. ..              \n"
               "       ..  .              \n"
               "                          \n"
               "       .. ..              \n"
               "       ..  .              \n"
               "                          \n"
               "       .. ..              \n"
               "       ..  .              \n",
               ExpandBitmap(b_.GetBitmap()));

  ASSERT_TRUE(b_.SetFromString("_ _ R _ _ _ _\n"
                               "_ _ R _ _ _ _\n"
                               "_ _ R _ _ _ _\n"
                               "_ _ Y R _ _ _\n"
                               "_ _ R Y _ _ _\n"
                               "_ Y Y R Y R R\n"));

  EXPECT_STREQ("       ..                 \n"
               "       ..                 \n"
               "                          \n"
               "       ..                 \n"
               "       ..                 \n"
               "                          \n"
               "       ..                 \n"
               "       ..                 \n"

               "       .. ..              \n"
               "        . ..              \n"
               "                          \n"
               "       .. ..              \n"
               "       ..  .              \n"
               "                          \n"
               "   ..  .. ..  .. ..  ..   \n"
               "    .   . ..   . ..  ..   \n",
               ExpandBitmap(b_.GetBitmap()));
}
