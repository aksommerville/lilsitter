#include "map.h"
#include "multiarcade.h"

#define GORE_LIMIT 128

uint8_t map[MAP_SIZE_LIMIT]={0};

uint8_t map_songid=4;

// We need to know this for gore tracking.
static uint8_t bgcolor=0;

static struct gore {
  int16_t x,y; // s7.8 pixels
  int16_t dx,dy; // s7.8 pixels
  uint8_t pixel;
} gorev[GORE_LIMIT];
static uint8_t gorec=0;

/* Validate a freshly-loaded map.
 * Force valid if necessary.
 */
 
static void map_validate() {
  uint16_t p=0;
  while (1) {
  
    if (p>=MAP_SIZE_LIMIT) {
      ma_log("ERROR: Overran map end (limit %d), blanking the whole thing.\n",MAP_SIZE_LIMIT);
      map[0]=MAP_CMD_EOF;
      return;
    }
    
    switch (map[p++]) {
      #define REQUIRE(c) if (p>MAP_SIZE_LIMIT-c) { \
        ma_log("ERROR: Overran map around byte %d, expecting %d more.\n",p,c); \
        map[0]=MAP_CMD_EOF; \
        return; \
      } p+=c;
      case MAP_CMD_EOF: return; // <-- the only safe way out
      case MAP_CMD_NOOP1: break;
      case MAP_CMD_NOOPN: REQUIRE(1) p+=map[p++]; break;
      case MAP_CMD_BGCOLOR: REQUIRE(1) break;
      case MAP_CMD_SOLID: REQUIRE(4) break;
      case MAP_CMD_ONEWAY: REQUIRE(3) break;
      case MAP_CMD_GOAL: break;
      case MAP_CMD_HERO: REQUIRE(2) break;
      case MAP_CMD_DESMOND: REQUIRE(2) break;
      case MAP_CMD_SUSIE: REQUIRE(2) break;
      case MAP_CMD_FIRE: REQUIRE(2) break;
      case MAP_CMD_DUMMY: REQUIRE(3) break;
      case MAP_CMD_CROCBOT: REQUIRE(2) break;
      case MAP_CMD_PLATFORM: REQUIRE(3) break;
      case MAP_CMD_SHREDDER: REQUIRE(4) break;
      case MAP_CMD_BALLOON: REQUIRE(2) break;
      case MAP_CMD_SONG: REQUIRE(1) map_songid=map[p-1]; break;
      default: {
          p--;
          ma_log("ERROR: Unknown map command 0x%02x. Terminating map at byte %d.\n",map[p],p);
          map[p]=MAP_CMD_EOF;
          return;
        }
    }
  }
}

/* Load from disk.
 */
 
uint8_t map_load(uint16_t id) {

  gorec=0;

  char path[64];
  snprintf(path,sizeof(path),"/Sitter/map/%05d",id);
  
  int32_t err=ma_file_read(map,sizeof(map),path,0);
  if (err<0) {
    ma_log("Failed read file '%s' for map %d\n",path,id);
    map[0]=MAP_CMD_EOF;  
    return 0;
  }
  if (err<MAP_SIZE_LIMIT) map[err]=MAP_CMD_EOF;
  map_validate();
  return 1;
}

/* Draw primitives in framebuffer.
 */
 
void fill_rect(uint8_t *dst,int8_t x,int8_t y,int8_t w,int8_t h,uint8_t pixel) {
  if ((x>=96)||(y>=64)) return;
  if (x<0) { w+=x; x=0; }
  if (y<0) { h+=y; y=0; }
  if (x>96-w) w=96-x;
  if (y>64-h) h=64-y;
  if ((w<1)||(h<1)) return;
  dst+=y*96+x;
  for (;h-->0;dst+=96) memset(dst,pixel,w);
}

/* Draw goal.
 * It's always a lone round rect, unless it touches the screen edge.
 * Bottom edge may be outlined or stippled depending on (solid).
 */
 
static void draw_goal(uint8_t *dst,uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t solid) {
  // If extremely small, do a solid rectangle instead.
  if ((w<4)||(h<4)) {
    fill_rect(dst,x,y,w,h,solid?0xff:0xdb);
  } else {
    // Outline in black, skipping the corners, unless it touches an edge.
    uint8_t square_outline=0;
    if (!solid) {
      // If one-way, it always has 3 borders, even at screen edges.
      fill_rect(dst,x+1,y,w-2,1,0x00);
      fill_rect(dst,x,y+1,1,h-1,0x00);
      fill_rect(dst,x+w-1,y+1,1,h-1,0x00);
      x++; y++; w-=2; h-=1;
    } else if ((x<1)||(y<1)||(x+w>=96)||(y+h>=64)) {
      square_outline=1;
    } else {
      fill_rect(dst,x+1,y,w-2,1,0x00);
      fill_rect(dst,x+1,y+h-1,w-2,1,0x00);
      fill_rect(dst,x,y+1,1,h-2,0x00);
      fill_rect(dst,x+w-1,y+1,1,h-2,0x00);
      x++; y++; w-=2; h-=2;
    }
    // Dark gray and white checks on the remainder.
    uint8_t color0=0xff;
    uint8_t yi=y,hi=h;
    while (hi>0) {
      if (color0==0xff) color0=0x49; else color0=0xff;
      uint8_t rowh=2;
      if (rowh>hi) rowh=hi;
      uint8_t xi=w;
      uint8_t color=color0;
      while (xi>=2) {
        fill_rect(dst,x+w-xi,yi,2,rowh,color);
        if (color==0xff) color=0x49; else color=0xff;
        xi-=2;
      }
      if (xi) {
        fill_rect(dst,x+w-xi,yi,1,rowh,color);
      }
      yi+=rowh;
      hi-=rowh;
    }
    // If we skipped outlines due to screen edge, go back and fill in the non-edge ones.
    // We do sharp corners in this case.
    if (square_outline) {
      if (x>0) fill_rect(dst,x,y,1,h,0x00);
      if (y>0) fill_rect(dst,x,y,w,1,0x00);
      if (x+w<96) fill_rect(dst,x+w-1,y,1,h,0x00);
      if (y+h<64) fill_rect(dst,x,y+h-1,w,1,0x00);
    }
  }
}

/* Draw oneways and solids.
 * These are just markers that we will pick up on the next pass.
 */

static void draw_oneway(uint8_t *dst,uint8_t x,uint8_t y,uint8_t w) {
  fill_rect(dst,x,y,w,1,0x59);
}
 
static void draw_solid(uint8_t *dst,uint8_t x,uint8_t y,uint8_t w,uint8_t h) {
  fill_rect(dst,x,y,w,h,0x58);
}

/* Finish drawing.
 * (dst) contains some final things and some placeholders.
 * (scratch) is uninitialized.
 */
 
static void map_draw_finish(uint8_t *dst,uint8_t *scratch) {

  // Read only from (dst) and write only to (scratch).
  memcpy(scratch,dst,96*64);
  
  uint8_t *dstp=dst,*scratchp=scratch;
  uint8_t y=0;
  for (;y<64;y++) {
    uint8_t x=0;
    for (;x<96;x++,dstp++,scratchp++) {
    
      // Solid.
      if (*dstp==0x58) {
        // If it has one (bgcolor) neighbor on each axis, it's a corner, so nix it.
        if (
          ((x&&(dstp[-1]==bgcolor))||((x<95)&&(dstp[1]==bgcolor)))&&
          ((y&&(dstp[-96]==bgcolor))||((y<63)&&(dstp[96]==bgcolor)))
        ) {
          *scratchp=bgcolor;
        // Otherwise, if it has at least one cardinal (bgcolor) neighbor, it's an edge, so go dark.
        } else if (
          (x&&(dstp[-1]==bgcolor))||
          ((x<95)&&(dstp[1]==bgcolor))||
          (y&&(dstp[-96]==bgcolor))||
          ((y<63)&&(dstp[96]==bgcolor))
        ) {
          *scratchp=0x08;
        // None of the above? Go light. (NB 0x58 is not the actual color, it's just a placeholder)
        } else {
          *scratchp=0x0c;
        }
        
      // Oneway. Horz bar and 1/2 stipple for one row below.
      } else if (*dstp==0x59) {
        *scratchp=0x0a;
        if ((y<64)&&(dstp[96]==bgcolor)) scratchp[96]=(x&1)?0x0a:0x0b;
      }
    
    }
  }
  
  memcpy(dst,scratch,96*64);
}

/* Draw.
 */
 
void map_draw(uint8_t *dst,uint8_t *scratch) {
  uint8_t goal=0;
  const uint8_t *src=map;
  while (1) {
    switch (*src++) {
      case MAP_CMD_EOF: goto _done_;
      case MAP_CMD_NOOP1: break;
      case MAP_CMD_NOOPN: src+=*src++; break;
    
      case MAP_CMD_BGCOLOR: {
          bgcolor=*src;
          memset(dst,*src++,96*64);
        } break;
    
      case MAP_CMD_SOLID: {
          uint8_t x=*src++;
          uint8_t y=*src++;
          uint8_t w=*src++;
          uint8_t h=*src++;
          if (goal) draw_goal(dst,x,y,w,h,1);
          else draw_solid(dst,x,y,w,h);
        } break;
        
      case MAP_CMD_ONEWAY: {
          uint8_t x=*src++;
          uint8_t y=*src++;
          uint8_t w=*src++;
          uint8_t h=4;
          if (goal) draw_goal(dst,x,y,w,6,0);
          else draw_oneway(dst,x,y,w);
        } break;

      case MAP_CMD_GOAL: {
          goal=1;
        } continue;

      case MAP_CMD_HERO: src+=2; break;
      case MAP_CMD_DESMOND: src+=2; break;
      case MAP_CMD_SUSIE: src+=2; break;
      case MAP_CMD_FIRE: src+=2; break;
      case MAP_CMD_DUMMY: src+=3; break;
      case MAP_CMD_CROCBOT: src+=2; break;
      case MAP_CMD_PLATFORM: src+=3; break;
      case MAP_CMD_SHREDDER: src+=4; break;
      case MAP_CMD_BALLOON: src+=2; break;
      case MAP_CMD_SONG: src+=1; break;
      default: goto _done_;
    }
    goal=0;
  }
 _done_:;
  map_draw_finish(dst,scratch);
}

/* Add gore.
 */
 
static const uint8_t gore_colorv[]={
  0x01,0x02,0x03,0x05,0x0b,0x21,0x22,0x23,
};
 
void map_add_gore(int8_t x,int8_t y,int8_t w,int8_t h) {
  if ((w<1)||(h<1)) return;
  const uint8_t dropc=40;
  int16_t X=x<<8,Y=y<<8,W=w<<8,H=h<<8;
  uint8_t i=dropc;
  while ((i-->0)&&(gorec<GORE_LIMIT)) {
    struct gore *gore=gorev+gorec++;
    gore->x=X+rand()%W;
    gore->y=Y+rand()%H;
    gore->dx=(rand()&0xff)-0x80;
    gore->dy=-(rand()%0xff);
    gore->pixel=gore_colorv[rand()%sizeof(gore_colorv)];
  }
}

/* Update gore.
 */
 
void map_update_gore(uint8_t *bgbits,uint8_t *fb) {
  const int16_t gravity=0x08;
  const int16_t terminal_velocity=0x200;
  int i=gorec;
  struct gore *gore=gorev+i-1;
  for (;i-->0;gore--) {
    gore->dy+=gravity;
    if (gore->dy>terminal_velocity) gore->dy=terminal_velocity;
    gore->x+=gore->dx;
    gore->y+=gore->dy;
    
    int8_t x=gore->x>>8,y=gore->y>>8;
    
    // If it goes offscreen, drop it.
    if ((x<0)||(y<0)||(x>=96)||(y>=64)) {
      gorec--;
      memmove(gore,gore+1,sizeof(struct gore)*(gorec-i));
      continue;
    }
    
    uint16_t p=y*96+x;
    
    // If it struck a non-background pixel, stick there and drop.
    if (bgbits[p]!=bgcolor) {
      bgbits[p]=gore->pixel;
      gorec--;
      memmove(gore,gore+1,sizeof(struct gore)*(gorec-i));
      continue;
    }
    
    // Draw to framebuffer.
    fb[p]=gore->pixel;
  }
}
