#include "encoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

void encoder_cleanup(struct encoder *encoder) {
  if (encoder->v) free(encoder->v);
}

int encoder_require(struct encoder *encoder,int addc) {
  if (addc<1) return 0;
  if (encoder->c<=encoder->a-addc) return 0;
  int na=encoder->c+addc;
  if (na<INT_MAX-1024) na=(na+1024)&~1023;
  void *nv=realloc(encoder->v,na);
  if (!nv) return -1;
  encoder->v=nv;
  encoder->a=na;
  return 0;
}

int encoder_append(struct encoder *encoder,const void *src,int srcc) {
  if (!src) return 0;
  if (srcc<0) { srcc=0; while (((char*)src)[srcc]) srcc++; }
  if (encoder_require(encoder,srcc)<0) return -1;
  memcpy(encoder->v+encoder->c,src,srcc);
  encoder->c+=srcc;
  return 0;
}

int encoder_appendf(struct encoder *encoder,const char *fmt,...) {
  while (1) {
    va_list vargs;
    va_start(vargs,fmt);
    int err=vsnprintf(encoder->v+encoder->c,encoder->a-encoder->c,fmt,vargs);
    if ((err<0)||(err>=INT_MAX)) return -1;
    if (encoder->c<encoder->a-err) { // sic < not <=
      encoder->c+=err;
      return 0;
    }
    if (encoder_require(encoder,err+1)<0) return -1;
  }
}
