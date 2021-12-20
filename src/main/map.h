/* map.h
 * One encoded level.
 * Includes terrain geometry, level rules, maybe graphics?
 * I'm going to work directly from the encoded format in memory, see if that's workable.
 */
 
#ifndef MAP_H
#define MAP_H

#include <stdint.h>

/* Encoded map is a stream of commands with a 1-byte introducer.
 * Must be terminated somewhere by an EOF command -- once live, you do not need range checks.
 * Unknown commands are an error.
 */
#define MAP_SIZE_LIMIT 512
extern uint8_t map[MAP_SIZE_LIMIT];

#define MAP_CMD_EOF            0x00 /* () End of file, don't read beyond this point. */
#define MAP_CMD_NOOP1          0x01 /* () Ignore one byte. */
#define MAP_CMD_NOOPN          0x02 /* (u8 len) Ignore content. */
#define MAP_CMD_BGCOLOR        0x03 /* (u8 color) */
#define MAP_CMD_SOLID          0x04 /* (u8 x,u8 y,u8 w,u8 h) Solid block, coords in pixels. */
#define MAP_CMD_ONEWAY         0x05 /* (u8 x,u8 y,u8 w) One-way platform. */
#define MAP_CMD_GOAL           0x06 /* () Next SOLID or ONEWAY is the goal. */
#define MAP_CMD_HERO           0x07 /* (u8 x,u8 y) Hero's starting position (mid-bottom). */
#define MAP_CMD_DESMOND        0x08 /* (u8 x,u8 y) */
#define MAP_CMD_SUSIE          0x09 /* (u8 x,u8 y) */
#define MAP_CMD_FIRE           0x0a /* (u8 x,u8 y) */
#define MAP_CMD_DUMMY          0x0b /* (u8 x,u8 y,u8 tileid) */
#define MAP_CMD_CROCBOT        0x0c /* (u8 x,u8 y) */
#define MAP_CMD_PLATFORM       0x0d /* (u8 x,u8 y,u8 mode: (0,1,2)=(still,horz,vert)) */
#define MAP_CMD_SHREDDER       0x0e /* (u8 x,u8 y,u8 orient: (0,1)=(left,right),u8 h) */
#define MAP_CMD_BALLOON        0x0f /* (u8 x,u8 y) */
#define MAP_CMD_SONG           0x10 /* (u8 songid) */

/* Initially 4, and updates when we load.
 * If the map didn't specify one, the previous value sticks.
 */
extern uint8_t map_songid;

/* Map ids are sequential from zero, and the file name is derived from it.
 * Returns >0 if loaded, 0 if not found or malformed.
 * In any case, we leave (map) well formed.
 */
uint8_t map_load(uint16_t id);

/* Draw the static terrain to a 96x64x8 framebuffer.
 * You must provide two buffers because we know you have them, and we need some scratch buffer too.
 */
void map_draw(uint8_t *dst,uint8_t *scratch);

void fill_rect(uint8_t *dst,int8_t x,int8_t y,int8_t w,int8_t h,uint8_t pixel);

void map_add_gore(int8_t x,int8_t y,int8_t w,int8_t h);

/* Gore is like a sprite initially, it draws into the framebuffer.
 * Once stabilized, the splatterlets get written onto (bgbits) for safekeeping.
 */
void map_update_gore(uint8_t *bgbits,uint8_t *fb);

#endif
