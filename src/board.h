#ifndef _CONNECT4_BOARD_H
#define _CONNECT4_BOARD_H

#include <stdint.h>

#ifdef TESTING
#include <string>
using std::string;
#endif  // TESTING

enum CellContents {
  kEmpty,
  kRedDisc,
  kYellowDisc,
  kError
};

class Board {
 public:
  Board();

#ifdef TESTING
  string ToString() const;
  bool SetFromString(const string& s);
#endif  // TESTING

  bool Add(int column, CellContents disc, int* row = nullptr);
  bool UnAdd(int column);
  int FindMaxStreakAt(int start_row, int start_column) const;

  // Find a win. There is ambiguity about direction. Horizontal, vertical, slash, and backslash all
  // can start at either end and go either direction. We resolve this by never searching in negative
  // row direction and only negative column direction when looking in positive row direction. So
  // these cardinal directions are ((0, 1), (1, 0), (1, 1), (1, -1) and win row/col are based on
  // the first cell in that direction. 
  bool FindAnyWin(int* win_row, int* win_col, int* win_delta_row, int* win_delta_col) const;

  bool IsTerminal(bool* is_draw = nullptr) const;

  CellContents Get(int row, int column) const {
    if (column < 0 || column > 6) return kError;
    if (row < 0 || row > 5) return kError;
    return contents_[row][column];
  }

  static CellContents GetOpposite(CellContents c) {
    if (c == kRedDisc) return kYellowDisc;
    if (c == kYellowDisc) return kRedDisc;
    return kError;
  }

  uint8_t* GetBitmap() const;

#ifdef TESTING
  static const char* GetContentsName(CellContents disc);
#endif

  static const char* GetCellLocator(int row, int column);

  const char* GetWinLocator();

  // Bottom of board is row 0, top row 5.
  CellContents contents_[6][7];
  int8_t top_contents_[7];
};

#endif  // _CONNECT4_BOARD_H

