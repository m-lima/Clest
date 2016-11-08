//Single cell
__kernel void fractal(__global unsigned char* output,
                      const unsigned char seed) {
  unsigned long index = get_global_id(0);
  unsigned char value = index & (index >> 8);
  unsigned char base = value;
  unsigned char pow = 0;
  for (; pow < seed; pow++) {
    value *= base;
  }
  //value = pow((double)value, 2);
  //value = convert_uchar_rtz(powr(convert_float_rtz(value), 2));
  output[index] = value;
}
