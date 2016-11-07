//Single cell
__kernel void fractal(__global unsigned char* output,
                      const unsigned char seed) {
  unsigned long index = get_global_id(0);
  unsigned char value = index & (index >> 8);
  value = convert_uchar_rtz(powr(convert_float(value), 2));
  output[index] = value;
}
