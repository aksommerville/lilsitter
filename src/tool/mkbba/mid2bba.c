#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "common/encoder.h"
#include "bb_midi.h"

/* Context.
 */
 
struct mid2bba_context {
  struct encoder dst;
  const char *srcpath;
  const void *midi;
  int midic;
  struct bb_midi_file *midifile;
  struct bb_midi_file_reader *reader;
  int delay; // in frames, 256 frames per bba tick
  uint8_t chid;
  int duration; // in ticks
  int last_note_time; // ticks at the last note event
  int last_note_dstc; // '' (dst.c), including the note event
};

static void mid2bba_context_cleanup(struct mid2bba_context *ctx) {
  encoder_cleanup(&ctx->dst);
  bb_midi_file_del(ctx->midifile);
  bb_midi_file_reader_del(ctx->reader);
}

/* Encode any pending delay and drop to zero in context.
 * Encoded delay is just an 8-bit integer with the high bit clear.
 */
 
static int mid2bba_flush_delay(struct mid2bba_context *ctx) {
  int tickc=ctx->delay>>8;
  ctx->duration+=tickc;
  ctx->delay&=0xff;
  while (tickc>=0x7f) {
    if (encode_u8(&ctx->dst,0x7f)<0) return -1;
    tickc-=0x7f;
  }
  if (tickc>0) {
    if (encode_u8(&ctx->dst,tickc)<0) return -1;
  }
  return 0;
}

/* Emit note.
 */
 
static int mid2bba_emit_note(struct mid2bba_context *ctx,uint8_t chid,uint8_t noteid,uint8_t velocity) {

  if ((noteid<0x20)||(noteid>=0x60)) {
    //TODO shift offset if necessary
    return 0;
  }
  noteid-=0x20;
  
  if (chid!=ctx->chid) {
    if (encode_u8(&ctx->dst,0xe0|chid)<0) return -1;
    ctx->chid=chid;
  }

  if (encode_u8(&ctx->dst,0x80|noteid)<0) return -1;
  if (encode_u8(&ctx->dst,velocity)<0) return -1;
  ctx->last_note_time=ctx->duration;
  ctx->last_note_dstc=ctx->dst.c;

  return 0;
}

/* Receive event.
 */
 
static int mid2bba_receive_event(struct mid2bba_context *ctx,const struct bb_midi_event *event) {

  // Check for any events we can ignore.
  //TODO
  
  // If a delay is pending, emit it.
  if (mid2bba_flush_delay(ctx)<0) return -1;
  
  // Encode the event.
  //TODO
  //fprintf(stderr,"%s %02x @%02x (%02x,%02x) %d\n",__func__,event->opcode,event->chid,event->a,event->b,event->c);
  switch (event->opcode) {
    case BB_MIDI_OPCODE_NOTE_ON: if (mid2bba_emit_note(ctx,event->chid,event->a,event->b)<0) return -1; break;
    case BB_MIDI_OPCODE_NOTE_OFF: if (mid2bba_emit_note(ctx,event->chid,event->a,0)<0) return -1; break;
  }
  
  return 0;
}

/* Receive delay.
 * We don't encode it right away: Could be that we're going to ignore the next event, and can combine some delays.
 */
 
static int mid2bba_receive_delay(struct mid2bba_context *ctx,int tickc) {
  if (ctx->delay>INT_MAX-tickc) return -1;
  ctx->delay+=tickc;
  return 0;
}

/* After reencoding, examine the output:
 *  - Remove any leading delay. At least one of my MIDIs has a silent bar at the start.
 *  - Look for excessive trailing delay. Logic Pro X does this and I can't figure out why.
 *    Beware that I'm not going to try to figure out where the bar ends.
 *    Recommend drawing a note out to the very end of the last bar.
 */
 
static void mid2bba_trim_edge_delays(struct mid2bba_context *ctx) {
  
  // We've recorded the position of the last note. If there's a lot of time after that, trim it.
  int trailing_delay=ctx->duration-ctx->last_note_time;
  if (trailing_delay>=48) {
    //fprintf(stderr,"%s: Trimming final delay of %d ticks (>1 second).\n",ctx->srcpath,trailing_delay);
    ctx->duration=ctx->last_note_time;
    ctx->dst.c=ctx->last_note_dstc;
  }
  
  // Leading delay is pretty easy too.
  int leading_delay=0;
  int leadc=0;
  while ((leadc<ctx->dst.c)&&!(ctx->dst.v[leadc]&0x80)) {
    leading_delay+=ctx->dst.v[leadc];
    leadc++;
  }
  if (leading_delay) {
    //fprintf(stderr,"%s: Trimming leading delay of %d ticks.\n",ctx->srcpath);
    ctx->dst.c-=leadc;
    memmove(ctx->dst.v,ctx->dst.v+leadc,ctx->dst.c);
    ctx->duration-=leading_delay;
  }
  
  int sec=(ctx->duration+24)/48;
  int min=sec/60;
  sec%=60;
  //fprintf(stderr,"%s: Duration %d ticks, about %d:%02d\n",ctx->srcpath,ctx->duration,min,sec);
}

/* Reencode file in memory.
 */
 
static int mid2bba(struct mid2bba_context *ctx) {
  
  if (!(ctx->midifile=bb_midi_file_new(ctx->midi,ctx->midic))) {
    fprintf(stderr,"%s: Failed to parse MIDI file (%d bytes).\n",ctx->srcpath,ctx->midic);
    return -1;
  }
  if (!(ctx->reader=bb_midi_file_reader_new(ctx->midifile,48<<8))) {
    return -1;
  }
  ctx->reader->repeat=0;
  
  // Play the song and capture its output.
  while (1) {
    struct bb_midi_event event={0};
    int framec=bb_midi_file_reader_update(&event,ctx->reader);
    if (framec<0) {
      if (bb_midi_file_reader_is_complete(ctx->reader)) break;
      fprintf(stderr,"%s: Error playing MIDI file.\n",ctx->srcpath);
      return -1;
    }
    if (framec) {
      if (mid2bba_receive_delay(ctx,framec)<0) return -1;
      if (bb_midi_file_reader_advance(ctx->reader,framec)<0) return -1;
    } else {
      if (mid2bba_receive_event(ctx,&event)<0) return -1;
    }
  }
  if (mid2bba_flush_delay(ctx)<0) return -1;
  
  mid2bba_trim_edge_delays(ctx);
  
  return 0;
}

/* Main.
 *
 
int main_mid2bba(char **argv,int argc) {

  if ((argc!=2)||memcmp(argv[0],"-o",2)||(argv[1][0]=='-')) {
    fprintf(stderr,"Usage: beepbot mid2bba -oOUTPUT INPUT\n");
    return 1;
  }
  struct mid2bba_context ctx={
    .dstpath=argv[0]+2,
    .srcpath=argv[1],
  };
  
  if ((ctx.midic=bb_file_read(&ctx.midi,ctx.srcpath))<0) {
    fprintf(stderr,"%s: Failed to read file.\n",ctx.srcpath);
    mid2bba_context_cleanup(&ctx);
    return 1;
  }
  
  if (mid2bba(&ctx)<0) {
    fprintf(stderr,"%s: Failed to reencode song.\n",ctx.srcpath);
    mid2bba_context_cleanup(&ctx);
    return 1;
  }
  
  if (bb_file_write(ctx.dstpath,ctx.dst.v,ctx.dst.c)<0) {
    fprintf(stderr,"%s: Failed to write file.\n",ctx.dstpath);
    mid2bba_context_cleanup(&ctx);
    return 1;
  }
  
  mid2bba_context_cleanup(&ctx);
  return 0;
}
/**/

/* Main, for Multiarcade.
 */
 
int mid2bba_convert(void *dstpp,const void *src,int srcc,const char *srcpath) {
  struct mid2bba_context ctx={
    .srcpath=srcpath,
    .midi=src,
    .midic=srcc,
  };
  if (mid2bba(&ctx)<0) {
    mid2bba_context_cleanup(&ctx);
    return -1;
  }
  *(void**)dstpp=ctx.dst.v; // HANDOFF
  ctx.dst.v=0;
  int dstc=ctx.dst.c;
  mid2bba_context_cleanup(&ctx);
  return dstc;
}
