#include "multiarcade.h"
#include "bba.h"
#include "map.h"
#include "sprite.h"
#include "hiscore.h"
#include "sound.h"
#include <stdio.h>
#include <string.h>

extern struct ma_font font_basic;

/* Globals.
 */
 
#define GAME_END_TIMEOUT 60 /* Terminal condition must remain stable for so long before game over. */
#define SPLASH_MINIMUM_TIME 30

#define SPLASH_INTRO     1
#define SPLASH_WIN       2
#define SPLASH_LOSE      3
#define SPLASH_ALLWIN    4

// Try to keep (AUTO_FAIL_TIME-AUTO_FAIL_WARN_TIME) one less than a multiple of 16.
#define AUTO_FAIL_TIME (16*60)
#define AUTO_FAIL_WARN_TIME (10*60+7)

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
static uint8_t splash_selection=0; // SPLASH_* or zero if playing
static uint8_t music_enable=1;
static uint8_t songid=0;
static uint16_t pvinput=0;
static uint32_t idle_framec=0; // For timed auto-fail

static uint8_t song[4096];
static uint16_t songc=0;

// All times are in frames and don't count splash time.
// Convert to minute:second.milli for display only.
static uint32_t level_time=0; // For this map run only.
static uint32_t valid_time=0; // Total of wins only.
static uint32_t total_time=0; // All in-game time since the intro splash.
static uint16_t failc=0; // Count of failures since the intro splash.

/* Audio.
 */

struct bba_synth synth={0};
 
int16_t audio_next() {
  return bba_synth_update(&synth);
}

/* Begin song.
 */
 
static void restart_music() {
  bba_synth_play_song(&synth,0,0);
  bba_synth_quiet(&synth);
  if (!music_enable) return; // Don't bother loading it; toggling also calls this and reloads
  char path[64];
  snprintf(path,sizeof(path),"/Sitter/song/%03d.bba",songid);
  int32_t err=ma_file_read(song,sizeof(song),path,0);
  if (err<=0) {
    songc=0;
    return;
  }
  songc=err;
  bba_synth_play_song(&synth,song,songc);
}

static void set_song(uint8_t id) {
  if (songid==id) return;
  songid=id;
  restart_music();
}

/* Turn music on or off.
 */
 
static void toggle_music() {
  // Careful with the order here: Changing song will drop sound effects too.
  if (music_enable) {
    music_enable=0;
    bba_synth_play_song(&synth,0,0);
    bba_synth_quiet(&synth);
  } else {
    music_enable=1;
    restart_music();
  }
  sound_ui_toggle();
  // Redraw the "OFF", "ON" bit in bgbits. Cheat...
  if (splash&&(splash_selection==SPLASH_INTRO)) {
    ma_framebuffer_fill_rect(&bgbits,42,36,20,7,0xa4);
    ma_font_render(&bgbits,42,36,&font_basic,music_enable?"ON":"OFF",-1,0x1f);
  }
}

/* Render scene.
 */
 
static void render_scene(ma_pixel_t *v) {
  memcpy(fb.v,bgbits.v,fb.stride*fb.h*sizeof(ma_pixel_t));
  if (!splash) {
    map_update_gore(bgbits.v,fb.v);
    draw_sprites(&fb);
    
    if (idle_framec>=AUTO_FAIL_WARN_TIME) {
      char msg[32];
      uint8_t msgc=snprintf(msg,sizeof(msg),"PRESS A KEY! %d",(AUTO_FAIL_TIME-idle_framec+30)/60);
      ma_font_render(&fb,16,28,&font_basic,msg,msgc,(idle_framec&16)?0x00:0xff);
    }
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
 
static void begin_splash(uint8_t which) {
  splash=1;
  splash_input_zero=0;
  splash_input_one=0;
  splash_selection=which;
  
  const char *path="";
  switch (which) {
    case SPLASH_INTRO: {
        path="/Sitter/splash/intro.tsv";
        set_song(1);
      } break;
    case SPLASH_WIN: {
        path="/Sitter/splash/win.tsv";
        set_song(2);
      } break;
    case SPLASH_LOSE: {
        path="/Sitter/splash/lose.tsv";
        set_song(3);
      } break;
    case SPLASH_ALLWIN: {
        path="/Sitter/splash/allwin.tsv";
        set_song(5);
      } break;
  }
  
  if (ma_file_read(bgbits.v,96*64,path,0)!=96*64) {
    memset(bgbits.v,0x00,96*64);
  }
  
  /* Update high scores if we do that.
   */
  uint32_t record_time=HIGH_SCORE_UNSET;
  uint8_t highlight_best=0;
  switch (which) {
    case SPLASH_WIN: {
        record_time=get_high_score(mapid-1);
        if (level_time<record_time) {
          save_high_score(mapid-1,level_time);
          record_time=level_time;
          highlight_best=1;
        }
      } break;
    case SPLASH_LOSE: {
        record_time=get_high_score(mapid-1);
      } break;
    case SPLASH_ALLWIN: {
        record_time=get_high_score(-1);
        if (total_time<record_time) {
          save_high_score(-1,total_time);
          record_time=total_time;
          highlight_best=1;
        }
      } break;
    case SPLASH_INTRO: {
        record_time=get_high_score(-1);
      } break;
  }
  
  /* Draw the various decorations.
   */
  char buf[64];
  uint8_t bufc;
  switch (which) {
    case SPLASH_WIN:
    case SPLASH_LOSE:
    case SPLASH_ALLWIN: {
        bufc=format_time(buf,sizeof(buf),"Level",level_time);
        ma_font_render(&bgbits,14,19,&font_basic,buf,bufc,0x7b);
        bufc=format_time(buf,sizeof(buf),"Total",total_time);
        ma_font_render(&bgbits,14,27,&font_basic,buf,bufc,0x7b);
        bufc=format_time(buf,sizeof(buf)," Best",record_time);
        ma_font_render(&bgbits,14,35,&font_basic,buf,bufc,highlight_best?0x1f:0x7b);
      } break;
    case SPLASH_INTRO: {
        bufc=snprintf(buf,sizeof(buf),"Music: %s",music_enable?"ON":"OFF");
        ma_font_render(&bgbits,14,36,&font_basic,buf,bufc,0x1f);
        if (record_time<HIGH_SCORE_UNSET) {
          bufc=format_time(buf,sizeof(buf),"Best",record_time);
          ma_font_render(&bgbits,14,46,&font_basic,buf,bufc,0xfc);
        }
      } break;
  }
}

static void end_splash() {
  if (splash_selection==SPLASH_ALLWIN) {
    begin_splash(SPLASH_INTRO);
  } else {
    set_song(map_songid);
    splash=0;
    level_time=0;
    map_draw(bgbits.v,fb.v);
    //ma_log("Begin map %05d\n",mapid);
  }
}

/* Check if the level is won or lost.
 * That means either the hero is dead, or every human is on the goal.
 * Once completion is achieved, it has to be sustained for a brief time before taking effect.
 * If complete and any human is dead, it's a loss.
 */
 
static void check_completion() {

  // One new way to complete: Timed auto-fail.
  if (idle_framec>=AUTO_FAIL_TIME) {
    completion_status=-GAME_END_TIMEOUT;
    return;
  }

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
  begin_splash(SPLASH_LOSE);
  completion_status=0;
  map_load(mapid);
  spawn_sprites();
}

static void win_level() {
  valid_time+=level_time;
  total_time+=level_time;
  completion_status=0;
  
  if (map_load(++mapid)) {
    begin_splash(SPLASH_WIN);
  } else {
    map_load(mapid=0);
    begin_splash(SPLASH_ALLWIN);
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
  
  if (input) idle_framec=0;
  else idle_framec++;
  sound_frame();
  
  if (input!=pvinput) {
    if (splash&&(splash_selection==SPLASH_INTRO)) {
      if ((input&MA_BUTTON_UP)&&!(pvinput&MA_BUTTON_UP)) toggle_music();
      if ((input&MA_BUTTON_DOWN)&&!(pvinput&MA_BUTTON_DOWN)) toggle_music();
    }
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
 
#if MA_NON_TINY
  #define AUDIO_RATE 44100
#else
  #define AUDIO_RATE 22050
#endif

void setup() {

  bba_synth_init(&synth,AUDIO_RATE);

  struct ma_init_params params={
    .videow=fb.w,
    .videoh=fb.h,
    .rate=60,
    .audio_rate=AUDIO_RATE,
  };
  if (!ma_init(&params)) return;
  if ((params.videow!=fb.w)||(params.videoh!=fb.h)) return;

  srand(millis());

  begin_splash(SPLASH_INTRO);
  mapid=0;
  map_load(mapid);
  spawn_sprites();
}
