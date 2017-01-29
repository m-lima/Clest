//_____________________________________________________________________________
// Create fractal with simple math
//
// output: Fractal output
//_____________________________________________________________________________
__kernel void fractalS(__global unsigned char* output) {
  unsigned long index = get_global_id(0);
  output[index]  = index & (index >> 8);
}
//_____________________________________________________________________________

//_____________________________________________________________________________
// Create fractal with extra math
//
// output: Fractal output
// seed: Seed to avoid optimization
//_____________________________________________________________________________
__kernel void fractalP(__global unsigned char* output,
                      const unsigned char seed) {
  unsigned long index = get_global_id(0);
  unsigned char value = index & (index >> 8);
  unsigned char base = value;
  unsigned char pow = 0;
  for (; pow < seed; pow++) {
    value *= base;
  }
  output[index] = value;
}
//_____________________________________________________________________________
