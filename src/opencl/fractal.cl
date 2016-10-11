//Single cell
__kernel void fractalSingle(__global unsigned char* output ) {
  unsigned int index = get_global_id(0);
  output[index] = index & (index << 4);
}

//Block cell
__kernel void fractalBlock(
               __global unsigned char* output,
               const unsigned int iterations) {

  unsigned int index = get_global_id(0);
  unsigned int i;
  unsigned int workingValue;

  for (i = 0; i < iterations; i++) {
    workingValue = index * iterations + i;
    output[workingValue] = workingValue & (workingValue << 4);
  }
}
