#include <stdio.h>
#include <string.h>

#include "board.h"
#include "r2d2bot.h"

enum {
	kRedWinExit,
	kYellowWinExit,
	kTieExit,
	kErrorExit
};


PlayerBot* FindPlayerBot(CellContents disc, const char* name) {
	if (!strcasecmp(name, "R2D2")) {
		return new R2D2Bot(disc);
	}
	fprintf(stderr, "No bot named %s\n", name);
	exit(kErrorExit);
}

int RunPlay(PlayerBot** player) {
	Board b;
	while (!b.IsTerminal()) {
		int column;
		for (int num = int(kRedDisc); num <= int(kYellowDisc); ++num) {
			printf("%s\n\n", b.ToString().c_str());
			if (b.IsTerminal())
				break;
			printf("%s Player Go!\n", b.GetContentsName(CellContents(num)));
			PlayerBot* current = player[num - int(kRedDisc)];
			if (!current->FindNextMove(&b, &column)) {
				fprintf(stderr, "Find next move failed!\n");
				exit(kErrorExit);
			}
			if (!b.Add(column, current->my_disc())) {
				fprintf(stderr, "Error adding disc!\n");
				exit(kErrorExit);
			}
		}
	}
	int win_row, win_col, win_delta_row, win_delta_col;
	if (!b.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col)) {
		printf("Tie game!\n");
		return int(kTieExit);
	} else {
		const char* win_desc = b.GetWinLocator();
		CellContents c = b.Get(win_row, win_col);
		printf("%s won %s!\n", b.GetContentsName(c), win_desc);
		return c == kRedDisc ? kRedWinExit : kYellowWinExit;
	}
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Usage: %s [red player] [yellow player]\n", argv[0]);
		return kErrorExit;
	}
	PlayerBot* picked[2] = {0};

	picked[0] = FindPlayerBot(kRedDisc, argv[1]);
	picked[1] = FindPlayerBot(kYellowDisc, argv[2]);

	return RunPlay(picked);
}