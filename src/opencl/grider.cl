//_____________________________________________________________________________
// Create grid from LAS
//
//#define CONST_OFFSET (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_STEP (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_SIZE_X 0
//#define CONST_SIZE_Y 0
//#define CONST_SIZE_Z 0
//_____________________________________________________________________________
struct Point {
  uint3 coord;
  ushort3 color;
};

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
// Colorize the grid
//
//#define CONST_OFFSET (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_STEP (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_SIZE_X 0
//#define CONST_SIZE_Y 0
//#define CONST_SIZE_Z 0
//#define CONST_POINT_COUNT
//_____________________________________________________________________________
kernel
__attribute__((vec_type_hint(float3)))
void createGridolor(const global Point * points,
                    global volatile uint * output) {
  const uint x = get_global_id(0);
  const uint y = get_global_id(1);
  const uint z = get_global_id(2);
  uint value = 0;
  float3 color = { 0.0f, 0.0f, 0.0f };

  uint i = 0;
  for (i; i < CONST_POINT_COUNT; i++) {
    Point point = points[i];
    ushort3 grid = convert_ushort3((point - CONST_OFFSET) / CONST_STEP);

    if (grid.x == x && grid.y == y && grid.z == z) {
      value++;
      color = color - color / value + point.color / value;
    } else {
      if (grid.x == CONST_SIZE_X) grid.x--;
      if (grid.y == CONST_SIZE_Y) grid.y--;
      if (grid.z == CONST_SIZE_Z) grid.z--;
      if (grid.x == x && grid.y == y && grid.z == z) {
        value++;
        color = color - color / value + point.color / value;
      }
    }
  }

  output[z + y * CONST_SIZE_Z + x * CONST_SIZE_Y * CONST_SIZE_Z] = value;
}
//_____________________________________________________________________________

