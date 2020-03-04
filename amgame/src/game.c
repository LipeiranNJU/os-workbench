#include <game.h>

void _halt(int code);
int read_key_inamgame();
void update_screen(int);
static int SIDE;
static int w, h;
int max(int a, int b) {
  if (a >= b) 
    return a;
  else 
    return b;
  
}
typedef int color;
static color block[] = {
  0xffbcd6dd, 
  0xffadd9d8,
  0xff9fdcd4,
  0xff91e0d0,
  0xff83e3cc,
  0x0075e7c8,
  0xff67eac4,
  0xff58edc0,
  0xff4af1bc,
  0xff3cf4b8,
  0xff2ef8b4,
  0xff20fbb0,
  0xff12ffac
};


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
    update_screen(read_key_inamgame());
  }
  return 0;
}


static void init() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
  SIDE=max(w, h)/4;
}

int read_key_inamgame() {
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
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, block[(x+y)%13]); 
      } else {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, block[(x+y)%13]);
      }
    }
  }
  return;
}