/* encoder.h
 * Helper for generating variable-length output.
 */

#ifndef ENCODER_H
#define ENCODER_H
 
struct encoder {
  char *v;
  int c,a;
};

void encoder_cleanup(struct encoder *encoder);
int encoder_require(struct encoder *encoder,int addc);
int encoder_append(struct encoder *encoder,const void *src,int srcc);
int encoder_appendf(struct encoder *encoder,const char *fmt,...);

#endif
