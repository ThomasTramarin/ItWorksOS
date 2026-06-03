#include "kernel.h"

void kmain() {

  char *video = (char *)0xB8000;

  video[0] = 'A';
  video[1] = 0xF0;

  while (1) {
  }
}