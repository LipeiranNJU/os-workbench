#include <game.h>
#include <time.h>
void _halt(int code);
int read_key_of_mine(int game[][4]);
void update_screen(int, int[][4]);
#define SIDE 16
static int w, h;
int lipeiran;
int min(int a, int b){
  return a > b ? b : a;
}
static int I;
void my_srand(int seed) {
  I = seed;
}
int my_rand() {
  I = 1664525 * I + 1013904223;
  return I;
}
static void init();

int move_up(int game[][4], int size) {
  int valid = 0;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        while (k != 0 && game[k - 1][j] < 0) {
          game[k - 1][j] = game[k][j];
          game[k][j] = -1;
          valid = 1;
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
          valid = 1;
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
  return valid;
}

int move_down(int game[][4], int size) {
  int valid = 0;
  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        while (k != size -1 && game[k + 1][j] < 0 ) {
          game[k + 1][j] = game[k][j];
          game[k][j] = -1;
          k++;
          valid = 1;

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
          valid = 1;
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
  return valid;
}

int move_left(int game[][4], int size) {
  int valid = 0;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != 0) {
        int k = i;
        while (k != 0 && game[j][k - 1] < 0) {
          game[j][k - 1] = game[j][k];
          game[j][k] = -1;
          k--;
          valid = 1;
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
          valid = 1;
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
  return valid;
}

int move_right(int game[][4], int size) {
  int valid = 0;
  for (int i = size - 1; i >= 0; i--) {
    for (int j = size -1; j >= 0; j--) {
      if (i != size - 1) {
        int k = i;
        while (k != size -1 && game[j][k + 1] < 0 ) {
          game[j][k + 1] = game[j][k];
          game[j][k] = -1;
          k++;
          valid = 1;
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
          valid = 1;
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
  return valid;
}

typedef int color;
static color block[] = {
  0xbcd6dd, 
  0xadd9d8,
  0x9fdcd4,
  0x91e0d0,
  0x83e3cc,
  0x75e7c8,
  0x67eac4,
  0x58edc0,
  0x4af1bc,
  0x3cf4b8,
  0x2ef8b4,
  0x20fbb0,
  0x12ffac
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
  init();
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

  // puts("Press any key to see its key code...\n");
  while (1) {
    int mode = read_key_of_mine(game);
    update_screen(mode, game);
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

int read_key_of_mine(int game[][4]) {

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
  if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
  }
  if ((event.keycode == _KEY_A || event.keycode == _KEY_LEFT) && event.keydown) {
    return move_up(game, 4);
  }
  if ((event.keycode == _KEY_D || event.keycode == _KEY_RIGHT) && event.keydown) {
    return move_down(game, 4);
  }
  if ((event.keycode == _KEY_W || event.keycode == _KEY_UP) && event.keydown) {
    return move_left(game, 4);
  }
  if ((event.keycode == _KEY_S || event.keycode == _KEY_DOWN) && event.keydown) {
    return move_right(game, 4);
  }
  return 0;

}

void update_screen(int update, int game[][4]) {
  if (update == 0) {
    return;
  }



  
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if (x <= 7 && y <= 7) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, block[game[x/2][y/2] + 1]);
        // game[3][3] = 0;
      }
      else {
        // if ((x & 1) ^ (y & 1) ) {
        //   draw_tile(x * 16, y * 16, 16, 16, block[(x+y)%13]); 
        // } else {
        //   draw_tile(x * 16, y * 16, 16, 16, block[(x+y)%13]);
        // }
        ;
      }
    }
  }
  return;
}