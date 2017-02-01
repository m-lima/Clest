//_____________________________________________________________________________
// Marching cubes from grid
//
// #define CONST_ISO 0
// #define CONST_SIZE_X 0
// #define CONST_SIZE_Y 0
// #define CONST_SIZE_Z 0
// #define CONST_SIZE_YZ 0
//_____________________________________________________________________________

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
