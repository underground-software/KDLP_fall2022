#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bf.h"

int main() {
#define INIT(src)                                                              \
  memset(bf, 0, sizeof(struct bf));                                            \
  memcpy(bf->program, src, sizeof(src));                                       \
  bf->program_len = sizeof(src);

#define INPUT(s)                                                               \
  memcpy(bf->input + bf->input_len, s, strlen(s));                             \
  bf->input_len += strlen(s);

  struct bf *bf = malloc(sizeof(struct bf));

  INIT(hello_world);
  assert(bf->state == STARTING);
  bf_run(bf);
  assert(bf->state == FINISHED);
  printf("%s\n", bf->output);

  INIT(three_inputs);
  bf_run(bf);
  assert(bf->state == NEED_INPUT);
  INPUT("1");
  bf_run(bf);
  assert(bf->state == NEED_INPUT);
  printf("%s\n", bf->output);
  INPUT("23");
  bf_run(bf);
  assert(bf->state == FINISHED);
  printf("%s\n", bf->output);

  INIT(encrypt);
  bf_run(bf);
  printf("%s\n", bf->output);
  assert(bf->state == NEED_INPUT);
  INPUT("hello");
  bf_run(bf);
  printf("%s\n", bf->output);
  char ciphertext[6] = {0};
  int len = strlen(bf->output);
  memcpy(ciphertext, bf->output + len - 5, 5);

  INIT(decrypt);
  bf_run(bf);
  assert(bf->state == NEED_INPUT);
  printf("%s\n", bf->output);
  INPUT(ciphertext);
  bf_run(bf);
  assert(bf->state == FINISHED);
  printf("%s\n", bf->output);

  INIT("[+++++++++++++++");
  bf_run(bf);
  assert(bf->state == INVALID);

  INIT("+++++++++++++++]");
  bf_run(bf);
  assert(bf->state == INVALID);
}
