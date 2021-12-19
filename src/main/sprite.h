/* sprite.h
 */
 
#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

struct ma_framebuffer;
struct ma_texture;

// This can grow but never beyond 255.
#define SPRITE_LIMIT 32

#define SPRITE_TYPE_DUMMY    0
#define SPRITE_TYPE_HERO     1
#define SPRITE_TYPE_SUSIE    2
#define SPRITE_TYPE_SOLID    3
#define SPRITE_TYPE_ONEWAY   4
#define SPRITE_TYPE_FIRE     5
#define SPRITE_TYPE_CROCBOT  6
#define SPRITE_TYPE_PLATFORM 7
#define SPRITE_TYPE_SHREDDER 8
#define SPRITE_TYPE_BALLOON  9

#define SPRITE_CONSTRAINT_N   0x01
#define SPRITE_CONSTRAINT_W   0x02
#define SPRITE_CONSTRAINT_E   0x04
#define SPRITE_CONSTRAINT_S   0x08

extern struct sprite {
  int8_t x,y,w,h;
  uint8_t type;
  int8_t vs8[8]; // initially zero; usage depends on type
  uint8_t constraint;
  uint8_t visible;
  uint8_t physics;
  uint8_t mobile;
  uint8_t target;
  uint8_t gravity;
  uint8_t dead;
  int8_t dx,dy; // most recent move
  int8_t pvx,pvy;
  const struct ma_texture *texture;
  uint8_t xform;
  int8_t autodx,autody; // frame count +- to pay out motion in that direction.
} spritev[SPRITE_LIMIT];
extern uint8_t spritec;

/* Clear the current sprite list and rebuild from map.
 */
void spawn_sprites();

void update_sprites(uint16_t input);
void draw_sprites(struct ma_framebuffer *dst);

uint8_t sprite_is_on_goal(const struct sprite *sprite);

#endif
