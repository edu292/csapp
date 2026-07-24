#include <stddef.h>
#include <sys/types.h>

#define IDENT 1
#define OP *

typedef double data_t;

typedef struct {
  size_t length;
  data_t *data;
} *vec_ptr;

size_t vec_length(vec_ptr vec) { return vec->length; }

data_t *get_vec_start(vec_ptr vec) { return vec->data; }

void combine5(vec_ptr v, data_t *dest) {
  long i;
  long length = vec_length(v);
  long limit = length - 4;
  data_t *data = get_vec_start(v);
  data_t acc = IDENT;
  /* Combine 2 elements at a time */
  for (i = 0; i < limit; i += 5) {
    acc = ((((acc OP data[i])OP data[i + 1]) OP data[i + 2]) OP data[i + 3])
        OP data[i + 4];
  }

  /* Finish any remaining elements */
  for (; i < length; i++) {
    acc = acc OP data[i];
  }
  *dest = acc;
}
