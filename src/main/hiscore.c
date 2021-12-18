#include "hiscore.h"
#include "multiarcade.h"

/* Get high score.
 */
 
uint32_t get_high_score(int32_t mapid) {
  int32_t p;
  if (mapid<-1) return HIGH_SCORE_UNSET;
  else if (mapid==-1) p=0;
  else if (mapid<0x10000) p=4*(mapid+1);
  else return HIGH_SCORE_UNSET;
  uint32_t score=HIGH_SCORE_UNSET;
  ma_file_read(&score,4,"/Sitter/hiscore",p);
  if (!score) return HIGH_SCORE_UNSET; // Zero is not valid, but does happen due to seeking
  return score;
}

/* Set high score.
 */
 
void save_high_score(int32_t mapid,uint32_t score) {
  int32_t p;
  if (mapid<-1) return;
  else if (mapid==-1) p=0;
  else if (mapid<0x10000) p=4*(mapid+1);
  else return;
  ma_file_write("/Sitter/hiscore",&score,4,p);
}
