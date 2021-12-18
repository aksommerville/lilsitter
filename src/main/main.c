#include "multiarcade.h"
#include "bba.h"
#include "map.h"
#include "sprite.h"
#include "hiscore.h"
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

static uint16_t mapid=0;
static int8_t completion_status=0; // counts away from zero when the game is finished
static uint8_t splash=0; // if positive, counts up to SPLASH_MINIMUM_TIME
static uint8_t splash_input_zero=0; // nonzero if all inputs have dropped during the splash -- requirement for dismissal
static uint8_t splash_input_one=0; // splash ends when inputs released

// All times are in frames and don't count splash time.
// Convert to minute:second.milli for display only.
static uint32_t level_time=0; // For this map run only.
static uint32_t valid_time=0; // Total of wins only.
static uint32_t total_time=0; // All in-game time since the intro splash.
static uint16_t failc=0; // Count of failures since the intro splash.

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

/* Format time for display.
 */
 
static int format_time(char *dst,int dsta,const char *label,uint32_t framec) {
  uint32_t sec=framec/60;
  uint32_t min=sec/60;
  uint32_t hour=min/60;
  framec%=60;
  sec%=60;
  min%=60;
  uint32_t milli=(framec*1000)/60;
  return snprintf(dst,dsta,"%s: %d:%02d:%02d.%03d",label,hour,min,sec,milli);
}

/* Load splash.
 */
 
static void begin_splash(const char *path,int sethi) {
  splash=1;
  splash_input_zero=0;
  splash_input_one=0;
  
  if (ma_file_read(bgbits.v,96*64,path,0)!=96*64) {
    memset(bgbits.v,0x00,96*64);
  }
  
  uint32_t record_time;
  uint8_t highlight_best=0;
  if (mapid) {
    record_time=get_high_score(mapid-1);
    if (sethi&&(level_time<record_time)) {
      save_high_score(mapid-1,level_time);
      highlight_best=1;
      record_time=level_time;
    }
  } else {
    record_time=get_high_score(-1);
    if (sethi&&(total_time<record_time)) {
      save_high_score(-1,total_time);
      highlight_best=1;
      record_time=total_time;
    }
  }
  
  char buf[64];
  int bufc=format_time(buf,sizeof(buf),"Level",level_time);
  ma_font_render(&bgbits,14,19,&font_basic,buf,bufc,0x7b);
  bufc=format_time(buf,sizeof(buf),"Total",total_time);
  ma_font_render(&bgbits,14,27,&font_basic,buf,bufc,0x7b);
  bufc=format_time(buf,sizeof(buf)," Best",record_time);
  ma_font_render(&bgbits,14,35,&font_basic,buf,bufc,highlight_best?0x1f:0x7b);
}

static void end_splash() {
  splash=0;
  level_time=0;
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
  failc++;
  total_time+=level_time;
  begin_splash("/Sitter/splash/lose.tsv",0);
  completion_status=0;
  map_load(mapid);
  spawn_sprites();
}

static void win_level() {
  valid_time+=level_time;
  total_time+=level_time;
  completion_status=0;
  
  if (map_load(++mapid)) {
    begin_splash("/Sitter/splash/win.tsv",1);
  } else {
    map_load(mapid=0);
    begin_splash("/Sitter/splash/allwin.tsv",-1);
    valid_time=0;
    total_time=0;
    failc=0;
  }
  spawn_sprites();
}

/* Update.
 */
 
void loop() {
  uint16_t input=ma_update();
  
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
    level_time++;
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

  //TODO intro splash
  mapid=0;
  map_load(mapid);
  map_draw(bgbits.v,fb.v);
  spawn_sprites();
}
