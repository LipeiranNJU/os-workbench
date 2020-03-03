#include <game.h>

void _halt(int code);
void read_key();
void update_screen();
#define SIDE 16
static int w, h;
size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size);
static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // careful! stack is limited!
  _DEV_VIDEO_FBCTRL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTRL, &event, sizeof(event));
}

// Operating system is a C program!
int main(const char *args) {
  _ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();

  puts("Press any key to see its key code...\n");
  while (1) {
    read_key();
    print_key();
    update_screen();
  }
  return 0;
}


static void init() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

void read_key() {
  puts("read\n");
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  puts("read2\n");
  if (event.keycode == _KEY_ESCAPE && event.keydown) {
    _halt(0);
  }
}

void update_screen() {
  init();
  static int bias = 0;
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1) ) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff-bias); // white
      } else {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x0+bias);
      }
    }
  }
  bias += 0x010101;
  return;
}