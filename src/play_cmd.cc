#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "board.h"
#include "prng.h"
#include "r2d2bot.h"
#include "roombabot.h"

enum GameResult {
  kRedWinGame,
  kYellowWinGame,
  kTieGame,
  kErrorGame
};

class CmdUser : public PlayerBot {
 public:
  CmdUser(CellContents disc) : PlayerBot("User", disc, nullptr) {}
  bool FindNextMove(Board* b, int* column);
};

bool CmdUser::FindNextMove(Board* b, int* column) {
  struct termios old, raw;
  tcgetattr(STDIN_FILENO, &old);
  cfmakeraw(&raw);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  printf("Enter column [A-G]: ");
  fflush(stdout);
  bool valid = false;
  while (true) {
    int c = toupper(getchar());
    if (c == 3 || c == 4) break;
    if (c < 'A' || c > 'G') continue;
    int col = c - 'A';
    if (b->Add(col, my_disc())) {
      b->UnAdd(col);
      *column = col;
      valid = true;
      break;
    }
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &old);
  return valid;
}

PlayerBot* FindPlayerBot(CellContents disc, const char* name) {
  if (!strcasecmp(name, "R2D2")) {
    return new R2D2Bot(disc, new SmallPRNG(time(nullptr)));
  }
  if (!strcasecmp(name, "Roomba")) {
    return new RoombaBot(disc, new SmallPRNG(time(nullptr)));
  }
  if (!strcasecmp(name, "user")) {
    return new CmdUser(disc);
  }
  fprintf(stderr, "No bot named %s\n", name);
  exit(kErrorGame);
}

GameResult RunGame(PlayerBot** player) {
  Board b;
  while (!b.IsTerminal()) {
    int column;
    for (int num = int(kRedDisc); num <= int(kYellowDisc); ++num) {
      printf("%s\nA B C D E F G\n\n", b.ToString().c_str());
      if (b.IsTerminal())
        break;
      printf("%s Player Go!\n", b.GetContentsName(CellContents(num)));
      PlayerBot* current = player[num - int(kRedDisc)];
      if (!current->FindNextMove(&b, &column)) {
        fprintf(stderr, "Find next move failed!\n");
        exit(kErrorGame);
      }
      if (!b.Add(column, current->my_disc())) {
        fprintf(stderr, "Error adding disc!\n");
        exit(kErrorGame);
      }
    }
  }
  int win_row, win_col, win_delta_row, win_delta_col;
  if (!b.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col)) {
    printf("Tie game!\n");
    return kTieGame;
  } else {
    const char* win_desc = b.GetWinLocator();
    CellContents c = b.Get(win_row, win_col);
    printf("%s won %s!\n", b.GetContentsName(c), win_desc);
    return c == kRedDisc ? kRedWinGame : kYellowWinGame;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s [red player] [yellow player]\n", argv[0]);
    return kErrorGame;
  }
  PlayerBot* picked[2] = {0};

  picked[0] = FindPlayerBot(kRedDisc, argv[1]);
  picked[1] = FindPlayerBot(kYellowDisc, argv[2]);

  return int(RunGame(picked));
}