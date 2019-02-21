#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "board.h"
#include "maxbot.h"
#include "prng.h"
#include "rule3bot.h"
#include "randombot.h"

uint64_t GetMicros() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}

uint64_t GetCpuMicros() {
  return clock() * 1000000 / CLOCKS_PER_SEC;
}

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
  if (!strncasecmp(name, "max", 3)) {
    bool use_alphabeta = false;
    bool use_constant_evals = false;
    name += 3;
    if (name[0] == 'e') {
      ++name;
      use_constant_evals = true;
    }
    if (name[0] == 'a') {
      ++name;
      use_alphabeta = true;
    }
    if (name[0] && !isdigit(name[0])) {
      fprintf(stderr, "Unknown max bot config at %s\n", name);
      exit(kErrorGame);
    }
    if (!use_constant_evals) {
      int depth = atoi(name);
      if (depth < 1)
        depth = 4;
      return new MaxBot(disc, depth, new SmallPRNG(time(nullptr)),
                        use_alphabeta);
    } else {
      int evals = atoi(name);
      if (evals < 100)
        evals = 100;
      return new MaxBotConstantEvals(disc, evals,
                                     new SmallPRNG(time(nullptr)),
                                     use_alphabeta);
    }
  }
  if (!strcasecmp(name, "user")) {
    return new CmdUser(disc);
  }
  fprintf(stderr, "No bot named %s\n", name);
  fprintf(stderr, "Allowed: random, rule3, max[e][a], user\n");
  exit(kErrorGame);
}

void PrintBoard(Board* b) {
  printf("%s\nA B C D E F G\n\n", b->ToString().c_str());
}

class StatsObserver : public SimpleObserver {
 public:
  bool Observe(PlayerBot::Observer::State* s) {
    if (s->kind == kHeuristicDone) {
      ++count;
      if (s->depth > max_depth)
        max_depth = s->depth;
    }
    return SimpleObserver::Observe(s);
  }
  int count = 0;
  int max_depth = 0;
};

GameResult RunGame(PlayerBot** player, int* plies) {
  Board b;
  *plies = 0;
  uint64_t accum_time[2] = {0};
  int turns[2] = {0};

  while (!b.IsTerminal()) {
    for (int num = int(kRedDisc); num <= int(kYellowDisc); ++num) {
      PrintBoard(&b);
      if (b.IsTerminal())
        break;
      printf("%s Player Go!\n", b.GetContentsName(CellContents(num)));
      PlayerBot* current = player[num - int(kRedDisc)];
      StatsObserver o;
      auto start = GetCpuMicros();
      current->FindNextMove(&b, &o);
      auto elapsed = GetCpuMicros() - start;
      printf("\nTime to find next move: %luus, %d heuristics, %d depth\n",
             elapsed, o.count, o.max_depth);
      accum_time[num - int(kRedDisc)] += elapsed;
      ++turns[num - int(kRedDisc)];
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
  for (int i = 0; i < 2; ++i) {
    printf("%s player average turn time: %luus\n",
           (i ? "Yellow" : "Red"), accum_time[i] / turns[i]);
  }
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
      case kErrorGame:
        return int(kErrorGame);
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