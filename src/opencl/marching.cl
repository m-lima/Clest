kernel
__attribute__((vec_type_hint(float3)))
void createGrid(const global uint3 * points,
                global volatile uint * output,
                float3 step,
                float3 offset,
                ushort3 size) {
  const ulong index = get_global_id(0);
  float3 point = convert_float3(points[index]);

  ushort3 grid = convert_ushort3((point - offset) / step);

  if (grid.x == size.x) grid.x--;
  if (grid.y == size.y) grid.y--;
  if (grid.z == size.z) grid.z--;

  atomic_inc(&output[grid.z + grid.y * size.z + grid.x * size.y * size.z]);
}

//kernel
////__attribute__((work_group_size_hint(2, 2, 2)))
//void marching(global uint * grid,
//              global uchar * output) {
//  ushort indexX = get_global_id(0);
//  ushort indexY = get_global_id(1);
//  ushort indexZ = get_global_id(2);
//
//  output[indeX][inde]
//}