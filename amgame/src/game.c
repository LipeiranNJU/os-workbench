#include <game.h>

void _halt(int code);
int read_key();
void update_screen(int);
#define SIDE 16
static int w, h;

typedef int color;

// static color block2 = 0xffbcd6dd;
// static color block4 = 0xffadd9d8;
// static color block8 = 0xff9fdcd4;
// static color block8 = 0xff91e0d0;
// static color block16 = 0xff83e3cc;
// static color block32 = 0xff75e7c8;
// static color block64 = 0xff67eac4;
// static color block128 = 0xff58edc0;
// static color block256 = 0xff4af1bc;
// static color block512 = 0xff3cf4b8;
static color block1024 = 0xff2ef8b4;
// static color block2048 = 0xff20fbb0;
// static color block4096 = 0xff12ffac;

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
    update_screen(read_key());
  }
  return 0;
}


static void init() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

int read_key() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode == _KEY_ESCAPE && event.keydown) {
    _halt(0);
  }
    if ((event.keycode != _KEY_NONE && event.keydown)) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
  }
  return event.keycode;
}

void update_screen(int bias) {
  init();
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1) ) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x110000*(bias%3)+0x001100*2+0x000011*0xf); 
      } else {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, block1024);
      }
    }
  }
  return;
}