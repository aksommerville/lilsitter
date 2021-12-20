#include "sound.h"
#include "bba.h"
#include "multiarcade.h"

// main.c
extern struct bba_synth synth;

#define SFX_CHID 0x0f

/* Prevent sound effects playing too close together.
 * Require 4 frames of margin: 60/4=15 Hz.
 * In particular, it should be at least 1, so when two sprites 
 * die at the same time the sound is not doubled in amplitude.
 */
#define SOUND_MARGIN 4
static uint32_t sound_framec=0;
static uint32_t sound_recent_frame=0;

void sound_frame() {
  sound_framec++;
}

/* Channel properties, from bb/etc/doc/bba-song-format.txt:
           0x00 Reset: 0=clear, 1=default
           0x01 Sustain (0,1)
           0x02 Attack time low (0..255 +1 ms)
           0x03 Attack time high (0..255 +1 ms)
           0x04 Attack level low (0..255)
           0x05 Attack level high (0..255)
           0x06 Decay time low (0..255 +1 ms)
           0x07 Decay time high (0..255 +1 ms)
           0x08 Sustain level low (0..255)
           0x09 Sustain level high (0..255)
           0x0a Release time low (0..255 *4 ms)
           0x0b Release time high (0..255 *4 ms)
           0x0c Shape (0,1,2)=(silent,square,saw)
           0x0d Note base (0..64), natural note zero corresponds to this MIDI note.
With more added here:
           0x0e Pitch shift, signed
*/

static void sfx(
  uint8_t shape, // BBA_VOICE_SHAPE_(SQUARE,SAW)
  uint8_t midinoteid,
  int8_t shift,
  uint8_t attack_time,
  uint8_t attack_level,
  uint8_t decay_time,
  uint8_t decay_level,
  uint8_t release_time
) {
  if (sound_recent_frame+SOUND_MARGIN>sound_framec) return;
  sound_recent_frame=sound_framec;
  struct bba_channel *channel=synth.channelv+SFX_CHID;
  bba_channel_set_property(&synth,channel,0x00,0x01); // reset
  bba_channel_set_property(&synth,channel,0x01,0x00); // sustain
  bba_channel_set_property(&synth,channel,0x0c,shape);
  bba_channel_set_property(&synth,channel,0x03,attack_time);
  bba_channel_set_property(&synth,channel,0x05,attack_level);
  bba_channel_set_property(&synth,channel,0x07,decay_time);
  bba_channel_set_property(&synth,channel,0x09,decay_level);
  bba_channel_set_property(&synth,channel,0x0b,release_time);
  bba_channel_set_property(&synth,channel,0x0e,shift);
  bba_channel_set_property(&synth,channel,0x0d,midinoteid);
  bba_synth_note(&synth,SFX_CHID,0x00,0xff);
}

/* Sound effects definitions.
 */

void sound_ui_toggle() {
  sfx(
    BBA_VOICE_SHAPE_SAW,
    0x30,40,
    0x10,0xff,
    0x10,0x40,
    0x40
  );
}

void sound_jump() {
  sfx(
    BBA_VOICE_SHAPE_SQUARE,
    0x30,80,
    0x10,0xff,
    0x10,0x40,
    0x40
  );
}

void sound_jump_down() {
  sfx(
    BBA_VOICE_SHAPE_SQUARE,
    0x40,-80,
    0x10,0xff,
    0x10,0x40,
    0x40
  );
}

void sound_pickup() {
  sfx(
    BBA_VOICE_SHAPE_SAW,
    0x20,100,
    0x20,0xff,
    0x10,0x40,
    0x40
  );
}

void sound_toss() {
  sfx(
    BBA_VOICE_SHAPE_SAW,
    0x50,-80,
    0x10,0xff,
    0x10,0x40,
    0x80
  );
}

void sound_drop() {
  sfx(
    BBA_VOICE_SHAPE_SQUARE,
    0x40,-120,
    0x10,0xff,
    0x10,0x40,
    0x40
  );
}

void sound_kill() {
  sfx(
    BBA_VOICE_SHAPE_SQUARE,
    0x20,-10,
    0x18,0xff,
    0x10,0x40,
    0x60
  );
}

void sound_crocbot_chomp() {
  sfx(
    BBA_VOICE_SHAPE_SAW,
    0x20,-90,
    0x18,0xff,
    0x10,0x40,
    0x40
  );
}
