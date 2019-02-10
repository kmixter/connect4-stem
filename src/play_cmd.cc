#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "board.h"
#include "maxbot.h"
#include "prng.h"
#include "rule3bot.h"
#include "randombot.h"

enum GameResult {
  kRedWinGame,
  kYellowWinGame,
  kTieGame,
  kErrorGame
};

class CmdUser : public PlayerBot {
 public:
  CmdUser(CellContents disc) : PlayerBot("User", disc, nullptr) {}
  void FindNextMove(Board* b, Observer* o) override;
};

void CmdUser::FindNextMove(Board* b, Observer* o) {
  struct termios old, raw;
  tcgetattr(STDIN_FILENO, &old);
  cfmakeraw(&raw);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  printf("Enter column [A-G]: ");
  fflush(stdout);
  PlayerBot::Observer::State s;
  while (true) {
    int c = toupper(getchar());
    if (c == 3 || c == 4) break;
    if (c < 'A' || c > 'G') continue;
    int col = c - 'A';
    if (b->Add(col, my_disc())) {
      b->UnAdd(col);
      s.column = col;
      s.kind = PlayerBot::Observer::kMoveDone;
      o->Observe(&s);
      break;
    }
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &old);
}

PlayerBot* FindPlayerBot(CellContents disc, const char* name) {
  if (!strcasecmp(name, "random")) {
    return new RandomBot(disc, new SmallPRNG(time(nullptr)));
  }
  if (!strcasecmp(name, "rule3")) {
    return new Rule3Bot(disc, new SmallPRNG(time(nullptr)));
  }
  if (!strcasecmp(name, "max")) {
    return new MaxBot(disc, 4, new SmallPRNG(time(nullptr)));
  }
  if (!strcasecmp(name, "user")) {
    return new CmdUser(disc);
  }
  fprintf(stderr, "No bot named %s\n", name);
  exit(kErrorGame);
}

void PrintBoard(Board* b) {
  printf("%s\nA B C D E F G\n\n", b->ToString().c_str());
}

GameResult RunGame(PlayerBot** player, int* plies) {
  Board b;
  *plies = 0;
  while (!b.IsTerminal()) {
    int column;
    for (int num = int(kRedDisc); num <= int(kYellowDisc); ++num) {
      PrintBoard(&b);
      if (b.IsTerminal())
        break;
      printf("%s Player Go!\n", b.GetContentsName(CellContents(num)));
      PlayerBot* current = player[num - int(kRedDisc)];
      SimpleObserver o;
      current->FindNextMove(&b, &o);
      if (!o.success) {
        fprintf(stderr, "Find next move failed!\n");
        exit(kErrorGame);
      }
      if (!b.Add(o.column, current->my_disc())) {
        fprintf(stderr, "Error adding disc!\n");
        exit(kErrorGame);
      }
      ++*plies;
    }
  }
  PrintBoard(&b);
  int win_row, win_col, win_delta_row, win_delta_col;
  if (!b.FindAnyWin(&win_row, &win_col, &win_delta_row, &win_delta_col)) {
    printf("Tie game!\n");
    return kTieGame;
  } else {
    const char* win_desc = b.GetWinLocator();
    CellContents c = b.Get(win_row, win_col);
    printf("%s won %s in %d plies!\n", b.GetContentsName(c), win_desc, *plies);
    return c == kRedDisc ? kRedWinGame : kYellowWinGame;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3 && argc != 4) {
    printf("Usage: %s [red player] [yellow player] [opt count]\n", argv[0]);
    return kErrorGame;
  }
  PlayerBot* picked[2] = {0};

  picked[0] = FindPlayerBot(kRedDisc, argv[1]);
  picked[1] = FindPlayerBot(kYellowDisc, argv[2]);
  int count = 1;
  if (argc == 4)
    count = atoi(argv[3]);

  int red_wins = 0;
  int yellow_wins = 0;
  int tie_games = 0;
  long long total_plies = 0;

  for (int i = 0; i < count; ++i) {
    printf("###########\nGAME %d\n###########\n\n", i);
    int plies = 0;
    switch (RunGame(picked, &plies)) {
      case kRedWinGame:
        ++red_wins;
        break;
      case kYellowWinGame:
        ++yellow_wins;
        break;
      case kTieGame:
        ++tie_games;
        break;
    }
    total_plies += plies;
  }

  printf("Red: %d\n", red_wins);
  printf("Yellow: %d\n", yellow_wins);
  if (tie_games != 0)
    printf("Tie: %d\n", tie_games);
  printf("Average plies: %lld\n", total_plies / count);

  if (red_wins > yellow_wins)
    return int(kRedWinGame);
  else if (yellow_wins > red_wins)
    return int(kYellowWinGame);

  return int(kTieGame);
}