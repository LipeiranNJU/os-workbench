#include <game.h>

void _halt(int code);
void read_key();

// Operating system is a C program!
int main(const char *args) {
  _ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();

  puts("Press any key to see its key code...\n");
  while (1) {
    print_key();
    read_key();
  }
  return 0;
}


void read_key() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode == _KEY_ESCAPE && event.keydown) {
    _halt(0);
  }
}
