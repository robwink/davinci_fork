#include "endian_norm.h"



char * flip_endian(unsigned char * data, unsigned int data_elem,
		   unsigned int word_size) {
  /* Ensure the data is always read and written in big endian format.
     Try not to call this on big endian machines, since it's just a
     wasterful copy operation in that case. 
  */
  unsigned char * new_buf;
  unsigned int data_size, i = 0;
  
  data_size = data_elem * word_size;
  new_buf = calloc(data_elem, word_size);
  if (new_buf == NULL) {
    parse_error("Malloc failed. (Low memory?)");
    return NULL;
  }
#ifdef WORDS_BIGENDIAN
  for (i=0; i<data_size; i++) new_buf[i] = data[i];
#else
  for (i=0; i<data_size; i+=word_size) {
    switch (word_size) {
    case 1:
      new_buf[i] = data[i];
      break;
    case 2:
      new_buf[i+1] = data[i];
      new_buf[i] = data[i+1];
      break;
    case 4:
      new_buf[i+3] = data[i];
      new_buf[i+2] = data[i+1];
      new_buf[i+1] = data[i+2];
      new_buf[i] = data[i+3];
      break;
    case 8:
      new_buf[i+7] = data[i];
      new_buf[i+6] = data[i+1];
      new_buf[i+5] = data[i+2];
      new_buf[i+4] = data[i+3];
      new_buf[i+3] = data[i+4];
      new_buf[i+2] = data[i+5];
      new_buf[i+1] = data[i+6];
      new_buf[i] = data[i+7];
      break;
    default:
      new_buf[i] = data[i];
      break;
    }
  }
#endif /* WORDS_BIGENDIAN */
  return new_buf;
}


char * var_endian(Var * v) {
  unsigned int ws = 0;
  unsigned int el = 0;
  
  el = V_SIZE(v)[0] * V_SIZE(v)[1] * V_SIZE(v)[2];
  ws = NBYTES(V_FORMAT(v));
  return flip_endian(V_DATA(v), el, ws);
}

