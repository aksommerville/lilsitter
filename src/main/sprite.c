#include "multiarcade.h"
#include "sprite.h"
#include "map.h"

extern struct ma_texture tex_sprites_00; // desmond...
extern struct ma_texture tex_sprites_01;
extern struct ma_texture tex_sprites_10; // susie...
extern struct ma_texture tex_sprites_11;
extern struct ma_texture tex_sprites_12;
extern struct ma_texture tex_sprites_20; // head...
extern struct ma_texture tex_sprites_21;
extern struct ma_texture tex_sprites_22;
extern struct ma_texture tex_sprites_30; // torso...
extern struct ma_texture tex_sprites_31;
extern struct ma_texture tex_sprites_32;
extern struct ma_texture tex_sprites_33; // arms...
extern struct ma_texture tex_sprites_34;
extern struct ma_texture tex_sprites_35;
extern struct ma_texture tex_sprites_36; // raised arms
extern struct ma_texture tex_sprites_40; // fire...
extern struct ma_texture tex_sprites_41;
extern struct ma_texture tex_sprites_42;
extern struct ma_texture tex_sprites_50; // turnip
extern struct ma_texture tex_sprites_51; // tomato
extern struct ma_texture tex_sprites_52; // pumpkin
extern struct ma_texture tex_sprites_53; // cabbage
extern struct ma_texture tex_sprites_60; // crocbot...
extern struct ma_texture tex_sprites_61;
extern struct ma_texture tex_sprites_62;
extern struct ma_texture tex_sprites_63;
extern struct ma_texture tex_sprites_64; // shredder...
extern struct ma_texture tex_sprites_65;
extern struct ma_texture tex_sprites_66;
extern struct ma_texture tex_sprites_70; // floating platform
extern struct ma_texture tex_sprites_71; // balloon

struct sprite spritev[SPRITE_LIMIT];
uint8_t spritec=0;

/* Spawn one sprite.
 */
 
static struct sprite *spawn() {
  if (spritec>=SPRITE_LIMIT) {
    ma_log("Exceeded sprite limit.\n");
    return 0;
  }
  struct sprite *sprite=spritev+spritec++;
  memset(sprite,0,sizeof(struct sprite));
  sprite->gravity=1;
  return sprite;
}

/* Spawn sprites.
 */
 
void spawn_sprites() {
  spritec=0;
  uint8_t goal=0;
  struct sprite *sprite;
  const uint8_t *src=map;
  while (1) {
    switch (*src++) {
      case MAP_CMD_EOF: return;
      case MAP_CMD_NOOP1: break;
      case MAP_CMD_NOOPN: src+=*src++; break;
      case MAP_CMD_BGCOLOR: src+=1; break;
      
      case MAP_CMD_SOLID: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_SOLID;
            sprite->x=src[0];
            sprite->y=src[1];
            sprite->w=src[2];
            sprite->h=src[3];
            sprite->physics=1;
            sprite->vs8[0]=goal;
          }
          src+=4;
        } break;
      
      case MAP_CMD_ONEWAY: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_ONEWAY;
            sprite->x=src[0];
            sprite->y=src[1];
            sprite->w=src[2];
            sprite->h=1;
            sprite->physics=1;
            sprite->vs8[0]=goal;
          }
          src+=3;
        } break;
        
      case MAP_CMD_GOAL: goal=1; continue;
      
      case MAP_CMD_HERO: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_HERO;
            sprite->w=8;
            sprite->h=12;
            sprite->x=src[0]-(sprite->w>>1);
            sprite->y=src[1]-sprite->h;
            sprite->visible=1;
            sprite->physics=1;
            sprite->mobile=1;
            sprite->target=1;
            sprite->vs8[4]=-1;
          }
          src+=2;
        } break;
        
      case MAP_CMD_DESMOND: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_DUMMY;
            sprite->w=8;
            sprite->h=8;
            sprite->x=src[0]-(sprite->w>>1);
            sprite->y=src[1]-sprite->h;
            sprite->visible=1;
            sprite->physics=1;
            sprite->mobile=1;
            sprite->target=1;
            sprite->texture=&tex_sprites_00;
          }
          src+=2;
        } break;
        
      case MAP_CMD_SUSIE: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_SUSIE;
            sprite->w=8;
            sprite->h=8;
            sprite->x=src[0]-(sprite->w>>1);
            sprite->y=src[1]-sprite->h;
            sprite->visible=1;
            sprite->physics=1;
            sprite->mobile=1;
            sprite->target=1;
            sprite->texture=&tex_sprites_10;
          }
          src+=2;
        } break;
        
      case MAP_CMD_FIRE: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_FIRE;
            sprite->w=8;
            sprite->h=8;
            sprite->x=src[0]-(sprite->w>>1);
            sprite->y=src[1]-sprite->h;
            sprite->visible=1;
            sprite->physics=0;
            sprite->mobile=0;
            sprite->texture=&tex_sprites_40;
          }
          src+=2;
        } break;
        
      case MAP_CMD_DUMMY: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_DUMMY;
            sprite->w=8;
            sprite->h=8;
            sprite->x=src[0]-(sprite->w>>1);
            sprite->y=src[1]-sprite->h;
            sprite->visible=1;
            sprite->physics=1;
            sprite->mobile=1;
            switch (src[2]) {
              case 0x50: sprite->texture=&tex_sprites_50; break;
              case 0x51: sprite->texture=&tex_sprites_51; break;
              case 0x52: sprite->texture=&tex_sprites_52; break;
              case 0x53: sprite->texture=&tex_sprites_53; break;
              default: sprite->texture=&tex_sprites_50; break;
            }
          }
          src+=3;
        } break;
        
      case MAP_CMD_CROCBOT: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_CROCBOT;
            sprite->w=16;
            sprite->h=5;
            sprite->x=src[0]-12;
            sprite->y=src[1]-sprite->h;
            sprite->visible=1;
            sprite->physics=1;
            sprite->mobile=0;
          }
          src+=2;
        } break;
        
      case MAP_CMD_PLATFORM: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_PLATFORM;
            sprite->w=16;
            sprite->h=6;
            sprite->x=src[0]-8;
            sprite->y=src[1]-8;
            sprite->visible=1;
            sprite->physics=1;
            sprite->mobile=1;
            sprite->gravity=0;
            switch (src[2]) {
              case 1: sprite->vs8[0]=1; break;
              case 2: sprite->vs8[1]=1; break;
            }
          }
          src+=3;
        } break;
        
      case MAP_CMD_SHREDDER: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_SHREDDER;
            sprite->w=8;
            sprite->h=src[3];
            if (sprite->h<16) sprite->h=16;
            sprite->x=src[0]-4;
            sprite->y=src[1]; // y is the top, unlike most sprites
            sprite->visible=1;
            sprite->physics=0;
            sprite->mobile=0;
            sprite->gravity=0;
            sprite->vs8[0]=src[2];
          }
          src+=4;
        } break;
        
      case MAP_CMD_BALLOON: {
          if (sprite=spawn()) {
            sprite->type=SPRITE_TYPE_BALLOON;
            sprite->w=8;
            sprite->h=8;
            sprite->x=src[0]-(sprite->w>>1);
            sprite->y=src[1]-sprite->h;
            sprite->visible=1;
            sprite->physics=1;
            sprite->mobile=1;
            sprite->gravity=0;
            sprite->texture=&tex_sprites_71;
          }
          src+=2;
        } break;
        
      case MAP_CMD_SONG: src+=1; break;
        
      default: return;
    }
    goal=0;
  }
}

/* Kill sprite.
 */
 
static uint8_t sprite_produces_gore(struct sprite *sprite) {
  switch (sprite->type) {
    case SPRITE_TYPE_BALLOON:
      return 0;
  }
  return 1;
}
 
static void kill_sprite(struct sprite *victim,struct sprite *assailant) {

  // Confirm it really is killable.
  switch (victim->type) {
    case SPRITE_TYPE_HERO:
    case SPRITE_TYPE_DUMMY:
    case SPRITE_TYPE_SUSIE:
    case SPRITE_TYPE_BALLOON:
      break;
    default: return;
  }
  
  // If it's the hero, and she's carrying a pumpkin, release it.
  if (victim->type==SPRITE_TYPE_HERO) {
    if ((victim->vs8[4]>=0)&&(victim->vs8[4]<spritec)) {
      struct sprite *pumpkin=spritev+victim->vs8[4];
      victim->vs8[4]=-1;
      pumpkin->x=victim->x;
      pumpkin->y=victim->y;
      pumpkin->visible=1;
      pumpkin->physics=1;
      victim->y+=8;
      victim->h-=8;
    }
  }
  
  if (sprite_produces_gore(victim)) {
    map_add_gore(victim->x,victim->y,victim->w,victim->h);
  }
  victim->visible=0;
  victim->physics=0;
  victim->dead=1;
}

/* Check for pumpkins and pick one up if possible.
 * Caller ensures there is no current pumpkin (ie vs8[4]<0).
 */
 
static void pickup(struct sprite *sprite,uint16_t input) {

  // Confirm we have head room.
  // Don't abort just yet. If we're pointing up, a pumpkin would trip this condition.
  // Do abort if we are within 8 pixels of the top edge: Picking up simply not possible.
  struct sprite *headblock=0;
  if (!(input&(MA_BUTTON_UP|MA_BUTTON_DOWN))) {
    if (sprite->y<8) return;
    struct sprite *q=spritev;
    uint8_t i=spritec;
    for (;i-->0;q++) {
      if (q==sprite) continue;
      if (!q->physics) continue;
      if (q->dead) continue;
      if (q->type==SPRITE_TYPE_ONEWAY) continue;
      if (q->x+q->w<=sprite->x) continue;
      if (q->y+q->h<=sprite->y-8) continue;
      if (q->x>=sprite->x+sprite->w) continue;
      if (q->y>=sprite->y) continue;
      headblock=q;
      break;
    }
  }
  
  // Measure the pumpkin patch.
  const uint8_t patchw=2;
  uint8_t x=sprite->x,y=sprite->y,w=sprite->w,h=sprite->h;
  if (input&MA_BUTTON_DOWN) {
    y+=h;
    h=patchw;
  } else if (input&MA_BUTTON_UP) {
    h=patchw;
    y-=h;
  } else if (sprite->xform&MA_XFORM_XREV) {
    w=patchw;
    x-=w;
  } else {
    x+=w;
    w=patchw;
  }
  
  // Find a pumpkin in the patch.
  // If there's more than one, take the one with more overlap.
  struct sprite *pumpkin=0;
  struct sprite *q=spritev;
  uint8_t score=0;
  uint8_t i=spritec;
  for (;i-->0;q++) {
    if (!q->visible) continue;
    if (q->dead) continue;
    switch (q->type) {
      case SPRITE_TYPE_DUMMY:
      case SPRITE_TYPE_SUSIE:
      case SPRITE_TYPE_BALLOON:
        break;
      default: continue;
    }
    int8_t penn,penw,pene,pens;
    if ((penn=q->y+q->h-y)<=0) continue;
    if ((penw=q->x+q->w-x)<=0) continue;
    if ((pene=x+w-q->x)<=0) continue;
    if ((pens=y+h-q->y)<=0) continue;
    uint8_t overlap=((penn<pens)?penn:pens)*((penw<pene)?penw:pene);
    if (overlap>score) {
      pumpkin=q;
      score=overlap;
    }
  }
  if (!pumpkin) {
    return;
  }
  if (headblock&&(headblock!=pumpkin)) {
    return;
  }
  
  // Commit the change.
  pumpkin->visible=0;
  pumpkin->physics=0;
  sprite->vs8[4]=pumpkin-spritev;
  sprite->y-=8;
  sprite->h+=8;
  pumpkin->pvx=pumpkin->x;
  pumpkin->pvy=pumpkin->y;
  sprite->pvx=sprite->x;
  sprite->pvy=sprite->y;
  pumpkin->dx=0;
  pumpkin->dy=0;
  sprite->dx=0;
  sprite->dy=0;
}

/* Toss the current pumpkin.
 */
 
static void toss(struct sprite *sprite,uint16_t input) {
  if ((sprite->vs8[4]<0)||(sprite->vs8[4]>=spritec)) return;
  struct sprite *pumpkin=spritev+sprite->vs8[4];
  sprite->vs8[4]=-1;
  
  pumpkin->x=sprite->x;
  pumpkin->visible=1;
  pumpkin->physics=1;
  sprite->y+=8;
  sprite->h-=8;
  // Cheat the previous positions too, to fool physics.
  pumpkin->pvx=pumpkin->x;
  pumpkin->pvy=pumpkin->y;
  sprite->pvx=sprite->x;
  sprite->pvy=sprite->y;
  pumpkin->dx=0;
  pumpkin->dy=0;
  sprite->dx=0;
  sprite->dy=0;
  
  if (input&MA_BUTTON_UP) {
    pumpkin->y=sprite->y-pumpkin->h;
    pumpkin->autody=-10;
  } else if (input&MA_BUTTON_DOWN) {
    sprite->y-=pumpkin->h;
    pumpkin->y=sprite->y+sprite->h;
  } else {
    pumpkin->y=sprite->y-pumpkin->h+4;
    if (sprite->xform&MA_XFORM_XREV) {
      pumpkin->x-=pumpkin->w;
      if (input&MA_BUTTON_LEFT) {
        pumpkin->autodx=-10;
      }
    } else {
      pumpkin->x+=sprite->w;
      if (input&MA_BUTTON_RIGHT) {
        pumpkin->autodx=10;
      }
    }
  }
}

/* Check foot neighbors.
 */
 
static uint8_t sprite_is_grounded(const struct sprite *sprite) {
  uint8_t y=sprite->y+sprite->h;
  if (y>=64) return 1;
  const struct sprite *q=spritev;
  uint8_t i=spritec;
  for (;i-->0;q++) {
    if (!q->physics) continue;
    int8_t dy=q->y-y;
    if (dy<-1) continue;
    if (dy>1) continue;
    if (q->x>=sprite->x+sprite->w) continue;
    if (q->x+q->w<=sprite->x) continue;
    return 1;
  }
  return 0;
}

/* Update hero.
 * vs8[0]=animation counter
 * vs8[1]=animation frame
 * vs8[2]=jump time available
 * vs8[3]=previous input
 * vs8[4]=pumpkin index
 * vs8[5]=head (0,1,2)=(normal,down,up)
 */
 
static void update_hero(struct sprite *sprite,uint16_t input) {

  // If carrying a balloon, float up every other frame. ("up", i mean cancel gravity)
  if ((sprite->vs8[0]&3)&&(sprite->vs8[4]>=0)&&(sprite->vs8[4]<spritec)&&(spritev[sprite->vs8[4]].type==SPRITE_TYPE_BALLOON)) {
    if (!sprite_is_grounded(sprite)) {
      sprite->y--;
    }
  }

  // Animate walking.
  sprite->vs8[0]++;
  if (sprite->vs8[0]>=8) {
    sprite->vs8[0]=0;
    sprite->vs8[1]++;
    sprite->vs8[1]&=3;
  }
  
  // Point head in the proper direction.
  if (input&MA_BUTTON_UP) sprite->vs8[5]=2;
  else if (input&MA_BUTTON_DOWN) sprite->vs8[5]=1;
  else sprite->vs8[5]=0;

  // Walk left or right, or reset animation.
  if (input&MA_BUTTON_LEFT) {
    sprite->x--;
    sprite->xform|=MA_XFORM_XREV;
  } else if (input&MA_BUTTON_RIGHT) {
    sprite->x++;
    sprite->xform&=~MA_XFORM_XREV;
  } else {
    sprite->vs8[1]=0;
  }
  
  // Down jump?
  if (
    (input&MA_BUTTON_A)&&!(sprite->vs8[3]&MA_BUTTON_A)&&
    (input&MA_BUTTON_DOWN)&&
    (sprite->constraint&SPRITE_CONSTRAINT_S)
  ) {
    // Cheat her down one pixel. If we enter a solid, physics will correct it.
    // But if it's a oneway, this will put us on the other side.
    sprite->y++;
    sprite->pvy++;
    sprite->vs8[2]=0;
  
  // Regular jump.
  } else if (input&MA_BUTTON_A) {
    if (sprite->vs8[2]>0) {
      sprite->vs8[2]--;
      if (sprite->vs8[2]>10) sprite->y-=3;
      else if (sprite->vs8[2]>5) sprite->y-=2;
      else sprite->y-=1; // -1 is effectively motionless
    }
  } else if (sprite->dy<0) {
    sprite->vs8[2]=0;
  } else if (sprite_is_grounded(sprite)) {
    sprite->vs8[2]=18; // Jump power.
  } else {
    sprite->vs8[2]=0;
  }
  
  // Pickup/toss?
  if ((input&MA_BUTTON_B)&&!(sprite->vs8[3]&MA_BUTTON_B)) {
    if (sprite->vs8[4]>=0) {
      toss(sprite,input);
    } else {
      pickup(sprite,input);
    }
  }
  
  sprite->vs8[3]=input;
}

/* Update susie.
 * vs8[0]=dx
 * vs8[1]=turn-around delay
 * vs8[2]=step delay
 * vs8[3]=stop counter
 * vs8[4]=animation counter
 * vs8[5]=animation frame
 */
 
static uint8_t susie_has_nose_room(const struct sprite *sprite) {
  uint8_t x,y=sprite->y,w=4,h=sprite->h;
  if (sprite->vs8[0]>0) {
    x=sprite->x+sprite->w;
  } else {
    x=sprite->x-w;
  }
  if (x<0) return 0;
  if (x>96-w) return 0;
  const struct sprite *q=spritev;
  uint8_t i=spritec;
  for (;i-->0;q++) {
    if (!q->physics) continue;
    if (q->x>=x+w) continue;
    if (q->y>=y+h) continue;
    if (q->x+q->w<=x) continue;
    if (q->y+q->h<=y) continue;
    return 0;
  }
  return 1;
}
 
static void update_susie(struct sprite *sprite) {

  // If we're being carried, stop doing things.
  if (!sprite->physics) {
    sprite->texture=&tex_sprites_10;
    sprite->vs8[1]=0;
    return;
  }

  // Turn-around delay?
  if (sprite->vs8[1]>0) {
    sprite->vs8[1]--;
    if (!sprite->vs8[1]) {
      if (susie_has_nose_room(sprite)) {
        // Don't change anything, try to keep walking.
      } else {
        sprite->vs8[0]=-sprite->vs8[0];
        sprite->xform^=MA_XFORM_XREV;
      }
    }
    return;
  }
  
  // Stop counter.
  if (sprite->dx) {
    sprite->vs8[3]=0;
  } else if (sprite->vs8[3]<10) { // Threshold for turn-around.
    sprite->vs8[3]++;
  } else {
    sprite->vs8[3]=0;
    sprite->vs8[1]=30; // Turn-around time.
    sprite->texture=&tex_sprites_10;
    return;
  }
  
  // Animation.
  sprite->vs8[4]++;
  if (sprite->vs8[4]>=8) { // Animation frame time.
    sprite->vs8[4]=0;
    sprite->vs8[5]++;
    switch (sprite->vs8[5]&=3) {
      case 0: sprite->texture=&tex_sprites_10; break;
      case 1: sprite->texture=&tex_sprites_11; break;
      case 2: sprite->texture=&tex_sprites_10; break;
      case 3: sprite->texture=&tex_sprites_12; break;
    }
  }
  
  // Motion.
  if (sprite->vs8[0]>0) {
    if (sprite->vs8[2]) {
      sprite->vs8[2]--;
    } else {
      sprite->vs8[2]=1;
      sprite->x++;
    }
  } else if (sprite->vs8[0]<0) {
    if (sprite->vs8[2]) {
      sprite->vs8[2]--;
    } else {
      sprite->vs8[2]=1;
      sprite->x--;
    }
  } else {
    sprite->vs8[0]=1;
  }
}

/* Fire.
 */
 
static void update_fire(struct sprite *sprite) {

  // Animate.
  sprite->vs8[0]++;
  if (sprite->vs8[0]>=6) {
    sprite->vs8[0]=0;
    sprite->vs8[1]++;
    if (sprite->vs8[1]>=6) sprite->vs8[1]=0;
    switch (sprite->vs8[1]) {
      case 0: sprite->texture=&tex_sprites_40; sprite->xform=0; break;
      case 1: sprite->texture=&tex_sprites_41; sprite->xform=0; break;
      case 2: sprite->texture=&tex_sprites_42; sprite->xform=0; break;
      case 3: sprite->texture=&tex_sprites_40; sprite->xform=MA_XFORM_XREV; break;
      case 4: sprite->texture=&tex_sprites_41; sprite->xform=MA_XFORM_XREV; break;
      case 5: sprite->texture=&tex_sprites_42; sprite->xform=MA_XFORM_XREV; break;
    }
  }
  
  // Find victims.
  // Cut some slack on the edges, say 2 pixels each edge?
  const uint8_t slack=2;
  int8_t left=sprite->x+slack;
  int8_t top=sprite->y+slack;
  int8_t right=sprite->x+sprite->w-slack;
  int8_t bottom=sprite->y+sprite->h-slack;
  struct sprite *victim=spritev;
  uint8_t i=spritec;
  for (;i-->0;victim++) {
    if (!victim->visible) continue;
    
    if (victim->x>=right) continue;
    if (victim->y>=top) continue;
    if (victim->x+victim->w<=left) continue;
    if (victim->y+victim->h<=top) continue;
    
    kill_sprite(victim,sprite);
  }
}

/* Crocbot.
 * vs8[0]=delay
 * vs8[1]=phase: 0=wait, 1=raise, 2=waithi, 3=chomp
 * vs8[2]=extension
 */
 
static void update_crocbot(struct sprite *sprite) {
  if (sprite->vs8[0]>0) {
    sprite->vs8[0]--;
  } else switch (sprite->vs8[1]) {
    case 0: {
        sprite->vs8[1]=1;
      } break;
    case 1: {
        if (sprite->vs8[2]<16) {
          sprite->vs8[2]++;
          sprite->vs8[0]=2;
          sprite->y--;
        } else {
          sprite->vs8[1]=2;
          sprite->vs8[0]=90;
        }
      } break;
    case 2: {
        sprite->vs8[1]=3;
      } break;
    case 3: {
        if (sprite->vs8[2]>0) {
          sprite->vs8[2]-=2;
          sprite->y+=2;
        } else {
          sprite->vs8[1]=0;
          sprite->vs8[0]=60;
        }
      } break;
  }
  // Look for victims.
  if (sprite->vs8[1]!=1) {
    struct sprite *victim=spritev;
    uint8_t i=spritec;
    for (;i-->0;victim++) {
      if (!victim->visible) continue;
      if (!victim->mobile) continue;
      if (victim->x>=sprite->x+sprite->w) continue;
      if (victim->x+victim->w<=sprite->x) continue;
      int8_t dy=victim->y-sprite->h-sprite->y;
      if (dy<0) continue;
      if (dy>2) continue;
      kill_sprite(victim,sprite);
    }
  }
}

/* Platform.
 * vs8[0]=dx
 * vs8[1]=dy
 * vs8[2]=still time
 * vs8[3]=counter for animation
 */
 
static void update_platform(struct sprite *sprite) {
  sprite->vs8[3]++;

  // Anything mobile resting on my head, move it too.
  // But use my physics-checked (dx,dy) instead of the optimistic (vs8[0,1]).
  // Also, don't do this when moving up; physics takes care of that.
  if (sprite->dx||(sprite->dy>0)) {
    struct sprite *passenger=spritev;
    uint8_t i=spritec;
    for (;i-->0;passenger++) {
      if (!passenger->mobile) continue;
      if (passenger->x>=sprite->x+sprite->w) continue;
      if (passenger->x+passenger->w<=sprite->x) continue;
      if (passenger->type==SPRITE_TYPE_PLATFORM) continue;
      int8_t dy=passenger->y+passenger->h-sprite->y;
      if ((dy<-1)||(dy>1)) continue;
      passenger->x+=sprite->dx;
      passenger->y+=sprite->dy;
    }
  }

  sprite->x+=sprite->vs8[0];
  sprite->y+=sprite->vs8[1];
  
  if (!sprite->dx&&!sprite->dy) {
    if (sprite->vs8[2]<30) {
      sprite->vs8[2]++;
    } else {
      sprite->vs8[2]=0;
      sprite->vs8[0]*=-1;
      sprite->vs8[1]*=-1;
    }
  } else {
    sprite->vs8[2]=0;
  }
}

/* Shredder.
 * vs8[0]=orient
 * vs8[1]=counter
 */
 
static void update_shredder(struct sprite *sprite) {
  sprite->vs8[1]++;
  
  int8_t w=sprite->x;
  int8_t e=w+(sprite->w>>1);
  int8_t n=sprite->y;
  int8_t s=sprite->y+sprite->h;
  if (sprite->vs8[0]==1) {
    w+=sprite->w>>1;
    e+=sprite->w>>1;
  }
  struct sprite *victim=spritev;
  uint8_t i=spritec;
  for (;i-->0;victim++) {
    if (!victim->visible) continue;
    if (victim->x>=e) continue;
    if (victim->y>=s) continue;
    if (victim->x+victim->w<=w) continue;
    if (victim->y+victim->h<=n) continue;
    kill_sprite(victim,sprite);
  }
}

/* Balloon.
 * vs8[0]=float up counter
 */
 
static void update_balloon(struct sprite *sprite) {
  sprite->vs8[0]++;
  if (sprite->vs8[0]>=5) {
    sprite->vs8[0]=0;
    sprite->y--;
  }
}

/* With the sprite logic all applied, now detect and correct collisions. 
 * Returns nonzero if anything changed.
 */
 
static uint8_t rectify_collisions(uint8_t seed) {
  uint8_t result=0;
  struct sprite *a=spritev;
  uint8_t ai=spritec;
  for (;ai-->0;a++) {
  
    // Skip immobile sprites in slot (a).
    // They'll show up as (b) for every mobile sprite, and they don't need to compare to each other.
    if (!a->mobile) continue;
    if (!a->physics) continue;
    if (a->dead) continue;
    
    // Correct immediately against screen edges.
    if (a->x<0) {
      a->x=0;
      a->constraint|=SPRITE_CONSTRAINT_W;
      result++;
    } else if (a->x+a->w>96) {
      a->x=96-a->w;
      a->constraint|=SPRITE_CONSTRAINT_E;
      result++;
    }
    if (a->y<0) {
      a->y=0;
      a->constraint|=SPRITE_CONSTRAINT_N;
      result++;
    } else if (a->y+a->h>64) {
      a->y=64-a->h;
      a->constraint|=SPRITE_CONSTRAINT_S;
      result++;
    }
    
    struct sprite *b=spritev;
    uint8_t bi=spritec;
    for (;bi-->0;b++) {
    
      if (a==b) continue;
      if (!b->physics) continue;
      if (b->dead) continue;
      if (!a->mobile&&!b->mobile) continue;
      
      // Determine existence and extent of collision.
      int8_t penn,penw,pene,pens;
      if ((penn=b->y+b->h-a->y)<=0) continue;
      if ((penw=b->x+b->w-a->x)<=0) continue;
      if ((pene=a->x+a->w-b->x)<=0) continue;
      if ((pens=a->y+a->h-b->y)<=0) continue;
      
      // One-way platforms are special.
      // They can only be (b), due to (mobile).
      // Only relevant if (a) is moving down and his toes just crossed (b)'s head.
      if (b->type==SPRITE_TYPE_ONEWAY) {
        if (pens>a->dy) continue;
        a->y=b->y-a->h;
        a->constraint|=SPRITE_CONSTRAINT_S;
        result++;
        continue;
      }
      
      // Select the direction of minimum penetration.
      int8_t adx=0,ady=0;
      uint8_t acon,bcon;
      if ((penn<=penw)&&(penn<=pene)&&(penn<=pens)) {
        ady=penn;
        acon=SPRITE_CONSTRAINT_S;
        bcon=SPRITE_CONSTRAINT_N;
      } else if ((pens<=pene)&&(pens<=penw)) {
        ady=-pens;
        acon=SPRITE_CONSTRAINT_N;
        bcon=SPRITE_CONSTRAINT_S;
      } else if (penw<=pene) {
        adx=penw;
        acon=SPRITE_CONSTRAINT_E;
        bcon=SPRITE_CONSTRAINT_W;
      } else {
        adx=-pene;
        acon=SPRITE_CONSTRAINT_W;
        bcon=SPRITE_CONSTRAINT_E;
      }
      if ((a->constraint&acon)&&(b->constraint&bcon)) continue;
      
      // Apportion the correction between them.
      int8_t bdx=0,bdy=0;
      if (!a->mobile) {
        bdx=-adx; adx=0;
        bdy=-ady; ady=0;
        b->constraint|=acon;
      } else if (!b->mobile) {
        a->constraint|=bcon;
      } else if (a->constraint&acon) {
        bdx=-adx; adx=0;
        bdy=-ady; ady=0;
        b->constraint|=acon;
      } else if (b->constraint&bcon) {
        a->constraint|=bcon;
      } else if (a->type==SPRITE_TYPE_PLATFORM) {
        bdx=-adx; adx=0;
        bdy=-ady; ady=0;
      } else if (b->type==SPRITE_TYPE_PLATFORM) {
      } else if (a->type==SPRITE_TYPE_HERO) {
        bdx=-adx; adx=0;
        bdy=-ady; ady=0;
      } else if (b->type==SPRITE_TYPE_HERO) {
      } else if (b->dx&&!a->dx) {
        bdx=-adx; adx=0;
        bdy=-ady; ady=0;
        b->constraint|=acon;
      } else if (a->dx&&!b->dx) {
        a->constraint|=bcon;
      // Unclear which should move. We can't eg just go "always A" or "always B", so use the provided seed to decide.
      } else if (seed&1) {
        bdx=-adx; adx=0;
        bdy=-ady; ady=0;
      }
      
      // Apply correction and propagate constraints.
      a->x+=adx;
      a->y+=ady;
      b->x+=bdx;
      b->y+=bdy;
      a->constraint|=(b->constraint&bcon);
      b->constraint|=(a->constraint&acon);
      result++;
    }
  }
  return result;
}

/* Update sprites.
 */
 
void update_sprites(uint16_t input) {

  // First update each sprite and set some initial physics stuff.
  struct sprite *sprite=spritev;
  uint8_t i=spritec;
  for (;i-->0;sprite++) {
  
    if (sprite->dead) continue;
  
    sprite->pvx=sprite->x;
    sprite->pvy=sprite->y;
    
    if (sprite->autodx<0) {
      sprite->autodx++;
      sprite->x-=2;
    } else if (sprite->autodx>0) {
      sprite->autodx--;
      sprite->x+=2;
    }
    if (sprite->autody<0) {
      sprite->autody++;
      sprite->y-=2;
    } else if (sprite->autody>0) {
      sprite->autody--;
      sprite->y++;
    }
    
    switch (sprite->type) {
      case SPRITE_TYPE_HERO: update_hero(sprite,input); break;
      case SPRITE_TYPE_SUSIE: update_susie(sprite); break;
      case SPRITE_TYPE_FIRE: update_fire(sprite); break;
      case SPRITE_TYPE_CROCBOT: update_crocbot(sprite); break;
      case SPRITE_TYPE_PLATFORM: update_platform(sprite); break;
      case SPRITE_TYPE_SHREDDER: update_shredder(sprite); break;
      case SPRITE_TYPE_BALLOON: update_balloon(sprite); break;
    }
    
    if (sprite->mobile&&sprite->gravity) sprite->y++;
    
    sprite->dx=sprite->x-sprite->pvx;
    sprite->dy=sprite->y-sprite->pvy;
    sprite->constraint=0;
  }
  
  // Rectify collisions.
  // Requires multiple passes to settle, yeah yeah the algorithm is imperfect...
  uint8_t repc=16;
  while ((repc-->0)&&rectify_collisions(repc)) ;
  
  // Set the final delta.
  for (sprite=spritev,i=spritec;i-->0;sprite++) {
    sprite->dx=sprite->x-sprite->pvx;
    sprite->dy=sprite->y-sprite->pvy;
  }
}

/* Draw sprites.
 */
 
void draw_sprites(struct ma_framebuffer *dst) {
  struct sprite *sprite=spritev;
  uint8_t i=spritec;
  for (;i-->0;sprite++) {
    if (!sprite->visible) continue;
    // Sprite types with multiple tiles or anything else weird, must handle special here.
    // One tile, it's generic.
    switch (sprite->type) {
    
      case SPRITE_TYPE_HERO: {
          int8_t y=sprite->y;
          if (sprite->vs8[4]>=0) y+=8; // (y,h) cover the pumpkin if present
          switch (sprite->vs8[1]) { // torso
            case 0: ma_blit(dst,sprite->x,y+4,&tex_sprites_30,sprite->xform); break;
            case 1: ma_blit(dst,sprite->x,y+4,&tex_sprites_31,sprite->xform); break;
            case 2: ma_blit(dst,sprite->x,y+4,&tex_sprites_30,sprite->xform); break;
            case 3: ma_blit(dst,sprite->x,y+4,&tex_sprites_32,sprite->xform); break;
          }
          switch (sprite->vs8[5]) { // head
            case 0: ma_blit(dst,sprite->x,y,&tex_sprites_20,sprite->xform); break;
            case 1: ma_blit(dst,sprite->x,y,&tex_sprites_21,sprite->xform); break;
            case 2: ma_blit(dst,sprite->x,y,&tex_sprites_22,sprite->xform); break;
          }
          if ((sprite->vs8[4]>=0)&&(sprite->vs8[4]<spritec)) { // pumpkin and raised arm
            struct sprite *pumpkin=spritev+sprite->vs8[4];
            ma_blit(dst,sprite->x,y-pumpkin->h,pumpkin->texture,pumpkin->xform|MA_XFORM_YREV);
            ma_blit(dst,sprite->x,y,&tex_sprites_36,sprite->xform);
          } else switch (sprite->vs8[1]) { // idle or walking arms
            case 0: ma_blit(dst,sprite->x,y+4,&tex_sprites_33,sprite->xform); break;
            case 1: ma_blit(dst,sprite->x,y+4,&tex_sprites_34,sprite->xform); break;
            case 2: ma_blit(dst,sprite->x,y+4,&tex_sprites_33,sprite->xform); break;
            case 3: ma_blit(dst,sprite->x,y+4,&tex_sprites_35,sprite->xform); break;
          }
        } break;
        
      case SPRITE_TYPE_CROCBOT: {
          if (sprite->vs8[2]>8) { // rail
            ma_blit(dst,sprite->x+8,sprite->y-11+sprite->vs8[2],&tex_sprites_63,0);
          }
          if (sprite->vs8[2]>1) { // rail
            ma_blit(dst,sprite->x+8,sprite->y-3+sprite->vs8[2],&tex_sprites_63,0);
          }
          ma_blit(dst,sprite->x+8,sprite->y-3+sprite->vs8[2],&tex_sprites_62,0); // foot
          // little dot on the foot to suggestion rotation of a gear
          if (sprite->vs8[2]&&((sprite->vs8[1]==1)||(sprite->vs8[1]==3))) {
            uint8_t subx=12,suby=2;
            switch (sprite->vs8[2]&0x06) {
              case 2: subx++; break;
              case 4: subx++; suby++; break;
              case 6: suby++; break;
            }
            dst->v[(sprite->y+sprite->vs8[2]+suby)*96+sprite->x+subx]=0x2c;
          } else if (sprite->vs8[2]) {
            dst->v[(sprite->y+sprite->vs8[2]+2)*96+sprite->x+12]=0x2c;
          }
          ma_blit(dst,sprite->x,sprite->y-3,&tex_sprites_60,0); // jaw
          ma_blit(dst,sprite->x+8,sprite->y-3,&tex_sprites_61,0); // head
        } break;
        
      case SPRITE_TYPE_PLATFORM: {
          ma_blit(dst,sprite->x,sprite->y,&tex_sprites_70,0);
          ma_blit(dst,sprite->x+8,sprite->y,&tex_sprites_70,MA_XFORM_XREV);
          uint8_t radius=((sprite->vs8[3]>>2)&3)+1;
          ma_framebuffer_fill_rect(dst,sprite->x+8-radius,sprite->y+sprite->h+1,radius<<1,1,0x00);
        } break;
        
      case SPRITE_TYPE_SHREDDER: {
          int8_t y,h;
          uint8_t x,xform;
          // blades:
          if (sprite->vs8[0]==1) {
            x=sprite->x+2;
            xform=MA_XFORM_XREV;
          } else {
            x=sprite->x-2;
            xform=0;
          }
          for (y=sprite->y+(sprite->vs8[1]&15);;y+=12) {
            if (y+8>sprite->y+sprite->h) break;
            ma_blit(dst,x,y,&tex_sprites_66,xform);
          }
          for (y=sprite->y+sprite->h-8-(sprite->vs8[1]&15);;y-=12) {
            if (y<sprite->y) break;
            ma_blit(dst,x,y,&tex_sprites_66,xform|MA_XFORM_YREV);
          }
          // rail:
          ma_blit(dst,sprite->x,sprite->y,&tex_sprites_65,0);
          ma_blit(dst,sprite->x,sprite->y+sprite->h-8,&tex_sprites_65,0);
          for (y=sprite->y+8,h=sprite->h-16;h>0;y+=8,h-=8) {
            ma_blit(dst,sprite->x,y,&tex_sprites_65,0);
          }
          // feet:
          ma_blit(dst,sprite->x,sprite->y,&tex_sprites_64,MA_XFORM_YREV);
          ma_blit(dst,sprite->x,sprite->y+sprite->h-8,&tex_sprites_64,0);
        } break;
        
      default: if (sprite->texture) {
          ma_blit(dst,sprite->x,sprite->y,sprite->texture,sprite->xform);
        }
    }
  }
}

/* Test sprite on goal.
 */
 
uint8_t sprite_is_on_goal(const struct sprite *sprite) {

  if (
    ((sprite->type==SPRITE_TYPE_SOLID)||(sprite->type==SPRITE_TYPE_ONEWAY))&&
    (sprite->vs8[0]==1)
  ) return 1; // It *is* the goal, so let's call that Yes.
  
  // Feet must be stable.
  if (!(sprite->constraint&SPRITE_CONSTRAINT_S)) return 0;
  
  // Recur into any sprite whose head touches my feet.
  // Any positive result, we're done.
  int8_t y=sprite->y+sprite->h;
  const struct sprite *q=spritev;
  uint8_t i=spritec;
  for (;i-->0;q++) {
    if (!q->physics) continue;
    if (q->y!=y) continue;
    if (q->x>=sprite->x+sprite->w) continue;
    if (q->x+q->w<=sprite->x) continue;
    if (sprite_is_on_goal(q)) return 1;
  }
  
  return 0;
}
