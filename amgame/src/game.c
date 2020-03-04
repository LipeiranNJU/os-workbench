#include <game.h>

void _halt(int code);
int read_key(int**);
void update_screen(int, int**);
#define SIDE 16
static int w, h;
int lipeiran;
int min(int a, int b){
  return a > b ? b : a;
}

void move_up(int** game, int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        while (k != 0 && game[k - 1][j] < 0) {
          game[k - 1][j] = game[k][j];
          game[k][j] = -1;
          k--;
        }
      }
    }
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        if (game[k - 1][j] >=0 && game[k - 1][j] == game[k][j]) {
          game[k-1][j] += 1;
          game[k][j] = -1;
          
        }
      }
    }
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        while (k != 0 && game[k - 1][j] < 0) {
          game[k - 1][j] = game[k][j];
          game[k][j] = -1;
          k--;
        }
      }
    }
  }
}

void move_down(int** game, int size) {
  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        while (k != size -1 && game[k + 1][j] < 0 ) {
          game[k + 1][j] = game[k][j];
          game[k][j] = -1;
          k++;

        }
      }
    }
  }

  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        if (game[k + 1][j] >=0 && game[k + 1][j] == game[k][j]) {
          game[k + 1][j] += 1;
          game[k][j] = -1;
        }
      }
    }
  }

  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        while (k != size -1 && game[k + 1][j] < 0 ) {
          game[k + 1][j] = game[k][j];
          game[k][j] = -1;
          k++;

        }
      }
    }
  }
}

void move_left(int** game, int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        while (k != 0 && game[j][k - 1] < 0) {
          game[j][k - 1] = game[j][k];
          game[j][k] = -1;
          k--;
        }
      }
    }
  }


  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        if (game[j][k - 1] >=0 && game[j][k - 1] == game[j][k]) {
          game[j][k-1] += 1;
          game[j][k] = -1;
          
        }
      }
    }
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        while (k != 0 && game[j][k - 1] < 0) {
          game[j][k - 1] = game[j][k];
          game[j][k] = -1;
          k--;
        }
      }
    }
  }
}

void move_right(int** game, int size) {
  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        while (k != size -1 && game[j][k + 1] < 0 ) {
          game[j][k + 1] = game[j][k];
          game[j][k] = -1;
          k++;

        }
      }
    }
  }

  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        if (game[j][k + 1] >=0 && game[j][k + 1] == game[j][k]) {
          game[j][k + 1] += 1;
          game[j][k] = -1;
        }
      }
    }
  }

  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        while (k != size -1 && game[j][k + 1] < 0 ) {
          game[j][k + 1] = game[j][k];
          game[j][k] = -1;
          k++;
        }
      }
    }
  }
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

  int game[4][4];


  for (int i = 0; i < 4; i ++) {
    for (int j = 0; j < 4; j ++) {
        game[i][j] = 0;
    }
  }

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();

  puts("Press any key to see its key code...\n");
  while (1) {
    update_screen(read_key((int **)game), (int**) game);
  }
  return 0;
}


static void init() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
  lipeiran = min(w, h) / 100;
}

int read_key(int** game) {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  if (game[0][0] >= 0) {
    _halt(0);
  }
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode == _KEY_ESCAPE && event.keydown) {
    _halt(0);
  }
  if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
  }
  if (event.keycode == _KEY_UP && event.keydown) {
    if (game[0][0] == 1) {
      _halt(1);
    }
    move_up(game, 4);
    return 1;
  }
  if (event.keycode == _KEY_DOWN && event.keydown) {
    move_up(game, 4);
    return 1;
  }
  if (event.keycode == _KEY_LEFT && event.keydown) {
    move_up(game, 4);
    return 1;
  }
  if (event.keycode == _KEY_RIGHT && event.keydown) {
    move_up(game, 4);
    return 1;
  }
  return -2;

}

void update_screen(int update, int** game) {
  if (update == -2) {
    return;
  }

  
  init();
  for (int x = 0; x * 32 <= w; x ++) {
    for (int y = 0; y * 32 <= h; y++) {
      if (x <= 3 && y <= 3) {
        if (game[0][0] == -1) {
          _halt(0);
        }
        draw_tile(x * 32, y * 32, 32, 32, block[game[x][y] + 1]);
      }
      else {
        if ((x & 1) ^ (y & 1) ) {
          draw_tile(x * 32, y * 32, 32, 32, block[(x+y)%13]); 
        } else {
          draw_tile(x * 32, y * 32, 32, 32, block[(x+y)%13]);
        }
      }
    }
  }
  return;
}