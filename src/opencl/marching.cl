//#define CONST_OFFSET (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_STEP (float3){ 0.0f, 0.0f, 0.0f }
//#define CONST_SIZE_X 0
//#define CONST_SIZE_Y 0
//#define CONST_SIZE_Z 0

kernel
__attribute__((vec_type_hint(float3)))
void createGrid(const global uint3 * points,
                global volatile uint * output) {
  float3 point = convert_float3(points[get_global_id(0)]);

  ushort3 grid = convert_ushort3((point - CONST_OFFSET) / CONST_STEP);

  if (grid.x == CONST_SIZE_X) grid.x--;
  if (grid.y == CONST_SIZE_Y) grid.y--;
  if (grid.z == CONST_SIZE_Z) grid.z--;

  atomic_inc(&output[grid.z + grid.y * CONST_SIZE_Z + grid.x * CONST_SIZE_Y * CONST_SIZE_Z]);
}

//struct GridInfo {
//  float3 step;
//  float3 offset;
//  ushort3 size;
//};
//
//kernel
//__attribute__((vec_type_hint(float3)))
//void createGrid(const global uint3 * points,
//                global volatile uint * output,
//                constant struct GridInfo * gridInfo) {
//  float3 point = convert_float3(points[get_global_id(0)]);
//
//  ushort3 size = gridInfo[0].size;
//  ushort3 grid = convert_ushort3((point - gridInfo[0].offset) / gridInfo[0].step);
//
//  if (grid.x == size.x) grid.x--;
//  if (grid.y == size.y) grid.y--;
//  if (grid.z == size.z) grid.z--;
//
//  //printf("%d-%d-%d:%d\n", grid.x, grid.y, grid.z, output[grid.z + grid.y * size.z + grid.x * size.y * size.z]);
//
//  atomic_inc(&output[grid.z + grid.y * size.z + grid.x * size.y * size.z]);
//}

//kernel
//__attribute__((vec_type_hint(float3)))
//void createGrid(const global uint3 * points,
//                global volatile uint * output,
//                float3 step,
//                float3 offset,
//                ushort3 size) {
//  const ulong index = get_global_id(0);
//  float3 point = convert_float3(points[index]);
//
//  ushort3 grid = convert_ushort3((point - offset) / step);
//
//  if (grid.x == size.x) grid.x--;
//  if (grid.y == size.y) grid.y--;
//  if (grid.z == size.z) grid.z--;
//
//  atomic_inc(&output[grid.z + grid.y * size.z + grid.x * size.y * size.z]);
//}
