#include <string.h>

// The order check makes sure that src does not run into dst
// when copying (e.g. we want to copy original data, not data
// we've already updated during the operation
void memmove(void* dstptr, const void* srcptr, size_t size){
  unsigned char* dst = (unsigned char*)dstptr;
  const unsigned char* src = (const unsigned char*)srcptr;

  if (dst < src) {
    for (size_t i = 0; i < size; i++){
      dst[i] = src[i];
    }
  } else {
    for (int i = size; i > 0; i--){
      dst[i-1] = src[i-1];
    }
  }
}
