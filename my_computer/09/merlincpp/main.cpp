#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <cassert>

#if 0
#include <stdlib.h>
#endif

// const int NUM_PIPS = 9;

struct HackFn {
  struct HackInt {
    HackInt(uint16_t v) : value(v & 0x7fff) {}

    operator int() const { return value; }

    uint16_t v() const { return value; }

    HackInt operator+(const HackInt& v2) const
    {
      return HackInt(this->value + v2.v());
    }

    HackInt operator-(const HackInt& v2) const
    {
      return HackInt(this->value - v2.v());
    }

    HackInt operator*(const HackInt& v2) const
    {
      return HackInt(this->value * v2.v());
    }

    HackInt operator/(const HackInt& v2) const
    {
      return HackInt(this->value / v2.v());
    }

    HackInt mod(const HackInt& v2) const
    {
      HackInt d = *this / v2;
      return *this - (d * v2);
    }

    HackInt operator%(const HackInt& v2) const { return this->mod(v2); }

   private:
    uint16_t value;
  };

  void set_random_seed(int index, int bias);
  HackInt rand();

 private:
  uint16_t rnd_seed;
  uint16_t random_values[40] = {
      0x3add, 0x2e9b, 0x738d, 0x35f2, 0x584d, 0x2ff5, 0x18a0, 0x42af,
      0x2267, 0x4d12, 0x6ac3, 0x2844, 0x2fd3, 0x7f02, 0x1b64, 0x508e,
      0x1f53, 0x2cdb, 0x6ea2, 0x678d, 0x0a1c, 0x5adb, 0x7b28, 0x00e7,
      0x4758, 0x1104, 0x3b93, 0x3677, 0x7e8b, 0x14bb, 0x1a32, 0x77c5,
      0x2d97, 0x2b4c, 0x4a35, 0x0272, 0x291c, 0x0354, 0x0f43, 0x019a};
  static const uint16_t A = 219 & 0x7fff;
  static const uint16_t M = 32749 & 0x7fff;
  static const uint16_t Q = M / A;
  static const uint16_t R = M % A;
};

void HackFn::set_random_seed(int index, int bias)
{
  rnd_seed = (random_values[index % 40] + bias) & 0x7fff;
}

HackFn::HackInt HackFn::rand()
{
  HackFn::HackInt test(A * (rnd_seed % Q) - R * (rnd_seed / Q));

  if (test < 0) { rnd_seed = test + HackFn::HackInt(M); }
  else {
    rnd_seed = test;
  }
  return rnd_seed;
}

struct Board {
  typedef union {
    struct {
      bool p1 : 1;
      bool p2 : 1;
      bool p3 : 1;
      bool p4 : 1;

      bool p5 : 1;
      bool p6 : 1;
      bool p7 : 1;
      bool p8 : 1;

      bool p9 : 1;
    } pip;

    uint16_t pips;
  } BoardState;

  Board() { state.pips = 0; }

  BoardState state;

  void press(BoardState p);
  void press(int pip);
  void print_raw();

  static const BoardState PRESS_1;
  static const BoardState PRESS_2;
  static const BoardState PRESS_3;
  static const BoardState PRESS_4;
  static const BoardState PRESS_5;
  static const BoardState PRESS_6;
  static const BoardState PRESS_7;
  static const BoardState PRESS_8;
  static const BoardState PRESS_9;

  static const BoardState SOLVED;

 private:
  static const BoardState xor_table[];
};

void Board::press(BoardState p)
{
  state.pips = state.pips ^ p.pips;
}

void Board::press(int pip)
{
  pip--;
  assert(pip < 9 && "Invalid pip number");
  BoardState p = xor_table[pip];
  state.pips = state.pips ^ p.pips;
}

const Board::BoardState Board::xor_table[] = {
    Board::PRESS_1, Board::PRESS_2, Board::PRESS_3,
    Board::PRESS_4, Board::PRESS_5, Board::PRESS_6,
    Board::PRESS_7, Board::PRESS_8, Board::PRESS_9};

void Board::print_raw()
{
  printf("0x%03x = %d\n", state.pips, state.pips);
}

const Board::BoardState Board::PRESS_1 = {{1, 1, 0, 1, 1, 0, 0, 0, 0}};
const Board::BoardState Board::PRESS_2 = {{1, 1, 1, 0, 0, 0, 0, 0, 0}};
const Board::BoardState Board::PRESS_3 = {{0, 1, 1, 0, 1, 1, 0, 0, 0}};
const Board::BoardState Board::PRESS_4 = {{1, 0, 0, 1, 0, 0, 1, 0, 0}};
const Board::BoardState Board::PRESS_5 = {{0, 1, 0, 1, 1, 1, 0, 1, 0}};
const Board::BoardState Board::PRESS_6 = {{0, 0, 1, 0, 0, 1, 0, 0, 1}};
const Board::BoardState Board::PRESS_7 = {{0, 0, 0, 1, 1, 0, 1, 1, 0}};
const Board::BoardState Board::PRESS_8 = {{0, 0, 0, 0, 0, 0, 1, 1, 1}};
const Board::BoardState Board::PRESS_9 = {{0, 0, 0, 0, 1, 1, 0, 1, 1}};

const Board::BoardState Board::SOLVED = {{1, 1, 1, 1, 0, 1, 1, 1, 1}};

struct GameState {
  void print();
  Board board;
};

typedef struct {
  int best_pip_choice;
  int tree_depth;
} PerfectChoice;

// Define solution table
PerfectChoice perfect_choices[0x200] =
{
#include "perfect_moves.h"
};

void GameState::print()
{
  puts("");
  printf("%c %c %c\n", board.state.pip.p1 ? '@' : '.',
         board.state.pip.p2 ? '@' : '.', board.state.pip.p3 ? '@' : '.');
  printf("%c %c %c\n", board.state.pip.p4 ? '@' : '.',
         board.state.pip.p5 ? '@' : '.', board.state.pip.p6 ? '@' : '.');
  printf("%c %c %c\n", board.state.pip.p7 ? '@' : '.',
         board.state.pip.p8 ? '@' : '.', board.state.pip.p9 ? '@' : '.');
  printf("             ");
  board.print_raw();
  printf("             steps to solution: %d\n", perfect_choices[board.state.pips].tree_depth);
  puts("----------");
}

int main(void)
{
  struct timeval timer;
  uint64_t tstart;
  uint64_t tend;
  GameState game;

  gettimeofday(&timer, NULL);
  tstart = timer.tv_sec << 32 | timer.tv_usec;

#if 0
  FILE* devrandom = fopen("/dev/random", "rb");
  uint32_t seed;
  fread(&seed, sizeof(uint32_t), 1, devrandom);
  srand(seed);
#endif

  // Since Hack doesn't have any clocks or timers, we can get a seed
  // using the delay waiting for the user to enter the difficulty.
  HackFn hack;
  hack.set_random_seed(0, 0);
#if 0
  hack.set_random_seed(seed % 40, seed & 0x7fff);
#endif

  const int STEPS_PER_LEVEL_OF_DIFFICULTY = 3;

  printf("Easy(1), Medium(2), Hard(3), Random(4), Manual(5) -- Enter: ");
  int level_difficulty;
  scanf("%d", &level_difficulty);
  level_difficulty = level_difficulty > 5 ? 1 : level_difficulty;

  printf("%d\n", level_difficulty);

  gettimeofday(&timer, NULL);
  tend = timer.tv_sec << 32 | timer.tv_usec;
  hack.set_random_seed((tend - tstart) % 0x7fff, (tend - tstart) % 0x7fff);

  if (level_difficulty == 4) { game.board.state.pips = hack.rand().v() & 0x1ff; }
  else if (level_difficulty == 5) {
    int manual_state;
    printf("Enter state in hex: 0x");
    scanf("%x", &manual_state);
    game.board.state.pips = manual_state & 0x1ff;
  }
  else {
    game.board.state.pips = Board::SOLVED.pips;

    for (int i = level_difficulty * STEPS_PER_LEVEL_OF_DIFFICULTY,
             previous_pip = -1;
         i > 0; i--) {
      int pip;
      do {
        pip = hack.rand().v() % 9 + 1;
        // puts("new pip");
      } while (pip == previous_pip);
      previous_pip = pip;
      game.board.press(pip);
      printf("random pip: %d\n", pip);
    }
  }

  while (game.board.state.pips != Board::SOLVED.pips) {
    game.print();
    printf("PIP: ");
    int pip;
    scanf("%d", &pip);
    game.board.press(pip);

    if (pip == 0) break;
  }

  if (game.board.state.pips == Board::SOLVED.pips) {
    game.print();
    puts("solved!");
  }
  else {
    puts("done!");
  }

  return 0;
}
