//Single cell
__kernel void fractalSingle(__global unsigned char* output ) {
  unsigned int index = get_global_id(0);
  output[index] = index & (index << 4);
}

//Block cell
__kernel void fractalBlock(__global unsigned char* output, const unsigned int iterations, const unsigned int repeat) {
  unsigned int index = get_global_id(0);
  unsigned int i;
  unsigned int workingIndex;
  unsigned int workingValue;

  for (i = 0; i < iterations, i++) {
    workingIndex = index + i;
    workingValue = workingIndex * repeat;
    output[workingIndex] = workingValue & (workingValue << 4);
  }
}