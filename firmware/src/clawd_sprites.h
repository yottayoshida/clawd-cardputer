#pragma once
#include <stdint.h>

namespace clawd {

constexpr uint16_t COLOR_MAIN =
    ((217 >> 3) << 11) | ((119 >> 2) << 5) | (87 >> 3);  // #D97757

// 16×10 quadrant grid extracted from terminal screenshot pixel data.
// Each cell = QW×QH pixels on the LCD.
constexpr int GRID_COLS = 16;
constexpr int GRID_ROWS = 10;
constexpr int QW = 6;
constexpr int QH = 7;
constexpr int TOTAL_W = GRID_COLS * QW;  // 96
constexpr int TOTAL_H = GRID_ROWS * QH;  // 70

// IDLE shape — row-major, 1=filled 0=empty
constexpr uint8_t SHAPE_IDLE[GRID_ROWS][GRID_COLS] = {
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // stalks
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},  // head + eyes (tall rect holes)
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // head + ears (full width)
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // body
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},  // legs
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// BLINK — eyes closed (fill in the eye holes)
constexpr uint8_t SHAPE_BLINK[GRID_ROWS][GRID_COLS] = {
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // eyes filled
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// HAPPY — legs spread outward
constexpr uint8_t SHAPE_HAPPY[GRID_ROWS][GRID_COLS] = {
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},  // legs wider
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},
};

// SURPRISED — wider stalks + bigger eyes
constexpr uint8_t SHAPE_SURPRISED[GRID_ROWS][GRID_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // stalks full width
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},  // wider eyes (2 quad cols)
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// SLEEPY — droopy stalks + thin eyes
constexpr uint8_t SHAPE_SLEEPY[GRID_ROWS][GRID_COLS] = {
    {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},  // shorter stalks
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // eyes closed (thin line)
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},  // only bottom row has eye holes
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// EXCITED — stalks + ears both full, legs wide
constexpr uint8_t SHAPE_EXCITED[GRID_ROWS][GRID_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // stalks full
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},  // legs wider
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},
};

// SLEEPING — squished flat, melted down
constexpr uint8_t SHAPE_SLEEPING[GRID_ROWS][GRID_COLS] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},  // small bump
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // spreading
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},  // wider
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // full width flat
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0},  // tiny feet poking out
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

enum Expression : uint8_t {
  EXPR_IDLE = 0,
  EXPR_BLINK,
  EXPR_HAPPY,
  EXPR_SURPRISED,
  EXPR_SLEEPY,
  EXPR_EXCITED,
  EXPR_SLEEPING,
  EXPR_COUNT,
};

inline const uint8_t (*shapeFor(Expression e))[GRID_COLS] {
  switch (e) {
    case EXPR_BLINK:     return SHAPE_BLINK;
    case EXPR_HAPPY:     return SHAPE_HAPPY;
    case EXPR_SURPRISED: return SHAPE_SURPRISED;
    case EXPR_SLEEPY:    return SHAPE_SLEEPY;
    case EXPR_EXCITED:   return SHAPE_EXCITED;
    case EXPR_SLEEPING:  return SHAPE_SLEEPING;
    default:             return SHAPE_IDLE;
  }
}

}  // namespace clawd
