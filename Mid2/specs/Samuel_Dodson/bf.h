#ifndef BF_H
#define BF_H
#include <stdlib.h>

enum bf_state {
  STARTING = 0, // The initial state of a new bf struct. bf_run hasn't been called yet.
  NEED_INPUT,   // The interpreter is on ',' but has no input to read
  FINISHED,     // The interpreter finished the program normally
  INVALID,      // The interpreter aborted an erroneous program
};

// This can be created by allocating and filling with 0
struct bf {
  char program[1024];  // Program source code
  char memory[1024];   // Program memory
  char output[1024];   // Program produced output (cumulative)
  char input[1024];    // User-provided input (cumulative)
  size_t program_i;    // Index into source code
  size_t memory_i;     // Index into memory AKA the data pointer
  size_t input_i;      // Index into input, how much of it we have used
  size_t program_len;  // Length of the source code
  size_t output_len;   // Length of output (cumulative)
  size_t input_len;    // Length of input (cumulative)
  enum bf_state state; // The current state of the program
};

static void bf_run(struct bf *bf) {
  char c;
  size_t depth;
  int iters = 0;

  if (bf->state == FINISHED || bf->state == INVALID)
    return;

  for (;; iters++) {
    if (bf->program_i == bf->program_len) {
      bf->state = FINISHED;
      return;
    }

    if (iters > 10000) {
      bf->state = INVALID;
      return;
    }

    c = bf->program[bf->program_i++];
    switch (c) {
    case '>':
      bf->memory_i++;
      if (bf->memory_i > 1024)
        bf->memory_i = 0;
      break;
    case '<':
      bf->memory_i--;
      if (bf->memory_i > 1024)
        bf->memory_i = 1024;
      break;
    case '+':
      bf->memory[bf->memory_i]++;
      break;
    case '-':
      bf->memory[bf->memory_i]--;
      break;
    case '.':
      bf->output[bf->output_len++] = bf->memory[bf->memory_i];
      break;
    case ',':
      if (bf->input_i == bf->input_len) {
        bf->program_i--;
        bf->state = NEED_INPUT;
        return;
      }
      bf->memory[bf->memory_i] = bf->input[bf->input_i++];
      break;
    case '[':
      if (bf->memory[bf->memory_i] != 0)
        continue;
      depth = 1;
      while (depth > 0) {
        switch (bf->program[bf->program_i++]) {
        case '[':
          depth++;
          break;
        case ']':
          depth--;
          break;
        default:
          if (bf->program_i >= 1024) {
            bf->state = INVALID;
            return;
          }
          break;
        }
      }
      break;
    case ']':
      if (bf->memory[bf->memory_i] == 0)
        continue;
      bf->program_i--;
      depth = 1;
      while (depth > 0) {
        c = bf->program[--bf->program_i];
        switch (c) {
        case '[':
          depth--;
          break;
        case ']':
          depth++;
          break;
        default:
          if (bf->program_i >= 1024) {
            bf->state = INVALID;
            return;
          }
          break;
        }
      }
      bf->program_i++;
      break;
    default:
      break;
    }
  }
}

static const char hello_world[] =
    // Print message
    "++++++++[>++++++++<-]>+ +++++++. <+++++++[>++++<-]>+. +++++++.. +++."
    ">+++++++++++[>++++<-]>. ------------. <<++++++++. --------. +++. ------. "
    "--------. >>+.";

static const char three_inputs[] =
    // Take an input and print it back out, three times.
    ",.,.,.";

static const char encrypt[] =
    // Print message
    "-[------->+<]>.+[--->+<]>.++.+++++.-.[---->+<]>+++.++[->+++<]>.+++.[->++++"
    "++<]>.+[->+++<]>.--[--->+<]>-.++[--->++<]>.-------.[--->+<]>---..+++[->+++"
    "<]>.+++++++++++++.+.>++++++++++."
    // Encrypt input and print
    ",+++. ,---. ,-----. ,+++++. ,++++++++++.";

static const char decrypt[] =
    // Print message
    "-[------->+<]>.+[--->+<]>.++.+++++.-.[---->+<]>+++.+[->+++<]>.++++++.+++++"
    "++.--------.---.+++++++++++++.++.+++[->+++<]>.[--->+<]>+.----.>++++++++++."
    // Decrypt input and print
    ",---. ,+++. ,+++++. ,-----. ,----------.";

#endif
