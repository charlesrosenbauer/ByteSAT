#include "stdint.h"
#include "stdio.h"
#include "time.h"





typedef struct{
  uint64_t bits[4];
}SAT;





SAT constrain(uint8_t pos, uint8_t neg){
  static const uint64_t ks[6] = {0xAAAAAAAAAAAAAAAA,
                                 0x6666666666666666,
                                 0xf0f0f0f0f0f0f0f0,
                                 0xff00ff00ff00ff00,
                                 0xffff0000ffff0000,
                                 0xffffffff00000000};

  SAT ret;

  uint64_t p = 0;
  p |= (pos &  1)?  ks[0] : 0;
  p |= (pos &  2)?  ks[1] : 0;
  p |= (pos &  4)?  ks[2] : 0;
  p |= (pos &  8)?  ks[3] : 0;
  p |= (pos & 16)?  ks[4] : 0;
  p |= (pos & 32)?  ks[5] : 0;

  uint64_t n = 0;
  n |= (neg &  1)? ~ks[0] : 0;
  n |= (neg &  2)? ~ks[1] : 0;
  n |= (neg &  4)? ~ks[2] : 0;
  n |= (neg &  8)? ~ks[3] : 0;
  n |= (neg & 16)? ~ks[4] : 0;
  n |= (neg & 32)? ~ks[5] : 0;

  uint64_t bits = p | n;
  ret.bits[0] = ((neg & 64) | (neg & 128))? 0xffffffffffffffff : bits;
  ret.bits[1] = ((pos & 64) | (neg & 128))? 0xffffffffffffffff : bits;
  ret.bits[2] = ((neg & 64) | (pos & 128))? 0xffffffffffffffff : bits;
  ret.bits[3] = ((pos & 64) | (pos & 128))? 0xffffffffffffffff : bits;

  return ret;
}



SAT and(SAT a, SAT b){
  SAT ret;
  for(int i = 0; i < 4; i++)
    ret.bits[i] = a.bits[i] & b.bits[i];
  return ret;
}


void printSAT(SAT x){
  printf("X: %lx %lx %lx %lx\n\n", x.bits[0], x.bits[1], x.bits[2], x.bits[3]);
}


void benchmark(int64_t n){
  SAT x;
  for(int i = 0; i < 4; i++) x.bits[i] = 0xffffffffffffffff;

  // Generate random constraints
  uint64_t rstate = 7596458374521;
  uint8_t poss[8192];
  uint8_t negs[8192];
  for(int i = 0; i < 8192; i++){
    rstate  = (781708961081 * rstate) + 5781967161115;
    int rot =  rstate >> 58;
    rstate  = (rstate >> rot) | (rstate << (64 - rot));
    poss[i] =  rstate         & 0xff;
    negs[i] = (rstate >>   8) & 0xff;
  }

  // Run the actual loop
  time_t t = clock();
  for(int64_t i = 0; i < n; i++){
    x = and(x, constrain(poss[i%8192], negs[i%8192]));
  }
  printSAT(x);
  t = clock() - t;
  printf("%li iterations takes %f seconds to run.\n", n, ((double)t)/CLOCKS_PER_SEC);
}



int main(){

  SAT x;
  for(int i = 0; i <  4; i++) x.bits[i] = 0xffffffffffffffff;

  /*
    Test code.
    This is basically a SAT instance with a couple of AND gates that must
    generate equal results, and some extra constraints to set the remaining
    parameters to both be zero.
  */
  // +1 +2 -3
  x = and(x, constrain(0x03, 0x04));
  printSAT(x);
  // -1 +3
  x = and(x, constrain(0x04, 0x01));
  printSAT(x);
  // -2 +3
  x = and(x, constrain(0x04, 0x02));
  printSAT(x);
  // +4 +5 -6
  x = and(x, constrain(0x18, 0x20));
  printSAT(x);
  // -4 +6
  x = and(x, constrain(0x20, 0x08));
  printSAT(x);
  // -5 +6
  x = and(x, constrain(0x20, 0x10));
  printSAT(x);
  // -3 +6
  x = and(x, constrain(0x20, 0x04));
  printSAT(x);
  // +3 -6
  x = and(x, constrain(0x04, 0x20));
  printSAT(x);
  // -7
  x = and(x, constrain(0x00, 0x40));
  printSAT(x);
  // +8
  x = and(x, constrain(0x00, 0x80));
  printSAT(x);

  benchmark(100000000);
}
