#include "stdint.h"
#include "stdio.h"




typedef struct{
  uint64_t space[4];
}SAT;

typedef struct{
  uint8_t pos;
  uint8_t neg;
}CONSTRAINT;

static const uint64_t SATFULL[] = {0xffffffffffffffff,
                                   0xffffffffffffffff,
                                   0xffffffffffffffff,
                                   0xffffffffffffffff};



/*
  This is *mostly* working.

  There seems to still be a bug where bits 4 and 5 in the constraints are
    getting confused. I have absolutely no idea why. Probably some stupid
    UB optimization reasons. This code is full of little compiler gotchas.

  Everything else seems to mostly work fine though. For the most part.
*/
void constrain(SAT* s, CONSTRAINT c){
  static const uint64_t ks[6] = {~0x5555555555555555,
                                 ~0x3333333333333333,
                                 ~0x0f0f0f0f0f0f0f0f,
                                 ~0x00ff00ff00ff00ff,
                                 ~0x0000ffff0000ffff,
                                 ~0x00000000ffffffff};

  static const uint64_t FULL =    0xffffffffffffffff;


  // No constraints, everything's fine.
  if((c.pos == 0) && (c.neg == 0)) return;

  SAT pos;

  // Constrain pos by all positive constraints
  uint64_t posmall = 0;
  for(int i = 0; i < 6; i++){
    posmall |=  ks[i] & (0l - (0 != (c.pos & (1l << i))));
  }

  for(int i = 0; i < 4; i++) pos.space[i] = posmall;

  uint64_t x6 = (0l - (0 != (c.pos &  64)));
  pos.space[0] |= FULL & x6;
  pos.space[1] |= 0     & x6;
  pos.space[2] |= FULL & x6;
  pos.space[3] |= 0     & x6;

  uint64_t x7 = (0l - (0 != (c.pos & 128)));
  pos.space[0] |= FULL & x7;
  pos.space[1] |= FULL & x7;
  pos.space[2] |= 0     & x7;
  pos.space[3] |= 0     & x7;


  SAT neg;

  // Constrain neg by all negative constraints
  uint64_t negmall = 0;
  for(int i = 0; i < 6; i++){
    negmall |= ~ks[i] & (0l - (0 != (c.neg & (1l << i))));
  }

  for(int i = 0; i < 4; i++) neg.space[i] = negmall;

  uint64_t y6 = (0l - (0 != (c.neg &  64)));
  neg.space[0] |= 0     & y6;
  neg.space[1] |= FULL & y6;
  neg.space[2] |= 0     & y6;
  neg.space[3] |= FULL & y6;

  uint64_t y7 = (0l - (0 != (c.neg & 128)));
  neg.space[0] |= 0     & y7;
  neg.space[1] |= 0     & y7;
  neg.space[2] |= FULL & y7;
  neg.space[3] |= FULL & y7;

  //Merge pos, neg, and s
  for(int i = 0; i < 4; i++){
    s->space[i] &= (pos.space[i] | neg.space[i]);
  }
}


void runSAT(SAT* s, CONSTRAINT* cs, int cct){
  for(int i = 0; i < cct; i++)
    constrain(s, cs[i]);
}




int main(){

  /*
    +1 +2 -3
    -1 +3
    -2 +3
  */
  CONSTRAINT cs[8];

  cs[0].pos = 3; cs[0].neg = 4;
  cs[1].pos = 4; cs[1].neg = 1;
  cs[2].pos = 4; cs[2].neg = 2;

  cs[3].pos = 0; cs[3].neg =   8;
  cs[4].pos = 0; cs[4].neg =  16;
  cs[5].pos = 0; cs[5].neg =  32;
  cs[6].pos = 0; cs[6].neg =  64;
  cs[7].pos = 0; cs[7].neg = 128;

  for(int i = 0; i < 8; i++){
    SAT s;
    for(int i = 0; i < 4; i++)
      s.space[i] = SATFULL[i];

    runSAT(&s, cs, i+1);

    printf("%16lx %16lx %16lx %16lx\n", s.space[0], s.space[1], s.space[2], s.space[3]);
  }

}
