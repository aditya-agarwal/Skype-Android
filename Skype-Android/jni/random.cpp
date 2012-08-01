#include "bmcrypto/tc_crypto.hpp"

uchar tc_random_func(void *s)
{
uchar c=(uchar)(rand()>>8);
FILE *fp=fopen("/dev/urandom","rb");
  if (fp) {
    c=fgetc(fp);
    fclose(fp);
  }
  return c;
}

