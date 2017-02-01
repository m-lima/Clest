//_____________________________________________________________________________
// Marching cubes from grid
//
// #define CONST_ISO 0
//_____________________________________________________________________________

inline uint getIndex(const ushort x,
                     const ushort y,
                     const ushort z) {
  return z + y * CONST_SIZE_Z + x * CONST_SIZE_Y * CONST_SIZE_Z;
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

  if (grid[getIndex(x, y, z)] > CONST_ISO) {
    classification += 1;
  }
  if (grid[getIndex(x - 1, y, z)] > CONST_ISO) {
    classification += 2;
  }
  if (grid[getIndex(x - 1, y - 1, z)] > CONST_ISO) {
    classification += 4;
  }
  if (grid[getIndex(x, y - 1, z)] > CONST_ISO) {
    classification += 8;
  }
  if (grid[getIndex(x, y, z - 1)] > CONST_ISO) {
    classification += 16;
  }
  if (grid[getIndex(x - 1, y, z - 1)] > CONST_ISO) {
    classification += 32;
  }
  if (grid[getIndex(x - 1, y - 1, z - 1)] > CONST_ISO) {
    classification += 64;
  }
  if (grid[getIndex(x, y - 1, z - 1)] > CONST_ISO) {
    classification += 128;
  }
}
//_____________________________________________________________________________
