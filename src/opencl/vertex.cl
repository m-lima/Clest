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
                    const global uchar3 * colors,
                    global volatile uint * indices,
                    global float * vertX,
                    global float * vertY,
                    global float * vertZ,
                    global float3 * vertColor) {
  const uint x = get_global_id(0);
  const uint y = get_global_id(1);
  const uint z = get_global_id(2);

  if (x == 0 || y == 0 || z == 0) {
    return;
  }

  uint index = z + y * CONST_SIZE_Z + x * CONST_SIZE_YZ;
  float value;
  uint globalIndex;

  // E: 5
  value = getValue(v5, v6);
  if (value >= 0.0) {
    globalIndex = atomic_inc(&indi)
  }
  vertices[index * 3] = getValue(v5, v6);

  // E: 6
  vertices[index * 3 + 1] = getValue(v7, v6);

  // E: 10
  vertices[index * 3 + 2] = getValue(v2, v6);
}

kernel
void createVertices(const global uint * grid,
                    const global uchar3 * colors,
                    global float3 * vertPos,
                    global float3 * vertColor) {
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