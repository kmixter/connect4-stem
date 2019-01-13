#include "r2d2bot.h"

bool R2D2Bot::FindNextMove(Board* board, int* out_column) {
	int max_streak_count = 0;
	int max_streak_column = -1;
	for (auto column = 0; column < 7; ++column) {
		int row;
		if (!board->Add(column, my_disc_, &row)) 
			continue;  // This column must be full.
		auto this_streak_count = board->FindMaxStreakAt(row, column);
		if (this_streak_count > max_streak_count) {
			max_streak_count = this_streak_count;
			max_streak_column = column;
		}
		board->UnAdd(column);
		if (this_streak_count >= 4) {
			*out_column = column;
			return true;
		}

		if (board->Add(column, opponent_disc_)) {
		  auto this_streak_count = board->FindMaxStreakAt(row, column);
			board->UnAdd(column);
			if (this_streak_count >= 4) {
				*out_column = column;
				return true;
			}
		}
	}
	if (!max_streak_count) {
		// We were unable to find a single column that we could add to.
		return false;
	}
	*out_column = max_streak_column;
	return true;
}
