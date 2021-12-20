#ifndef SOUND_H
#define SOUND_H

// Main must call this each frame, to enable margins.
void sound_frame();

void sound_ui_toggle();
void sound_jump();
void sound_jump_down();
void sound_pickup();
void sound_toss(); // up or +velocity
void sound_drop(); // down or no-velocity
void sound_kill();
void sound_crocbot_chomp();

// A sound for hitting the ground might be nice, but we don't track that conveniently.

#endif
