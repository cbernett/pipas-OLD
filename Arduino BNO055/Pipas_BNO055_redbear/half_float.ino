

// from ftp://ftp.fox-toolkit.org/pub/fasthalffloatconversion.pdf
// note that we are not handling any of the special cases:
//
// (conversion to half-float)
//1. Really small numbers and zero should map to a half-float value of zero.
//2. Small numbers should convert to subnormal half-float values.
//3. Regular magnitude numbers should just lose some precision.
//4. Large numbers should map to half-float Infinity.
//5. Infinity and NaNs should remain Infinity and NaNs in their half-float representation.
//
// (conversion from half-float)
//1. Zero (and negative zero).
//2. Subnormal half-float values should map to their corresponding float values; since floats have a
//larger range that half-floats, subnormal half-float values will map to proper float values.
//3. The half float value Infinity should map to a float value of Infinity.
//4. Half float NaN (not a number) values should map to float NaN values.
//
// that writeup, while a wonderful shortcut, uses 8k lookup tables for each direction, not ideal for our embedded environment.
// so for the moment we cheat.
//
// This C++ implementation looks better. Thanks ILM & friends!
// http://mrob.com/pub/math/s10e5.h.txt

// and another: https://github.com/signove/antidote/blob/master/src/util/bytelib.c

float halfFloatToFloat(uint16_t h) {
  // (ok, we do handle zero)
  if(h == 0) {
    return 0;
  }
  uint32_t f = ((h&0x8000)<<16) | (((h&0x7c00)+0x1C000)<<13) | ((h&0x03FF)<<13);
  return *(float *)&f;
}

uint16_t floatToHalfFloat(float val) {
  if(val == 0) {
    return val;
  }
  uint32_t f = *(uint32_t *)&val;
   return ((f>>16)&0x8000)|((((f&0x7f800000)-0x38000000)>>13)&0x7c00)|((f>>13)&0x03ff);
}

