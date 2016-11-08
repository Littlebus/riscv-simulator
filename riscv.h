#ifndef RISCV_H
#define RISCV_H

#define MEM		(1<<28)
#define OPCODE(x)	(x&127)
#define RD(x)		((x>>7)&31)
#define RS1(x)		((x>>15)&31)
#define RS2(x)		((x>>20)&31)
#define RS3(x)		((x>>27)&31)
#define RM(x)		((x>>12)&7)
#define RS3(x)		((x>>27)&31)
#define FMT(x)		((x>>25)&3)
#define FUNCT3(x)	((x>>12)&7)
#define FUNCT7(x)	((x>>25)&127)
#define SHAMT(x)	((x>>20)&31)

#define I_IMM(x)	(((x&(1<<31))>>20)|(((x>>25)&63)<<5)|(((x>>21)&15)<<1)|(x>>20&1))
#define S_IMM(x)	(((x&(1<<31))>>20)|(((x>>25)&63)<<5)|(((x>>8)&15)<<1)|(x>>7&1))
#define B_IMM(x)	(((x&(1<<31))>>19)|((x>>7&1)<<11)|(((x>>25)&63)<<5)|(((x>>8)&15)<<1))
#define U_IMM(x)	(x&0xfffff000)
#define J_IMM(x)	(((x&(1<<31))>>11)|(((x>>12)&255)<<12)|(((x>>20)&1)<<11)|(((x>>25)&63)<<5)|(((x>>21)&15)<<1))
#define MEM_64(x)	((long long)*((long long *)(mem+x)))
#define MEM_32(x)	((long long)*((int *)(mem+x)))
#define MEM_16(x)	((long long)*((short *)(mem+x)))
#define MEM_8(x)	((long long)*((char *)(mem+x)))
#define LOW_32(x)	(x&(0xFFFFFFFF))
#define HIGH_32(x)	((x&(0xFFFFFFFF00000000))>>32)
#define LOW_16(x)	(x&(0xFFFF))
#define HIGH_16(x)	((x&(0xFFFF000000000000))>>48)
#define LOW_8(x)	(x&(0xFF))
#define HIGH_8(x)	((x&(0xFF00000000000000))>>56)
#define SIGNEX_64(x)	(((((long long)x&(1<<31))<<32)>>31)|x)
#define UNSIGNEX_64(x)	((x&0x00000000FFFFFFFF))
#define DMEM(x)		((double)*((double *)(mem+x)))
#define FMEM(x)		((double)*((float *)(mem+x)))

int END;
long long i_reg[32];
char mem[MEM];
long long PC;
double d_reg[32];
void ecall(void);
void instruction_execute(int);

#endif
