#include <stdint.h>

#define _(r,g,b) 0x##r,0x##g,0x##b,
#define X _ /* for marking ones i changed */
const uint8_t ma_ctab8[768]={
 _(00,00,00) _(6d,00,00) X(c0,00,00) _(ff,00,00) // 00
 X(00,22,00) _(6d,24,00) X(98,24,00) _(ff,24,00)
 X(00,45,00) _(6d,49,00) _(92,49,00) _(ff,49,00)
 X(00,68,00) X(4d,4d,00) X(92,4d,00) _(ff,6d,00)
 X(00,98,00) _(6d,92,00) X(a2,82,00) _(ff,92,00) // 10
 X(00,bc,00) X(4d,a6,00) X(92,a6,00) _(ff,b6,00)
 X(00,e0,00) _(6d,db,00) X(9c,d8,00) X(ff,cb,00)
 _(00,ff,00) _(6d,ff,00) X(b8,ff,00) X(ff,ef,00)
 X(00,12,24) _(6d,00,24) _(92,00,24) _(ff,00,24) // 20
 _(00,24,24) X(5d,20,24) X(9c,30,38) _(ff,24,24)
 _(00,49,24) X(5d,42,24) X(a2,44,24) _(ff,49,24)
 X(00,62,22) X(62,62,22) X(a2,68,24) _(ff,6d,24)
 _(00,92,24) _(6d,92,24) X(9c,82,24) _(ff,92,24) // 30
 _(00,b6,24) _(6d,b6,24) X(9e,a8,24) _(ff,b6,24)
 _(00,db,24) _(6d,db,24) X(9c,c4,24) _(ff,db,24)
 _(00,ff,24) _(6d,ff,24) X(e8,e0,14) _(ff,ff,24)
 X(00,24,49) X(55,20,49) X(a2,00,49) _(ff,00,49) // 40
 _(00,24,49) _(6d,24,49) _(92,24,49) _(ff,24,49)
 X(00,40,40) X(40,40,40) _(92,49,49) X(ff,38,38)
 _(00,6d,49) _(6d,6d,49) X(a0,6e,4c) _(ff,6d,49)
 _(00,92,49) _(6d,92,49) X(9c,8c,4c) _(ff,92,49) // 50
 _(00,b6,49) _(6d,b6,49) X(9d,ac,4d) _(ff,b6,49)
 _(00,db,49) X(3d,db,49) X(a2,d0,50) _(ff,db,49)
 _(00,ff,49) _(6d,ff,49) X(a8,ff,54) _(ff,ff,49)
 X(00,36,6d) X(48,18,8d) _(92,00,6d) _(ff,00,6d) // 60
 _(00,24,6d) _(6d,24,6d) _(92,24,6d) _(ff,24,6d)
 _(00,49,6d) _(6d,49,6d) _(92,49,6d) _(ff,49,6d)
 _(00,60,60) X(48,65,78) X(9c,6a,6d) _(ff,60,60)
 _(00,92,6d) _(6d,92,6d) _(92,92,6d) _(ff,92,6d) // 70
 _(00,b6,6d) _(6d,b6,6d) X(98,ae,6d) _(ff,b6,6d)
 _(00,db,6d) _(6d,db,6d) X(9c,d4,6d) _(ff,db,6d)
 _(00,ff,6d) _(6d,ff,6d) X(c2,ff,6d) _(ff,ff,6d)
 X(00,4a,92) X(20,10,98) X(b2,40,b2) X(ff,18,92) // 80
 _(00,24,92) X(30,24,9a) _(92,24,92) X(ff,34,92)
 _(00,49,92) _(6d,49,92) _(92,49,92) X(ff,59,92)
 _(00,6d,92) _(6d,6d,92) _(92,6d,92) _(ff,6d,92)
 _(00,92,92) _(6d,92,92) X(a0,9c,9c) X(ff,a0,a0) // 90
 _(00,b6,92) X(54,a0,92) _(92,b6,92) _(ff,b6,92)
 _(00,db,92) X(63,d8,90) _(92,db,92) _(ff,db,92)
 _(00,ff,92) X(6d,ff,9c) _(92,ff,92) _(ff,ff,92)
 X(00,5e,b6) X(18,08,b6) X(92,40,b6) _(ff,00,b6) // a0
 _(00,24,b6) X(20,28,b6) _(92,24,b6) _(ff,24,b6)
 _(00,49,b6) X(3d,4c,b6) _(92,49,b6) _(ff,49,b6)
 _(00,6d,b6) X(4d,6d,bc) _(92,6d,b6) _(ff,6d,b6)
 _(00,92,b6) X(60,98,b6) X(92,9a,b6) _(ff,92,b6) // b0
 _(00,b6,b6) X(6a,b2,b3) _(92,b6,b6) X(ff,bc,bc)
 _(00,db,b6) X(3d,db,b6) X(9c,c6,ba) _(ff,db,b6)
 _(00,ff,b6) _(6d,ff,b6) _(92,ff,b6) _(ff,ff,b6)
 X(00,6e,db) X(20,10,db) _(92,00,db) X(d0,44,c8) // c0
 _(00,24,db) X(2d,24,db) _(92,24,db) _(ff,24,db)
 _(00,49,db) X(2d,49,db) _(92,49,db) _(ff,49,db)
 _(00,6d,db) X(2d,6d,db) _(92,6d,db) _(ff,6d,db)
 _(00,92,db) X(2d,92,db) _(92,92,db) _(ff,b2,db) // d0
 _(00,b6,db) X(30,b6,db) _(92,b6,db) _(ff,b6,db)
 _(00,db,db) X(34,db,db) _(92,db,db) X(f8,de,de)
 _(00,ff,db) _(6d,ff,db) _(92,ff,db) _(ff,ff,db)
 X(00,80,ff) X(2c,00,ff) _(92,00,ff) X(e4,50,ff) // e0
 X(00,94,ff) X(28,24,ff) _(92,24,ff) X(e8,5c,ff)
 _(00,49,ff) X(4c,9c,ff) _(92,49,ff) X(ec,70,ff)
 _(00,6d,ff) X(62,ac,ff) _(92,6d,ff) X(f0,7c,ff)
 _(00,92,ff) X(48,92,ff) X(92,d0,ff) X(ff,a8,ff) // f0
 _(00,b6,ff) X(4d,b6,ff) X(92,e4,ff) X(ff,c4,ff)
 _(00,db,ff) _(6d,db,ff) X(b2,db,ff) X(ff,e4,ff)
 _(00,ff,ff) _(4d,ff,ff) _(92,ff,ff) _(ff,ff,ff)
};
#undef _

/* Initial (linear) guess.
 _(00,00,00) _(6d,00,00) _(92,00,00) _(ff,00,00)
 _(00,24,00) _(6d,24,00) _(92,24,00) _(ff,24,00)
 _(00,49,00) _(6d,49,00) _(92,49,00) _(ff,49,00)
 _(00,6d,00) _(6d,6d,00) _(92,6d,00) _(ff,6d,00)
 _(00,92,00) _(6d,92,00) _(92,92,00) _(ff,92,00)
 _(00,b6,00) _(6d,b6,00) _(92,b6,00) _(ff,b6,00)
 _(00,db,00) _(6d,db,00) _(92,db,00) _(ff,db,00)
 _(00,ff,00) _(6d,ff,00) _(92,ff,00) _(ff,ff,00)
 _(00,00,24) _(6d,00,24) _(92,00,24) _(ff,00,24)
 _(00,24,24) _(6d,24,24) _(92,24,24) _(ff,24,24)
 _(00,49,24) _(6d,49,24) _(92,49,24) _(ff,49,24)
 _(00,6d,24) _(6d,6d,24) _(92,6d,24) _(ff,6d,24)
 _(00,92,24) _(6d,92,24) _(92,92,24) _(ff,92,24)
 _(00,b6,24) _(6d,b6,24) _(92,b6,24) _(ff,b6,24)
 _(00,db,24) _(6d,db,24) _(92,db,24) _(ff,db,24)
 _(00,ff,24) _(6d,ff,24) _(92,ff,24) _(ff,ff,24)
 _(00,00,49) _(6d,00,49) _(92,00,49) _(ff,00,49)
 _(00,24,49) _(6d,24,49) _(92,24,49) _(ff,24,49)
 _(00,49,49) _(6d,49,49) _(92,49,49) _(ff,49,49)
 _(00,6d,49) _(6d,6d,49) _(92,6d,49) _(ff,6d,49)
 _(00,92,49) _(6d,92,49) _(92,92,49) _(ff,92,49)
 _(00,b6,49) _(6d,b6,49) _(92,b6,49) _(ff,b6,49)
 _(00,db,49) _(6d,db,49) _(92,db,49) _(ff,db,49)
 _(00,ff,49) _(6d,ff,49) _(92,ff,49) _(ff,ff,49)
 _(00,00,6d) _(6d,00,6d) _(92,00,6d) _(ff,00,6d)
 _(00,24,6d) _(6d,24,6d) _(92,24,6d) _(ff,24,6d)
 _(00,49,6d) _(6d,49,6d) _(92,49,6d) _(ff,49,6d)
 _(00,6d,6d) _(6d,6d,6d) _(92,6d,6d) _(ff,6d,6d)
 _(00,92,6d) _(6d,92,6d) _(92,92,6d) _(ff,92,6d)
 _(00,b6,6d) _(6d,b6,6d) _(92,b6,6d) _(ff,b6,6d)
 _(00,db,6d) _(6d,db,6d) _(92,db,6d) _(ff,db,6d)
 _(00,ff,6d) _(6d,ff,6d) _(92,ff,6d) _(ff,ff,6d)
 _(00,00,92) _(6d,00,92) _(92,00,92) _(ff,00,92)
 _(00,24,92) _(6d,24,92) _(92,24,92) _(ff,24,92)
 _(00,49,92) _(6d,49,92) _(92,49,92) _(ff,49,92)
 _(00,6d,92) _(6d,6d,92) _(92,6d,92) _(ff,6d,92)
 _(00,92,92) _(6d,92,92) _(92,92,92) _(ff,92,92)
 _(00,b6,92) _(6d,b6,92) _(92,b6,92) _(ff,b6,92)
 _(00,db,92) _(6d,db,92) _(92,db,92) _(ff,db,92)
 _(00,ff,92) _(6d,ff,92) _(92,ff,92) _(ff,ff,92)
 _(00,00,b6) _(6d,00,b6) _(92,00,b6) _(ff,00,b6)
 _(00,24,b6) _(6d,24,b6) _(92,24,b6) _(ff,24,b6)
 _(00,49,b6) _(6d,49,b6) _(92,49,b6) _(ff,49,b6)
 _(00,6d,b6) _(6d,6d,b6) _(92,6d,b6) _(ff,6d,b6)
 _(00,92,b6) _(6d,92,b6) _(92,92,b6) _(ff,92,b6)
 _(00,b6,b6) _(6d,b6,b6) _(92,b6,b6) _(ff,b6,b6)
 _(00,db,b6) _(6d,db,b6) _(92,db,b6) _(ff,db,b6)
 _(00,ff,b6) _(6d,ff,b6) _(92,ff,b6) _(ff,ff,b6)
 _(00,00,db) _(6d,00,db) _(92,00,db) _(ff,00,db)
 _(00,24,db) _(6d,24,db) _(92,24,db) _(ff,24,db)
 _(00,49,db) _(6d,49,db) _(92,49,db) _(ff,49,db)
 _(00,6d,db) _(6d,6d,db) _(92,6d,db) _(ff,6d,db)
 _(00,92,db) _(6d,92,db) _(92,92,db) _(ff,92,db)
 _(00,b6,db) _(6d,b6,db) _(92,b6,db) _(ff,b6,db)
 _(00,db,db) _(6d,db,db) _(92,db,db) _(ff,db,db)
 _(00,ff,db) _(6d,ff,db) _(92,ff,db) _(ff,ff,db)
 _(00,00,ff) _(6d,00,ff) _(92,00,ff) _(ff,00,ff)
 _(00,24,ff) _(6d,24,ff) _(92,24,ff) _(ff,24,ff)
 _(00,49,ff) _(6d,49,ff) _(92,49,ff) _(ff,49,ff)
 _(00,6d,ff) _(6d,6d,ff) _(92,6d,ff) _(ff,6d,ff)
 _(00,92,ff) _(6d,92,ff) _(92,92,ff) _(ff,92,ff)
 _(00,b6,ff) _(6d,b6,ff) _(92,b6,ff) _(ff,b6,ff)
 _(00,db,ff) _(6d,db,ff) _(92,db,ff) _(ff,db,ff)
 _(00,ff,ff) _(6d,ff,ff) _(92,ff,ff) _(ff,ff,ff)
*/
