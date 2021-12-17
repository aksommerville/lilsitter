#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "common/fs.h"
#include "common/encoder.h"

/* Decode VLQ.
 */

int vlq_decode(int *dst,const void *src,int srcc) {
  const uint8_t *SRC=src;
  if (srcc<1) return -1;
  if (!(SRC[0]&0x80)) {
    *dst=SRC[0];
    return 1;
  }
  if (srcc<2) return -1;
  if (!(SRC[1]&0x80)) {
    *dst=((SRC[0]&0x7f)<<7)|SRC[1];
    return 2;
  }
  if (srcc<3) return -1;
  if (!(SRC[2]&0x80)) {
    *dst=((SRC[0]&0x7f)<<14)|((SRC[1]&0x7f)<<7)|SRC[2];
    return 3;
  }
  if (srcc<4) return -1;
  if (!(SRC[3]&0x80)) {
    *dst=((SRC[0]&0x7f)<<21)|((SRC[1]&0x7f)<<14)|((SRC[2]&0x7f)<<7)|SRC[3];
    return 4;
  }
  return -1;
}

/* MIDI file tracks.
 */
 
struct track {
  const uint8_t *v;
  int c;
  int p;
  uint8_t status;
  int delay; // <0 if we haven't read it yet
};

/* If this track doesn't have a delay, read it from the stream and store in (track->delay).
 */
 
static int require_delay(struct track *track) {
  if (track->delay>=0) return 0;
  int delay,err;
  if ((err=vlq_decode(&delay,track->v+track->p,track->c-track->p))<1) return -1;
  track->p+=err;
  track->delay=delay;
  return 0;
}

/* Decode payload and emit converted event, for the two events we care about.
 */
 
static int read_note_off(struct encoder *dst,struct track *track,uint8_t *outstate) {
  if (track->p>track->c-2) return -1;
  uint8_t noteid=track->v[track->p];
  track->p+=2;
  track->delay=-1;
  
  // Terminate all voices playing this note.
  int i=0; for (;i<4;i++) {
    if (outstate[i]==noteid) {
      uint8_t subnoteid=noteid;
      switch (i) {
        case 0: subnoteid-=0x20; break;
        case 1: subnoteid-=0x28; break;
        case 2: subnoteid-=0x30; break;
        case 3: subnoteid-=0x38; break;
      }
      uint8_t encoded=0x80|(i<<5)|subnoteid;
      if (encoder_append(dst,&encoded,1)<0) return -1;
      outstate[i]=0;
    }
  }
  
  return 0;
}

static int read_note_on(struct encoder *dst,struct track *track,uint8_t *outstate) {
  if (track->p>track->c-2) return -1;
  
  // Note On with velocity zero is equivalent to Note Off with velocity 0x40.
  // The two events are formatted the same, and as luck would have it, we ignore velocity otherwise.
  if (!track->v[track->p+1]) return read_note_off(dst,track,outstate);
  
  uint8_t noteid=track->v[track->p];
  track->p+=2;
  track->delay=-1;
  
  // Now it gets a little weird. Not every note is expressible, and each output voice has a different range.
  const struct { int lo,hi; } voicerange[4]={
    {0x21,0x20+0x1f}, // These need to match ma_synth_halfperiod_for_note().
    {0x29,0x28+0x1f},
    {0x31,0x30+0x1f},
    {0x39,0x38+0x1f},
  };
  int valid[4]={0};
  int validc=0;
  int i=4; while (i-->0) {
    if ((noteid>=voicerange[i].lo)&&(noteid<=voicerange[i].hi)) {
      valid[i]=1;
      validc++;
    }
  }
  if (!validc) { // can't do it
    fprintf(stderr,"WARNING: MIDI note 0x%02x not representable.\n",noteid);
    return 0;
  }
  
  // Prefer a voice with nothing on it yet.
  for (i=0;i<4;i++) {
    if (valid[i]&&!outstate[i]) {
      uint8_t encoded=0x80|(i<<5)|(noteid-voicerange[i].lo+1);
      if (encoder_append(dst,&encoded,1)<0) return -1;
      outstate[i]=noteid;
      return 0;
    }
  }
  // Replace a playing note if we have to.
  for (i=0;i<4;i++) {
    if (valid[i]) {
      uint8_t encoded=0x80|(i<<5)|(noteid-voicerange[i].lo+1);
      if (encoder_append(dst,&encoded,1)<0) return -1;
      outstate[i]=noteid;
      return 0;
    }
  }
  // Should have hit one of the above.
  return -1;
}

/* Read one event (not the delay), update track status, and emit events if warranted.
 */
 
static int read_event(struct encoder *dst,struct track *track,uint8_t *outstate) {
  if (track->p>=track->c) return -1;
  
  // Status byte.
  if (track->v[track->p]&0x80) {
    track->status=track->v[track->p++]; // could be meta, sysex, etc, we'll deal with that soon
  } else if (!track->status) {
    fprintf(stderr,"Unexpected leading byte 0x%02x at %d/%d in MTrk.\n",track->v[track->p],track->p,track->c);
    return -1;
  }
  
  // Payload.
  switch (track->status&0xf0) {
    case 0x80: return read_note_off(dst,track,outstate);
    case 0x90: return read_note_on(dst,track,outstate);
    case 0xa0: track->p+=2; break; // Note Adjust, not meaningful to us.
    case 0xb0: track->p+=2; break; // Control Change, not meaningful to us.
    case 0xc0: track->p+=1; break; // Program Change, not meaningful to us.
    case 0xd0: track->p+=1; break; // Channel Pressure, not meaningful to us.
    case 0xe0: track->p+=2; break; // Pitch Wheel, ha, you wish.
    default: {
        uint8_t prefix=track->status;
        track->status=0;
        switch (prefix) {
        
          case 0xff: {
              // Meta: We ignore everything, but still must measure it properly.
              //TODO Tempo?
              if (track->p>=track->c) return -1;
              uint8_t opcode=track->v[track->p++];
              int err,paylen;
              if ((err=vlq_decode(&paylen,track->v+track->p,track->c-track->p))<1) return -1;
              track->p+=err;
              if (track->p>track->c-paylen) return -1;
              track->p+=paylen;
              if (opcode==0x2f) { // explicit termination
                track->p=track->c;
              }
            } break;
            
          case 0xf0: case 0xf7: {
              // Sysex: Like Meta, all we do is measure and skip.
              int err,paylen;
              if ((err=vlq_decode(&paylen,track->v+track->p,track->c-track->p))<1) return -1;
              track->p+=err;
              if (track->p>track->c-paylen) return -1;
              track->p+=paylen;
            } break;
            
          default: {
              fprintf(stderr,"Unexpected leading byte 0x%02x at %d/%d in MTrk.\n",prefix,track->p-1,track->c);
              return -1;
            }
        }
      }
  }
  
  track->delay=-1;
  return 0;
}

/* Convert MIDI file to our private format -- just the binary.
 */
 
static int song_from_midi(struct encoder *dst,const void *src,int srcc) {

  // Locate the MTrk chunks.
  const uint8_t *SRC=src;
  int srcp=0;
  #define TRACK_LIMIT 16
  struct track trackv[TRACK_LIMIT];
  int trackc=0;
  while (srcp<=srcc-8) {
    const uint8_t *chunkid=SRC+srcp; srcp+=4;
    uint32_t chunklen=(SRC[srcp]<<24)|(SRC[srcp+1]<<16)|(SRC[srcp+2]<<8)|SRC[srcp+3]; srcp+=4;
    if ((chunklen>0x01000000)||(srcp>srcc-chunklen)) return -1;
    
    if (!memcmp(chunkid,"MThd",4)) {
      //TODO division is here, but we've nowhere to put it
    } else if (!memcmp(chunkid,"MTrk",4)) {
      if (trackc>=TRACK_LIMIT) {
        fprintf(stderr,"Too many MTrk in MIDI file, limit %d.\n",TRACK_LIMIT);
        return -1;
      }
      struct track *track=trackv+trackc++;
      memset(track,0,sizeof(struct track));
      track->v=SRC+srcp;
      track->c=chunklen;
      track->delay=-1;
    }
    
    srcp+=chunklen;
  }
  if (!trackc) {
    fprintf(stderr,"MIDI file doesn't contain any MTrk (maybe it's not MIDI?)\n");
    return -1;
  }
  #undef TRACK_LIMIT
  
  // Play it and re-encode...
  uint8_t outstate[4]={0}; // which *MIDI* note is playing on which voice -- this is the whole output state :)
  while (1) {
    int delay=INT_MAX; // vlq only go up to 0x0fffffff, so INT_MAX means we're done
    struct track *track=trackv;
    int i=trackc;
    for (;i-->0;track++) {
      
      // Track complete?
      if (track->p>=track->c) continue;
      
      // Read delay if we don't have it already.
      if (require_delay(track)<0) return -1;
      
      // Process any immediate events.
      while (!track->delay&&(track->p<track->c)) {
        if (read_event(dst,track,outstate)<0) return -1;
        if (track->p>=track->c) break;
        if (require_delay(track)<0) return -1;
      }
      
      // Again, track complete?
      if (track->p>=track->c) continue;
      
      // Collect the lowest delay.
      if (track->delay<delay) delay=track->delay;
    }
    
    // If we didn't get a master delay, we must be done. Cheers!
    if (delay>0x0fffffff) break;
    
    // Sanity check: delays >127 emit a string of bytes, don't let it be really long.
    const int delay_limit=0x00001000;
    if (delay>delay_limit) {
      fprintf(stderr,"ERROR: Suspiciously long delay %d ticks. Arbitrary limit %d.\n",delay,delay_limit);
      return -1;
    }
    
    // Drop the outgoing delay from all active tracks.
    for (i=trackc,track=trackv;i-->0;track++) {
      if (track->delay>=delay) track->delay-=delay;
    }
    
    // Emit delay, <=127 ticks at a time.
    while (delay>0x7f) {
      const uint8_t full=0x7f;
      if (encoder_append(dst,&full,1)<0) return -1;
      delay-=0x7f;
    }
    uint8_t byte=delay;
    if (encoder_append(dst,&byte,1)<0) return -1;
  }
  
  if (!dst->c) {
    fprintf(stderr,"Converted song but it ended up empty!\n");
    return -1;
  }
  return 0;
}

/* Wrap the converted binary in a C file.
 */
 
static int text_from_song(struct encoder *dst,const void *src,int srcc,const char *name,int namec) {
  if (encoder_appendf(dst,"const int song_%.*s_len=%d;\n",namec,name,srcc)<0) return -1;
  if (encoder_appendf(dst,"const unsigned char song_%.*s[]={\n",namec,name)<0) return -1;
  const uint8_t *SRC=src;
  int srcp=0;
  while (srcp<srcc) {
    int i=32;
    if (srcp+i>srcc) i=srcc-srcp;
    if (encoder_append(dst,"  ",2)<0) return -1;
    while (i-->0) {
      if (encoder_appendf(dst,"0x%02x,",SRC[srcp++])<0) return -1;
    }
    if (encoder_append(dst,"\n",1)<0) return -1;
  }
  if (encoder_append(dst,"};\n",3)<0) return -1;
  return 0;
}

/* Convert file.
 */
 
static int text_from_midi(void *dstpp,const void *src,int srcc,const char *name,int namec) {
  
  struct encoder bin={0};
  if (song_from_midi(&bin,src,srcc)<0) {
    encoder_cleanup(&bin);
    return -1;
  }
  
  struct encoder text={0};
  if (text_from_song(&text,bin.v,bin.c,name,namec)<0) {
    encoder_cleanup(&bin);
    encoder_cleanup(&text);
    return -1;
  }
  encoder_cleanup(&bin);
  
  *(void**)dstpp=text.v;//handoff
  return text.c;
}

/* Main.
 */
 
int main(int argc,char **argv) {

  const char *dstpath=0,*srcpath=0,*name=0;
  int argp=1;
  for (;argp<argc;argp++) {
    if (!memcmp(argv[argp],"-o",2)) {
      if (dstpath) {
        fprintf(stderr,"%s: Multiple output paths\n",argv[0]);
        return 1;
      }
      dstpath=argv[argp]+2;
    } else if (!memcmp(argv[argp],"-n",2)) {
      if (name) {
        fprintf(stderr,"%s: Multiple names\n",argv[0]);
        return 1;
      }
      name=argv[argp]+2;
    } else if (!argv[argp][0]||(argv[argp][0]=='-')) {
      fprintf(stderr,"%s: Unexpected argument '%s'\n",argv[0],argv[argp]);
      return 1;
    } else if (srcpath) {
      fprintf(stderr,"%s: Multiple input paths\n",argv[0]);
      return 1;
    } else {
      srcpath=argv[argp];
    }
  }
  if (!dstpath||!srcpath) {
    fprintf(stderr,"Usage: %s -oOUTPUT INPUT [-nNAME]\n",argv[0]);
    fprintf(stderr,"  OUTPUT is a C file, INPUT is a MIDI file.\n");
    return 1;
  }
  
  int namec=0;
  if (!name) {
    name=srcpath;
    int i=0; for (;srcpath[i];i++) if (srcpath[i]=='/') name=srcpath+i+1;
  }
  while (name[namec]&&(name[namec]!='.')) namec++;
  
  void *src=0;
  int srcc=file_read(&src,srcpath);
  if (srcc<0) {
    fprintf(stderr,"%s: Failed to read file\n",srcpath);
    return 1;
  }
  
  void *dst=0;
  int dstc=text_from_midi(&dst,src,srcc,name,namec);
  free(src);
  if (dstc<0) {
    fprintf(stderr,"%s: Failed to convert song\n",srcpath);
    return 1;
  }
  
  int err=file_write(dstpath,dst,dstc);
  free(dst);
  if (err<0) {
    fprintf(stderr,"%s: Failed to write file\n",dstpath);
    return 1;
  }
  return 0;
}
