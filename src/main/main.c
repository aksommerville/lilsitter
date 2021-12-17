#include "multiarcade.h"
#include "bba.h"
#include "map.h"
#include "sprite.h"
#include <stdio.h>
#include <string.h>

extern struct ma_font font_basic;

/* Globals.
 */
 
#define GAME_END_TIMEOUT 60 /* Terminal condition must remain stable for so long before game over. */
#define SPLASH_MINIMUM_TIME 30

static ma_pixel_t fb_storage[96*64];
static struct ma_framebuffer fb={
  .w=96,
  .h=64,
  .stride=96,
  .v=fb_storage,
};

static ma_pixel_t bgbits_storage[96*64];
static struct ma_framebuffer bgbits={
  .w=96,
  .h=64,
  .stride=96,
  .v=bgbits_storage,
};

static uint16_t pvinput=0;
static uint16_t mapid=0;
static int8_t completion_status=0; // counts away from zero when the game is finished
static uint8_t splash=0; // if positive, counts up to SPLASH_MINIMUM_TIME
static uint8_t splash_input_zero=0; // nonzero if all inputs have dropped during the splash -- requirement for dismissal
static uint8_t splash_input_one=0; // splash ends when inputs released

/* Audio.
 */

static struct bba_synth synth={0};
 
int16_t audio_next() {
  return bba_synth_update(&synth);
  return 0;
}

/* Render scene.
 */
 
static void render_scene(ma_pixel_t *v) {
  memcpy(fb.v,bgbits.v,fb.stride*fb.h*sizeof(ma_pixel_t));
  if (!splash) {
    map_update_gore(bgbits.v,fb.v);
    draw_sprites(&fb);
  }
}

/* Load splash.
 */
 
static void begin_splash(const char *path) {
  splash=1;
  splash_input_zero=0;
  splash_input_one=0;
  ma_file_read(bgbits.v,96*64,path,0);
}

static void end_splash() {
  splash=0;
  map_draw(bgbits.v,fb.v);
}

/* Check if the level is won or lost.
 * That means either the hero is dead, or every human is on the goal.
 * Once completion is achieved, it has to be sustained for a brief time before taking effect.
 * If complete and any human is dead, it's a loss.
 */
 
static void check_completion() {

  uint8_t deadc=0,goalc=0,targetc=0,liveheroc=0;
  const struct sprite *sprite=spritev;
  uint8_t i=spritec;
  for (;i-->0;sprite++) {
  
    if (!sprite->target) continue;
    targetc++;
    
    if (sprite->dead) {
      deadc++;
      continue;
    }
    
    if (!sprite->visible) { // being carried or something, don't count it
      targetc--;
      continue;
    }
    
    if (sprite->type==SPRITE_TYPE_HERO) liveheroc++;
    
    // Feet must be stable.
    if (!(sprite->constraint&SPRITE_CONSTRAINT_S)) continue;
    
    // The expensive test: Is it standing on the goal?
    if (sprite_is_on_goal(sprite)) goalc++;
  }
  //ma_log("deadc=%d goalc=%d targetc=%d liveheroc=%d spritec=%d\n",deadc,goalc,targetc,liveheroc,spritec);
  
  // If the hero is dead, you lose.
  if (!liveheroc) {
    completion_status--;
    return;
  }
  
  // At least one target sprite is neither dead nor goalled. This is normal, carry on.
  if (goalc+deadc<targetc) {
    completion_status=0;
    return;
  }
  
  // If a child is dead, you lose. (different from heroes, because you can still try to corral the rest of them).
  if (deadc) {
    completion_status--;
    return;
  }
  
  // You win!
  completion_status++;
}

/* End of level.
 */
 
static void lose_level() {
  begin_splash("/Sitter/splash/lose.tsv");
  completion_status=0;
  map_load(mapid);
  spawn_sprites();
}

static void win_level() {
  begin_splash("/Sitter/splash/win.tsv");
  completion_status=0;
  if (!map_load(++mapid)) {
    // TODO Win game
    map_load(mapid=0);
  }
  spawn_sprites();
}

/* Update.
 */
 
static int framec=0;
 
void loop() {
  uint16_t input=ma_update();
  
  //XXX for troubleshooting, stop at a given frame or slow down...
  //if (framec>0) return;
  framec++;
  uint8_t suspend=input&MA_BUTTON_A;//XXX
  
  if (input!=pvinput) {
    #define KEYDOWN(tag) ((input&MA_BUTTON_##tag)&&!(pvinput&MA_BUTTON_##tag))
    //TODO impulse events
    #undef KEYDOWN
    pvinput=input;
  }
  
  if (splash) {
    if (!input) splash_input_zero=1;
    if (splash<SPLASH_MINIMUM_TIME) {
      splash++;
    } else if (!input&&splash_input_one) {
      end_splash();
    } else if ((input&(MA_BUTTON_A|MA_BUTTON_B))&&splash_input_zero) {
      splash_input_one=1;
    }
    
  } else {
    update_sprites(input);
    check_completion();
    if (completion_status<=-GAME_END_TIMEOUT) lose_level();
    else if (completion_status>=GAME_END_TIMEOUT) win_level();
  }
  
  render_scene(fb.v);
  ma_send_framebuffer(fb.v);
}

/* Setup.
 */

void setup() {

  bba_synth_init(&synth,22050);

  struct ma_init_params params={
    .videow=fb.w,
    .videoh=fb.h,
    .rate=60,
    .audio_rate=22050,
  };
  if (!ma_init(&params)) return;
  if ((params.videow!=fb.w)||(params.videoh!=fb.h)) return;

  srand(millis());

  //TODO intro splash?
  map_load(mapid);
  map_draw(bgbits.v,fb.v);
  spawn_sprites();
}
