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