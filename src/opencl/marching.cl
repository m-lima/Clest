//_____________________________________________________________________________
// Marching cubes from grid
//
// #define CONST_ISO 0
// #define CONST_SIZE_X 0
// #define CONST_SIZE_Y 0
// #define CONST_SIZE_Z 0
// #define CONST_SIZE_YZ 0
//_____________________________________________________________________________

constant char2 cases[256] = {
  /*   0:                          */{ 0, -1 },
  /*   1: 0,                       */{ 1,  0 },
  /*   2:    1,                    */{ 1,  1 },
  /*   3: 0, 1,                    */{ 2,  0 },
  /*   4:       2,                 */{ 1,  2 },
  /*   5: 0,    2,                 */{ 3,  0 },
  /*   6:    1, 2,                 */{ 2,  3 },
  /*   7: 0, 1, 2,                 */{ 5,  0 },
  /*   8:          3,              */{ 1,  3 },
  /*   9: 0,       3,              */{ 2,  1 },
  /*  10:    1,    3,              */{ 3,  3 },
  /*  11: 0, 1,    3,              */{ 5,  1 },
  /*  12:       2, 3,              */{ 2,  5 },
  /*  13: 0,    2, 3,              */{ 5,  4 },
  /*  14:    1, 2, 3,              */{ 5,  9 },
  /*  15: 0, 1, 2, 3,              */{ 8,  0 },
  /*  16:             4,           */{ 1,  4 },
  /*  17: 0,          4,           */{ 2,  2 },
  /*  18:    1,       4,           */{ 3,  4 },
  /*  19: 0, 1,       4,           */{ 5,  2 },
  /*  20:       2,    4,           */{ 4,  2 },
  /*  21: 0,    2,    4,           */{ 6,  2 },
  /*  22:    1, 2,    4,           */{ 6,  9 },
  /*  23: 0, 1, 2,    4,           */{ 11,  0 },
  /*  24:          3, 4,           */{ 3,  8 },
  /*  25: 0,       3, 4,           */{ 5,  5 },
  /*  26:    1,    3, 4,           */{ 7,  3 },
  /*  27: 0, 1,    3, 4,           */{ 9,  1 },
  /*  28:       2, 3, 4,           */{ 6, 16 },
  /*  29: 0,    2, 3, 4,           */{ 14,  3 },
  /*  30:    1, 2, 3, 4,           */{ 12, 12 },
  /*  31: 0, 1, 2, 3, 4,           */{ 5, 24 },
  /*  32:                5,        */{ 1,  5 },
  /*  33: 0,             5,        */{ 3,  1 },
  /*  34:    1,          5,        */{ 2,  4 },
  /*  35: 0, 1,          5,        */{ 5,  3 },
  /*  36:       2,       5,        */{ 3,  6 },
  /*  37: 0,    2,       5,        */{ 7,  0 },
  /*  38:    1, 2,       5,        */{ 5, 10 },
  /*  39: 0, 1, 2,       5,        */{ 9,  0 },
  /*  40:          3,    5,        */{ 4,  3 },
  /*  41: 0,       3,    5,        */{ 6,  4 },
  /*  42:    1,    3,    5,        */{ 6, 11 },
  /*  43: 0, 1,    3,    5,        */{ 14,  1 },
  /*  44:       2, 3,    5,        */{ 6, 17 },
  /*  45: 0,    2, 3,    5,        */{ 12,  4 },
  /*  46:    1, 2, 3,    5,        */{ 11,  6 },
  /*  47: 0, 1, 2, 3,    5,        */{ 5, 25 },
  /*  48:             4, 5,        */{ 2,  8 },
  /*  49: 0,          4, 5,        */{ 5,  7 },
  /*  50:    1,       4, 5,        */{ 5, 12 },
  /*  51: 0, 1,       4, 5,        */{ 8,  1 },
  /*  52:       2,    4, 5,        */{ 6, 18 },
  /*  53: 0,    2,    4, 5,        */{ 12,  5 },
  /*  54:    1, 2,    4, 5,        */{ 14,  7 },
  /*  55: 0, 1, 2,    4, 5,        */{ 5, 28 },
  /*  56:          3, 4, 5,        */{ 6, 21 },
  /*  57: 0,       3, 4, 5,        */{ 11,  4 },
  /*  58:    1,    3, 4, 5,        */{ 12, 15 },
  /*  59: 0, 1,    3, 4, 5,        */{ 5, 30 },
  /*  60:       2, 3, 4, 5,        */{ 10,  5 },
  /*  61: 0,    2, 3, 4, 5,        */{ 6, 32 },
  /*  62:    1, 2, 3, 4, 5,        */{ 6, 39 },
  /*  63: 0, 1, 2, 3, 4, 5,        */{ 2, 12 },
  /*  64:                   6,     */{ 1,  6 },
  /*  65: 0,                6,     */{ 4,  0 },
  /*  66:    1,             6,     */{ 3,  5 },
  /*  67: 0, 1,             6,     */{ 6,  0 },
  /*  68:       2,          6,     */{ 2,  6 },
  /*  69: 0,    2,          6,     */{ 6,  3 },
  /*  70:    1, 2,          6,     */{ 5, 11 },
  /*  71: 0, 1, 2,          6,     */{ 14,  0 },
  /*  72:          3,       6,     */{ 3,  9 },
  /*  73: 0,       3,       6,     */{ 6,  5 },
  /*  74:    1,    3,       6,     */{ 7,  4 },
  /*  75: 0, 1,    3,       6,     */{ 12,  1 },
  /*  76:       2, 3,       6,     */{ 5, 14 },
  /*  77: 0,    2, 3,       6,     */{ 11,  3 },
  /*  78:    1, 2, 3,       6,     */{ 9,  4 },
  /*  79: 0, 1, 2, 3,       6,     */{ 5, 26 },
  /*  80:             4,    6,     */{ 3, 10 },
  /*  81: 0,          4,    6,     */{ 6,  6 },
  /*  82:    1,       4,    6,     */{ 7,  5 },
  /*  83: 0, 1,       4,    6,     */{ 12,  2 },
  /*  84:       2,    4,    6,     */{ 6, 19 },
  /*  85: 0,    2,    4,    6,     */{ 10,  1 },
  /*  86:    1, 2,    4,    6,     */{ 12, 13 },
  /*  87: 0, 1, 2,    4,    6,     */{ 6, 24 },
  /*  88:          3, 4,    6,     */{ 7,  7 },
  /*  89: 0,       3, 4,    6,     */{ 12,  9 },
  /*  90:    1,    3, 4,    6,     */{ 13,  1 },
  /*  91: 0, 1,    3, 4,    6,     */{ 7,  9 },
  /*  92:       2, 3, 4,    6,     */{ 12, 20 },
  /*  93: 0,    2, 3, 4,    6,     */{ 6, 33 },
  /*  94:    1, 2, 3, 4,    6,     */{ 7, 13 },
  /*  95: 0, 1, 2, 3, 4,    6,     */{ 3, 12 },
  /*  96:                5, 6,     */{ 2, 10 },
  /*  97: 0,             5, 6,     */{ 6,  7 },
  /*  98:    1,          5, 6,     */{ 5, 13 },
  /*  99: 0, 1,          5, 6,     */{ 11,  2 },
  /* 100:       2,       5, 6,     */{ 5, 16 },
  /* 101: 0,    2,       5, 6,     */{ 12,  7 },
  /* 102:    1, 2,       5, 6,     */{ 8,  3 },
  /* 103: 0, 1, 2,       5, 6,     */{ 5, 29 },
  /* 104:          3,    5, 6,     */{ 6, 22 },
  /* 105: 0,       3,    5, 6,     */{ 10,  2 },
  /* 106:    1,    3,    5, 6,     */{ 12, 17 },
  /* 107: 0, 1,    3,    5, 6,     */{ 6, 27 },
  /* 108:       2, 3,    5, 6,     */{ 14,  9 },
  /* 109: 0,    2, 3,    5, 6,     */{ 6, 34 },
  /* 110:    1, 2, 3,    5, 6,     */{ 5, 39 },
  /* 111: 0, 1, 2, 3,    5, 6,     */{ 2, 14 },
  /* 112:             4, 5, 6,     */{ 5, 20 },
  /* 113: 0,          4, 5, 6,     */{ 14,  5 },
  /* 114:    1,       4, 5, 6,     */{ 9,  5 },
  /* 115: 0, 1,       4, 5, 6,     */{ 5, 32 },
  /* 116:       2,    4, 5, 6,     */{ 11, 10 },
  /* 117: 0,    2,    4, 5, 6,     */{ 6, 35 },
  /* 118:    1, 2,    4, 5, 6,     */{ 5, 41 },
  /* 119: 0, 1, 2,    4, 5, 6,     */{ 2, 16 },
  /* 120:          3, 4, 5, 6,     */{ 12, 23 },
  /* 121: 0,       3, 4, 5, 6,     */{ 6, 37 },
  /* 122:    1,    3, 4, 5, 6,     */{ 7, 14 },
  /* 123: 0, 1,    3, 4, 5, 6,     */{ 3, 16 },
  /* 124:       2, 3, 4, 5, 6,     */{ 6, 46 },
  /* 125: 0,    2, 3, 4, 5, 6,     */{ 4,  6 },
  /* 126:    1, 2, 3, 4, 5, 6,     */{ 3, 21 },
  /* 127: 0, 1, 2, 3, 4, 5, 6,     */{ 1,  8 },
  /* 128:                      7,  */{ 1,  7 },
  /* 129: 0,                   7,  */{ 3,  2 },
  /* 130:    1,                7,  */{ 4,  1 },
  /* 131: 0, 1,                7,  */{ 6,  1 },
  /* 132:       2,             7,  */{ 3,  7 },
  /* 133: 0,    2,             7,  */{ 7,  1 },
  /* 134:    1, 2,             7,  */{ 6, 10 },
  /* 135: 0, 1, 2,             7,  */{ 12,  0 },
  /* 136:          3,          7,  */{ 2,  7 },
  /* 137: 0,       3,          7,  */{ 5,  6 },
  /* 138:    1,    3,          7,  */{ 6, 12 },
  /* 139: 0, 1,    3,          7,  */{ 11,  1 },
  /* 140:       2, 3,          7,  */{ 5, 15 },
  /* 141: 0,    2, 3,          7,  */{ 9,  2 },
  /* 142:    1, 2, 3,          7,  */{ 14,  6 },
  /* 143: 0, 1, 2, 3,          7,  */{ 5, 27 },
  /* 144:             4,       7,  */{ 2,  9 },
  /* 145: 0,          4,       7,  */{ 5,  8 },
  /* 146:    1,       4,       7,  */{ 6, 13 },
  /* 147: 0, 1,       4,       7,  */{ 14,  2 },
  /* 148:       2,    4,       7,  */{ 6, 20 },
  /* 149: 0,    2,    4,       7,  */{ 12,  6 },
  /* 150:    1, 2,    4,       7,  */{ 10,  3 },
  /* 151: 0, 1, 2,    4,       7,  */{ 6, 25 },
  /* 152:          3, 4,       7,  */{ 5, 18 },
  /* 153: 0,       3, 4,       7,  */{ 8,  2 },
  /* 154:    1,    3, 4,       7,  */{ 12, 16 },
  /* 155: 0, 1,    3, 4,       7,  */{ 5, 31 },
  /* 156:       2, 3, 4,       7,  */{ 11,  9 },
  /* 157: 0,    2, 3, 4,       7,  */{ 5, 34 },
  /* 158:    1, 2, 3, 4,       7,  */{ 6, 40 },
  /* 159: 0, 1, 2, 3, 4,       7,  */{ 2, 13 },
  /* 160:                5,    7,  */{ 3, 11 },
  /* 161: 0,             5,    7,  */{ 7,  2 },
  /* 162:    1,          5,    7,  */{ 6, 14 },
  /* 163: 0, 1,          5,    7,  */{ 12,  3 },
  /* 164:       2,       5,    7,  */{ 7,  6 },
  /* 165: 0,    2,       5,    7,  */{ 13,  0 },
  /* 166:    1, 2,       5,    7,  */{ 12, 14 },
  /* 167: 0, 1, 2,       5,    7,  */{ 7,  8 },
  /* 168:          3,    5,    7,  */{ 6, 23 },
  /* 169: 0,       3,    5,    7,  */{ 12, 10 },
  /* 170:    1,    3,    5,    7,  */{ 10,  4 },
  /* 171: 0, 1,    3,    5,    7,  */{ 6, 28 },
  /* 172:       2, 3,    5,    7,  */{ 12, 21 },
  /* 173: 0,    2, 3,    5,    7,  */{ 7, 10 },
  /* 174:    1, 2, 3,    5,    7,  */{ 6, 41 },
  /* 175: 0, 1, 2, 3,    5,    7,  */{ 3, 13 },
  /* 176:             4, 5,    7,  */{ 5, 21 },
  /* 177: 0,          4, 5,    7,  */{ 9,  3 },
  /* 178:    1,       4, 5,    7,  */{ 11,  8 },
  /* 179: 0, 1,       4, 5,    7,  */{ 5, 33 },
  /* 180:       2,    4, 5,    7,  */{ 12, 22 },
  /* 181: 0,    2,    4, 5,    7,  */{ 7, 11 },
  /* 182:    1, 2,    4, 5,    7,  */{ 6, 42 },
  /* 183: 0, 1, 2,    4, 5,    7,  */{ 3, 14 },
  /* 184:          3, 4, 5,    7,  */{ 14, 11 },
  /* 185: 0,       3, 4, 5,    7,  */{ 5, 36 },
  /* 186:    1,    3, 4, 5,    7,  */{ 6, 44 },
  /* 187: 0, 1,    3, 4, 5,    7,  */{ 2, 17 },
  /* 188:       2, 3, 4, 5,    7,  */{ 6, 47 },
  /* 189: 0,    2, 3, 4, 5,    7,  */{ 3, 18 },
  /* 190:    1, 2, 3, 4, 5,    7,  */{ 4,  7 },
  /* 191: 0, 1, 2, 3, 4, 5,    7,  */{ 1,  9 },
  /* 192:                   6, 7,  */{ 2, 11 },
  /* 193: 0,                6, 7,  */{ 6,  8 },
  /* 194:    1,             6, 7,  */{ 6, 15 },
  /* 195: 0, 1,             6, 7,  */{ 10,  0 },
  /* 196:       2,          6, 7,  */{ 5, 17 },
  /* 197: 0,    2,          6, 7,  */{ 12,  8 },
  /* 198:    1, 2,          6, 7,  */{ 11,  7 },
  /* 199: 0, 1, 2,          6, 7,  */{ 6, 26 },
  /* 200:          3,       6, 7,  */{ 5, 19 },
  /* 201: 0,       3,       6, 7,  */{ 14,  4 },
  /* 202:    1,    3,       6, 7,  */{ 12, 18 },
  /* 203: 0, 1,    3,       6, 7,  */{ 6, 29 },
  /* 204:       2, 3,       6, 7,  */{ 8,  4 },
  /* 205: 0,    2, 3,       6, 7,  */{ 5, 35 },
  /* 206:    1, 2, 3,       6, 7,  */{ 5, 40 },
  /* 207: 0, 1, 2, 3,       6, 7,  */{ 2, 15 },
  /* 208:             4,    6, 7,  */{ 5, 22 },
  /* 209: 0,          4,    6, 7,  */{ 11,  5 },
  /* 210:    1,       4,    6, 7,  */{ 12, 19 },
  /* 211: 0, 1,       4,    6, 7,  */{ 6, 30 },
  /* 212:       2,    4,    6, 7,  */{ 14, 10 },
  /* 213: 0,    2,    4,    6, 7,  */{ 6, 36 },
  /* 214:    1, 2,    4,    6, 7,  */{ 6, 43 },
  /* 215: 0, 1, 2,    4,    6, 7,  */{ 4,  4 },
  /* 216:          3, 4,    6, 7,  */{ 9,  7 },
  /* 217: 0,       3, 4,    6, 7,  */{ 5, 37 },
  /* 218:    1,    3, 4,    6, 7,  */{ 7, 15 },
  /* 219: 0, 1,    3, 4,    6, 7,  */{ 3, 17 },
  /* 220:       2, 3, 4,    6, 7,  */{ 5, 44 },
  /* 221: 0,    2, 3, 4,    6, 7,  */{ 2, 19 },
  /* 222:    1, 2, 3, 4,    6, 7,  */{ 3, 22 },
  /* 223: 0, 1, 2, 3, 4,    6, 7,  */{ 1, 10 },
  /* 224:                5, 6, 7,  */{ 5, 23 },
  /* 225: 0,             5, 6, 7,  */{ 12, 11 },
  /* 226:    1,          5, 6, 7,  */{ 14,  8 },
  /* 227: 0, 1,          5, 6, 7,  */{ 6, 31 },
  /* 228:       2,       5, 6, 7,  */{ 9,  6 },
  /* 229: 0,    2,       5, 6, 7,  */{ 7, 12 },
  /* 230:    1, 2,       5, 6, 7,  */{ 5, 42 },
  /* 231: 0, 1, 2,       5, 6, 7,  */{ 3, 15 },
  /* 232:          3,    5, 6, 7,  */{ 11, 11 },
  /* 233: 0,       3,    5, 6, 7,  */{ 6, 38 },
  /* 234:    1,    3,    5, 6, 7,  */{ 6, 45 },
  /* 235: 0, 1,    3,    5, 6, 7,  */{ 4,  5 },
  /* 236:       2, 3,    5, 6, 7,  */{ 5, 45 },
  /* 237: 0,    2, 3,    5, 6, 7,  */{ 3, 19 },
  /* 238:    1, 2, 3,    5, 6, 7,  */{ 2, 21 },
  /* 239: 0, 1, 2, 3,    5, 6, 7,  */{ 1, 11 },
  /* 240:             4, 5, 6, 7,  */{ 8,  5 },
  /* 241: 0,          4, 5, 6, 7,  */{ 5, 38 },
  /* 242:    1,       4, 5, 6, 7,  */{ 5, 43 },
  /* 243: 0, 1,       4, 5, 6, 7,  */{ 2, 18 },
  /* 244:       2,    4, 5, 6, 7,  */{ 5, 46 },
  /* 245: 0,    2,    4, 5, 6, 7,  */{ 3, 20 },
  /* 246:    1, 2,    4, 5, 6, 7,  */{ 2, 22 },
  /* 247: 0, 1, 2,    4, 5, 6, 7,  */{ 1, 12 },
  /* 248:          3, 4, 5, 6, 7,  */{ 5, 47 },
  /* 249: 0,       3, 4, 5, 6, 7,  */{ 2, 20 },
  /* 250:    1,    3, 4, 5, 6, 7,  */{ 3, 23 },
  /* 251: 0, 1,    3, 4, 5, 6, 7,  */{ 1, 13 },
  /* 252:       2, 3, 4, 5, 6, 7,  */{ 2, 23 },
  /* 253: 0,    2, 3, 4, 5, 6, 7,  */{ 1, 14 },
  /* 254:    1, 2, 3, 4, 5, 6, 7,  */{ 1, 15 },
  /* 255: 0, 1, 2, 3, 4, 5, 6, 7,  */{ 0, -1 }
};

inline float getValue(uint smaller, uint bigger) {
  if (smaller > CONST_ISO) {
    if (bigger <= CONST_ISO) {
      return abs(CONST_ISO - smaller) / fabs(bigger - smaller);
    } else {
      return -1;
    }
  } else {
    if (bigger > CONST_ISO) {
      return abs(CONST_ISO - smaller) / fabs(bigger - smaller);
    } else {
      return -1;
    }
  }
}

kernel
void createVertices(const global uint * grid,
                    gloabal float * vertices) {
  const uint x = get_global_id(0);
  const uint y = get_global_id(1);
  const uint z = get_global_id(2);

  if (x == 0 || y == 0 || z == 0) {
    return;
  }

  uint index = z + y * CONST_SIZE_Z + x * CONST_SIZE_YZ;
  uint v4 = grid[(index - CONST_SIZE_Z) - CONST_SIZE_YZ];
  uint v5 = grid[index - CONST_SIZE_Z];
  uint v6 = grid[index];
  uint v7 = grid[index - CONST_SIZE_YZ];
  index--;
  uint v0 = grid[(index - CONST_SIZE_Z) - CONST_SIZE_YZ];
  uint v1 = grid[index - CONST_SIZE_Z];
  uint v2 = grid[index];
  uint v3 = grid[index - CONST_SIZE_YZ];
  uint base = CONST_SIZE_X * CONST_SIZE_Y * CONST_SIZE_Z * 3;

  // E: 5
  vertices[index * 3] = getValue(v5, v6);

  // E: 6
  vertices[index * 3 + 1] = getValue(v7, v6);

  // E: 10
  vertices[index * 3 + 2] = getValue(v2, v6);


  if (z == 1) {
    if (y == 1) {
      // E: 0
      vertices[base + x] = getValue(v0, v1);
    }
    base += CONST_SIZE_X;

    if (x == 1) {
      // E: 3
      vertices[base + y] = getValue(v0, v3);
    }
    base += CONST_SIZE_Y;

    // E: 1
    vertices[base + y + x * CONST_SIZE_Y] = getValue(v1, v2);

    // E: 2
    vertices[base + y + x * CONST_SIZE_Y + 1] = getValue(v3, v2);
    base += CONST_SIZE_X * CONST_SIZE_Y * 2;
  } else {
    base += CONST_SIZE_X + CONST_SIZE_Y + CONST_SIZE_X * CONST_SIZE_Y * 2;
  }

  if (y == 1) {
    if (x == 1) {
      // E: 8
      vertices[base + y] = getValue(v0, v4);
    }
    base += CONST_SIZE_Y;

    // E: 4
    vertices[base + z + x * CONST_SIZE_Z] = getValue(v4, v5);

    // E: 9
    vertices[base + z + x * CONST_SIZE_Z + 1] = getValue(v1, v5);
    base += CONST_SIZE_X * CONST_SIZE_Z * 2;
  } else {
    base += CONST_SIZE_Y + CONST_SIZE_X * CONST_SIZE_Z * 2;
  }

  if (x == 1) {
    // E: 7
    vertices[base + z + y * CONST_SIZE_Z] = getValue(v4, v7);
    // E: 11
    vertices[base + z + y * CONST_SIZE_Z + 1] = getValue(v3, v7);
  }
}

kernel
void march(const global uint * grid,
           global volatile uint * output) {
  uchar classification = 0;
  const ushort x = get_global_id(0);
  const ushort y = get_global_id(1);
  const ushort z = get_global_id(2);

  if (x == 0 || y == 0 || z == 0) {
    return;
  }
  
  uint index = z + y * CONST_SIZE_Z + x * CONST_SIZE_YZ;

  // 4
  if (grid[(index - CONST_SIZE_YZ) - CONST_SIZE_Z] > CONST_ISO) {
    classification += 16;
  }

  // 5
  if (grid[index - CONST_SIZE_Z] > CONST_ISO) {
    classification += 32;
  }

  // 6
  if (grid[index] > CONST_ISO) {
    classification += 64;
  }

  // 7
  if (grid[index - CONST_SIZE_YZ] > CONST_ISO) {
    classification += 128;
  }

  index--;

  // 0 
  if (grid[(index - CONST_SIZE_YZ) - CONST_SIZE_Z] > CONST_ISO) {
    classification += 1;
  }

  // 1
  if (grid[index - CONST_SIZE_Z] > CONST_ISO) {
    classification += 2;
  }

  // 2
  if (grid[index - 1] > CONST_ISO) {
    classification += 4;
  }

  // 3
  if (grid[index - CONST_SIZE_YZ] > CONST_ISO) {
    classification += 8;
  }

  switch (classification) {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    case 8:
      break;
    case 9:
      break;
    case 10:
      break;
    case 11:
      break;
    case 12:
      break;
    case 13:
      break;
    case 14:
      break;
    case 15:
      break;
    case 16:
      break;
    case 17:
      break;
    case 18:
      break;
    case 19:
      break;
    case 20:
      break;
    case 21:
      break;
    case 22:
      break;
    case 23:
      break;
    case 24:
      break;
    case 25:
      break;
    case 26:
      break;
    case 27:
      break;
    case 28:
      break;
    case 29:
      break;
    case 30:
      break;
    case 31:
      break;
    case 32:
      break;
    case 33:
      break;
    case 34:
      break;
    case 35:
      break;
    case 36:
      break;
    case 37:
      break;
    case 38:
      break;
    case 39:
      break;
    case 40:
      break;
    case 41:
      break;
    case 42:
      break;
    case 43:
      break;
    case 44:
      break;
    case 45:
      break;
    case 46:
      break;
    case 47:
      break;
    case 48:
      break;
    case 49:
      break;
    case 50:
      break;
    case 51:
      break;
    case 52:
      break;
    case 53:
      break;
    case 54:
      break;
    case 55:
      break;
    case 56:
      break;
    case 57:
      break;
    case 58:
      break;
    case 59:
      break;
    case 60:
      break;
    case 61:
      break;
    case 62:
      break;
    case 63:
      break;
    case 64:
      break;
    case 65:
      break;
    case 66:
      break;
    case 67:
      break;
    case 68:
      break;
    case 69:
      break;
    case 70:
      break;
    case 71:
      break;
    case 72:
      break;
    case 73:
      break;
    case 74:
      break;
    case 75:
      break;
    case 76:
      break;
    case 77:
      break;
    case 78:
      break;
    case 79:
      break;
    case 80:
      break;
    case 81:
      break;
    case 82:
      break;
    case 83:
      break;
    case 84:
      break;
    case 85:
      break;
    case 86:
      break;
    case 87:
      break;
    case 88:
      break;
    case 89:
      break;
    case 90:
      break;
    case 91:
      break;
    case 92:
      break;
    case 93:
      break;
    case 94:
      break;
    case 95:
      break;
    case 96:
      break;
    case 97:
      break;
    case 98:
      break;
    case 99:
      break;
    case 100:
      break;
    case 101:
      break;
    case 102:
      break;
    case 103:
      break;
    case 104:
      break;
    case 105:
      break;
    case 106:
      break;
    case 107:
      break;
    case 108:
      break;
    case 109:
      break;
    case 110:
      break;
    case 111:
      break;
    case 112:
      break;
    case 113:
      break;
    case 114:
      break;
    case 115:
      break;
    case 116:
      break;
    case 117:
      break;
    case 118:
      break;
    case 119:
      break;
    case 120:
      break;
    case 121:
      break;
    case 122:
      break;
    case 123:
      break;
    case 124:
      break;
    case 125:
      break;
    case 126:
      break;
    case 127:
      break;
    case 128:
      break;
    case 129:
      break;
    case 130:
      break;
    case 131:
      break;
    case 132:
      break;
    case 133:
      break;
    case 134:
      break;
    case 135:
      break;
    case 136:
      break;
    case 137:
      break;
    case 138:
      break;
    case 139:
      break;
    case 140:
      break;
    case 141:
      break;
    case 142:
      break;
    case 143:
      break;
    case 144:
      break;
    case 145:
      break;
    case 146:
      break;
    case 147:
      break;
    case 148:
      break;
    case 149:
      break;
    case 150:
      break;
    case 151:
      break;
    case 152:
      break;
    case 153:
      break;
    case 154:
      break;
    case 155:
      break;
    case 156:
      break;
    case 157:
      break;
    case 158:
      break;
    case 159:
      break;
    case 160:
      break;
    case 161:
      break;
    case 162:
      break;
    case 163:
      break;
    case 164:
      break;
    case 165:
      break;
    case 166:
      break;
    case 167:
      break;
    case 168:
      break;
    case 169:
      break;
    case 170:
      break;
    case 171:
      break;
    case 172:
      break;
    case 173:
      break;
    case 174:
      break;
    case 175:
      break;
    case 176:
      break;
    case 177:
      break;
    case 178:
      break;
    case 179:
      break;
    case 180:
      break;
    case 181:
      break;
    case 182:
      break;
    case 183:
      break;
    case 184:
      break;
    case 185:
      break;
    case 186:
      break;
    case 187:
      break;
    case 188:
      break;
    case 189:
      break;
    case 190:
      break;
    case 191:
      break;
    case 192:
      break;
    case 193:
      break;
    case 194:
      break;
    case 195:
      break;
    case 196:
      break;
    case 197:
      break;
    case 198:
      break;
    case 199:
      break;
    case 200:
      break;
    case 201:
      break;
    case 202:
      break;
    case 203:
      break;
    case 204:
      break;
    case 205:
      break;
    case 206:
      break;
    case 207:
      break;
    case 208:
      break;
    case 209:
      break;
    case 210:
      break;
    case 211:
      break;
    case 212:
      break;
    case 213:
      break;
    case 214:
      break;
    case 215:
      break;
    case 216:
      break;
    case 217:
      break;
    case 218:
      break;
    case 219:
      break;
    case 220:
      break;
    case 221:
      break;
    case 222:
      break;
    case 223:
      break;
    case 224:
      break;
    case 225:
      break;
    case 226:
      break;
    case 227:
      break;
    case 228:
      break;
    case 229:
      break;
    case 230:
      break;
    case 231:
      break;
    case 232:
      break;
    case 233:
      break;
    case 234:
      break;
    case 235:
      break;
    case 236:
      break;
    case 237:
      break;
    case 238:
      break;
    case 239:
      break;
    case 240:
      break;
    case 241:
      break;
    case 242:
      break;
    case 243:
      break;
    case 244:
      break;
    case 245:
      break;
    case 246:
      break;
    case 247:
      break;
    case 248:
      break;
    case 249:
      break;
    case 250:
      break;
    case 251:
      break;
    case 252:
      break;
    case 253:
      break;
    case 254:
      break;
    case 255:
      break;
  }
}
