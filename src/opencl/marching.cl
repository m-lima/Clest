//_____________________________________________________________________________
// Create grid from LAS
//
//#define CONST_OFFSET (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_STEP (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_SIZE_X 0
//#define CONST_SIZE_Y 0
//#define CONST_SIZE_Z 0
//_____________________________________________________________________________
kernel
__attribute__((vec_type_hint(float3)))
void createGrid(const global uint3 * points,
                global volatile uint * output) {
  float3 point = convert_float3(points[get_global_id(0)]);

  ushort3 grid = convert_ushort3((point - CONST_OFFSET) / CONST_STEP);

  if (grid.x == CONST_SIZE_X) grid.x--;
  if (grid.y == CONST_SIZE_Y) grid.y--;
  if (grid.z == CONST_SIZE_Z) grid.z--;

  atomic_inc(&output[grid.z
             + grid.y * CONST_SIZE_Z
             + grid.x * CONST_SIZE_Y * CONST_SIZE_Z]);
}
//_____________________________________________________________________________

//_____________________________________________________________________________
// Marching cubes from grid
//
//#define CONST_ISO 0
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
