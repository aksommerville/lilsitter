/* hiscore.h
 * By "hi" of course we mean "lo"; these are times.
 */
 
#ifndef HISCORE_H
#define HISCORE_H

#include <stdint.h>

#define HIGH_SCORE_UNSET 2159999 /* one frame short of ten hours */

/* mapid -1 is the total.
 */
uint32_t get_high_score(int32_t mapid);
void save_high_score(int32_t mapid,uint32_t score);

#endif
