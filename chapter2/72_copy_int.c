#include <string.h>

void copy_int(int val, void *buf, unsigned short maxbytes) {
  if (maxbytes - sizeof(val) >= 0)
    memcpy(buf, (void *)&val, sizeof(val));
}
