//Single cell
__kernel void fractal(__global unsigned char* output,
                            const unsigned char seed) {
  unsigned int index = get_global_id(0);
  output[index] = index & ((index & seed) << 3);
}
