#ifndef _CONNECT4_BOARD_H
#define _CONNECT4_BOARD_H

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

enum AddResult {
  kOk,
  kIllegal,
  kWin
};

class Board {
 public:
  Board() {
    for (auto r = 0; r < 6; ++r)
      for (auto c = 0; c < 7; ++c)
        contents_[r][c] = kEmpty;
  }

#ifdef TESTING
  string ToString() const;
  bool SetFromString(const string& s);
#endif  // TESTING

  AddResult Add(int column, CellContents disc);
  
  // Find a win. There is ambiguity about direction. Horizontal, vertical, slash, and backslash all
  // can start at either end and go either direction. We resolve this by never searching in negative
  // row direction and only negative column direction when looking in positive row direction. So
  // these cardinal directions are ((0, 1), (1, 0), (1, 1), (1, -1) and win row/col are based on
  // the first cell in that direction. 
  bool FindAnyWin(int* win_row, int* win_col, int* win_delta_row, int* win_delta_col) const;

  CellContents Get(int row, int column) const {
    if (column < 0 || column > 6) return kError;
    if (row < 0 || row > 5) return kError;
    return contents_[row][column];
  }

 private:
  // Bottom of board is row 0, top row 5.
  CellContents contents_[6][7];
};

#ifdef TESTING
string Board::ToString() const {
	string output;
  for (int r = 5; r >= 0; --r) {
    for (auto c = 0; c < 7; ++c) {
      switch(contents_[r][c]) {
       case kEmpty:
        output += '_';
        break;
       case kRedDisc:
        output += 'R';
        break;
       case kYellowDisc:
        output += 'Y';
        break;
       default:
        output += '?';
        break;
      }
      if (c != 6)
        output += ' ';
      else
      	output += '\n';
    }
  }
  return output;
}

bool Board::SetFromString(const string& s) {
	int r = 5;
	int c = 0;
	int i = 0;
	do {
		if (s[i] == 'R')
			contents_[r][c] = kRedDisc;
		else if (s[i] == 'Y')
			contents_[r][c] = kYellowDisc;
		else if (s[i] == '_')
			contents_[r][c] = kEmpty;
		else
			return false;
		++i;
		if (c == 6) {
			if (s[i] != '\n')
				return false;
			++i;
			c = 0;
			--r;
		} else {
			if (s[i] != ' ')
				return false;
			++i;
			++c;
		}
	} while (r >= 0);
	return true;
}
#endif  // TESTING

AddResult Board::Add(int column, CellContents disc) {
  if (disc != kRedDisc && disc != kYellowDisc) return kIllegal;
  if (column < 0 || column > 6) return kIllegal;
  int top;
  for (top = 5; top >= 0; --top)
    if (contents_[top][column] != kEmpty) break;
  if (top == 5) return kIllegal;
  contents_[top + 1][column] = disc;
  
  // Check for win caused by our added piece. We test the 4 canonical
  // directions, counting in both canonical and opposite-canonical directions.
  for (int dr = 0; dr < 2; ++dr) {
    for (int dc = -1; dc < 2; ++dc) {
      if (dr == 0 && dc != 1) continue;
      int streak = -1;  // Start at -1 since we'll count the origin twice.
    	for (int opposite = 0; opposite < 2; ++opposite) {
		    int row = top + 1;
	      int col = column;
	    	do {
	        if (row > 5 || col > 6) break;
	        if (row < 0 || col < 0) break;
	        if (contents_[row][col] != disc) break;
	        if (!opposite) {
		        row += dr;
	  	      col += dc;
	  	    } else {
	  	    	row -= dr;
	  	    	col -= dc;
	  	    }
	  	    ++streak;
  	    } while (true);
      }
      if (streak >= 4) return kWin;
    }
  }
  return kOk;
}

bool Board::FindAnyWin(int* win_row, int* win_col, int* win_delta_row, int* win_delta_col) const {
  for (int r = 0; r < 6; ++r) {
    for (int c = 0; c < 7; ++c) {
      if (contents_[r][c] == kEmpty) continue;
      for (int dr = 0; dr < 2; ++dr) {
        for (int dc = -1; dc < 2; ++dc) {
		      int row = r;
          int col = c;
          if (dr == 0 && dc == 0) continue;
          if (dr == 0 && dc < 0) continue;
          int streak;
          for (streak = 0; streak < 4; ++streak) {
            if (row > 5 || col > 6) break;
            if (col < 0) break;  // we never decrement row, so don't check.
            if (contents_[row][col] != contents_[r][c]) break;
            row += dr;
            col += dc;
          }
          if (streak < 4) continue;
          *win_col = c;
          *win_row = r;
          *win_delta_col = dc;
          *win_delta_row = dr;
          return true;
        }
      }
    }
  }
  return false;
}


#endif  // _CONNECT4_BOARD_H

