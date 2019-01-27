#include "board.h"

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

bool Board::Add(int column, CellContents disc, int* out_row) {
  if (disc != kRedDisc && disc != kYellowDisc) return false;
  if (column < 0 || column > 6) return false;
  int top;
  for (top = 5; top >= 0; --top)
    if (contents_[top][column] != kEmpty) break;
  if (top == 5) return false;
  contents_[top + 1][column] = disc;
  if (out_row != nullptr)
  	*out_row = top + 1;
  return true;
}

bool Board::UnAdd(int column, int* out_row) {
  if (column < 0 || column > 6) return false;
  int top;
  for (top = 5; top >= 0; --top)
    if (contents_[top][column] != kEmpty) break;
  if (top < 0) return false;
  contents_[top][column] = kEmpty;
  if (out_row != nullptr)
  	*out_row = top;
  return true;
}

int Board::FindMaxStreakAt(int start_row, int start_column) const {
  // Check for maximum streak incident with (row,col) piece. If empty,
  // streak is 0. In a valid game streak should never be > 4, but we
  // return whatever we find here. If there are multiple streaks of the
  // same length, we return one arbitrarily. We check the 4 canonical
  // directions, counting in both canonical and opposite-canonical directions.
  int max_streak = 0;
  CellContents disc = Get(start_row, start_column);

  // Check if empty or invalid cell.
  if (disc != kYellowDisc && disc != kRedDisc)
  	return 0;

  for (int dr = 0; dr < 2; ++dr) {
    for (int dc = -1; dc < 2; ++dc) {
      if (dr == 0 && dc != 1) continue;
      int streak = -1;  // Start at -1 since we'll count the origin twice.
    	for (int opposite = 0; opposite < 2; ++opposite) {
		    int row = start_row;
	      int col = start_column;
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
      if (streak > max_streak)
      	max_streak = streak;
    }
  }
  return max_streak;
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

bool Board::IsTerminal(bool* is_draw) const {
  int win_row, win_col, win_delta_row, win_delta_col;
  if (FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col)) {
    *is_draw = false;
    return true;
  }
  for (int r = 0; r < 6; ++r) {
    for (int c = 0; c < 7; ++c) {
      if (contents_[r][c] == kEmpty)
        return false;
    }
  }
  *is_draw = true;
  return true;
}

// Not thread safe.
const char* Board::GetCellLocator(int row, int column) {
  static char result[3];

  if (column >= 0 && column < 7)
    result[0] = 'A' + column;
  else
    result[0] = 'X';

  if (row >= 0 && row < 6)
    result[1] = '1' + row;
  else
    result[1] = '#';

  result[2] = 0;
  return result;
}