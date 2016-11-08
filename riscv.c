#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "riscv.h"

void mul_unsigned_64(long long rs1, long long rs2, long long rd, long long funct3)
{
	long long tmp1 = i_reg[rs1];
	long long tmp2 = i_reg[rs2];
	int i, j;
	int type = 1;
	switch (funct3)
	{
	case 0b000:				//MUL
	case 0b001:				//MULH
		if (tmp1<0)
		{
			type = !type;
			if (tmp1>((long long)1) << 63)
				tmp1 = ~tmp1 + 1;
		}
		if (tmp2<0)
		{
			type = !type;
			if (tmp2>((long long)1) << 63)
				tmp2 = ~tmp2 + 1;
		}
		break;

	case 0b010:				//MULHSU
		if (tmp1<0)
		{
			type = !type;
			if (tmp1>((long long)1) << 63)
				tmp1 = ~tmp1 + 1;
		}
		break;
	}

	unsigned long long multcd = tmp1;
	unsigned long long multer = tmp2;

	unsigned int arr_multer[10];
	unsigned int arr_multcd[10];
	unsigned int result[20];

	for (i = 0; i<10; i++)
	{
		arr_multer[i] = 0;
		arr_multcd[i] = 0;
		result[i] = 0;
		result[i + 10] = 0;
	}

	for (i = 0; i<10; i++)
	{
		arr_multer[i] = multer % 64;
		arr_multcd[i] = multcd % 64;
		multer /= 64;
		multcd /= 64;
	}

	for (i = 0; i<10; i++)
	{
		for (j = 0; j<10; j++)
		{
			result[i + j] += arr_multcd[i] * arr_multer[j];
		}
	}

	for (i = 0; i<19; i++)
	{
		if (result[i] >= 64)
		{
			result[i + 1] += result[i] / 64;
			result[i] %= 64;
		}
	}

	unsigned long long high_ans = 0;
	unsigned long long low_ans = 0;

	for (i = 0; i<8; i++)
	{
		high_ans += result[i + 8] * pow(64, i);
		low_ans += result[i] * pow(64, i);
	}

	if (type == 0)
	{
		if (low_ans == 0)
		{
			low_ans = 0;
			high_ans = ~high_ans + 1;
		}
		else
		{
			low_ans = ~low_ans + 1;
			high_ans = ~high_ans;
		}
	}

	switch (funct3)
	{
	case 0b000:			//MUL
		i_reg[rd] = low_ans;
		break;

	default:			//MULH, MULHSU, MULHU
		i_reg[rd] = high_ans;
	}
}

void instruction_execute(int instruction)
{
	long long opcode = OPCODE(instruction);
	long long rd = RD(instruction);
	long long rs1 = RS1(instruction);
	long long rs2 = RS2(instruction);
	long long rs3 = RS3(instruction);
	long long rm = RM(instruction);
	long long shamt = SHAMT(instruction);
	long long u_imm = U_IMM(instruction);
	long long j_imm = J_IMM(instruction);
	long long i_imm = I_IMM(instruction);
	long long s_imm = S_IMM(instruction);
	long long b_imm = B_IMM(instruction);
	long long fmt = FMT(instruction);
	long long funct3 = FUNCT3(instruction);
	long long funct7 = FUNCT7(instruction);
	double tempd;
	long long templ;
	int si_tmp1, si_tmp2, si_tmp3, si_tmp4;
	unsigned int ui_tmp1, ui_tmp2, ui_tmp3, ui_tmp4;
	long long sl_tmp1, sl_tmp2, sl_tmp3, sl_tmp4;
	unsigned long long ul_tmp1, ul_tmp2, ul_tmp3, ul_tmp4;
	float tempf;
	int tempi;

	switch (opcode)
	{
	case 0b0000011:
		switch (funct3)
		{
			//LB
		case 0b000:
			i_reg[rd] = SIGNEX_64(MEM_8(i_reg[rs1] + i_imm));
			break;
			//LH
		case 0b001:
			i_reg[rd] = SIGNEX_64(MEM_16(i_reg[rs1] + i_imm));
			break;
			//LW
		case 0b010:
			i_reg[rd] = SIGNEX_64(MEM_32(i_reg[rs1] + i_imm));
			break;
			//LD
		case 0b011:
			i_reg[rd] = MEM_64(i_reg[rs1] + i_imm);
			break;
			//LBU
		case 0b100:
			i_reg[rd] = UNSIGNEX_64(MEM_8(i_reg[rs1] + i_imm));
			break;
			//LHU
		case 0b101:
			i_reg[rd] = UNSIGNEX_64(MEM_16(i_reg[rs1] + i_imm));
			break;
			//LWU
		case 0b110:
			i_reg[rd] = UNSIGNEX_64(MEM_32(i_reg[rs1] + i_imm));
			break;
		}
		break;

		//STORE
	case 0b0100011:
		switch (funct3)
		{
			//SB
		case 0b000:
			*(mem + rs1 + s_imm) = LOW_8(i_reg[rs2]);
			break;
			//SH
		case 0b001:
			*((short *)(mem + rs1 + s_imm)) = LOW_16(i_reg[rs2]);
			break;
			//SW
		case 0b010:
			*((int *)(mem + rs1 + s_imm)) = LOW_32(i_reg[rs2]);
			break;
			//SD
		case 0b011:
			*((long long *)(mem + rs1 + s_imm)) = i_reg[rs2];
			break;
		}
		break;

	case 0b0000111:
		switch (funct3)
		{
			//FLW
		case 0b010:
			// d_reg[rd] = FMEM_32(mem + i_imm + rs1);
			break;
			//FLD
		case 0b011:
			// pass();
			break;
		}
		break;

	case 0b0100111:
		switch (funct3)
		{
		case 0b010://FSW
			*((float *)(mem + rs1 + s_imm)) = d_reg[rs2];
			//FSD
		}
		break;

	case 0b1000011:	//FMADD
		tempd = d_reg[rs1] * d_reg[rs2] + d_reg[rs3];
		d_reg[rd] = fmt == 0b00 ? (float)tempd : tempd;
		break;

	case 0b1000111:	//FMSUB
		tempd = d_reg[rs1] * d_reg[rs2] - d_reg[rs3];
		d_reg[rd] = fmt == 0b00 ? (float)tempd : tempd;
		break;

	case 0b1001011:	//FNMSUB
		tempd = -(d_reg[rs1] * d_reg[rs2] - d_reg[rs3]);
		d_reg[rd] = fmt == 0b00 ? (float)tempd : tempd;
		break;

	case 0b1001111:	//FNMADD
		tempd = -(d_reg[rs1] * d_reg[rs2] + d_reg[rs3]);
		d_reg[rd] = fmt == 0b00 ? (float)tempd : tempd;
		break;

	case 0b1010011:
		switch (funct7)
		{
		case 0b0000000://FADD
			d_reg[rd] = (float)(d_reg[rs1] + d_reg[rs2]);
			break;
		case 0b0000001:
			d_reg[rd] = (d_reg[rs1] + d_reg[rs2]);
			break;
		case 0b0000100://FSUB
			d_reg[rd] = (float)(d_reg[rs1] - d_reg[rs2]);
			break;
		case 0b0000101:
			d_reg[rd] = (d_reg[rs1] - d_reg[rs2]);
			break;
		case 0b0001000://FMUL
			d_reg[rd] = (float)(d_reg[rs1] * d_reg[rs2]);
			break;
		case 0b0001001:
			d_reg[rd] = (d_reg[rs1] * d_reg[rs2]);
			break;
		case 0b0001100://FDIV
			d_reg[rd] = (float)(d_reg[rs1] / d_reg[rs2]);
			break;
		case 0b0001101:
			d_reg[rd] = (d_reg[rs1] / d_reg[rs2]);
			break;
		case 0b0101100://FSQRT
			d_reg[rd] = (float)sqrt(d_reg[rs1]);
			break;
		case 0b0101101://FSQRT
			d_reg[rd] = sqrt(d_reg[rs1]);
			break;

			/*
			case 0b0010000://FSGNJ
			tempf = (float)d_reg[rs1];
			tempi = (*(int*)(&tempf)) & 0x7FFF;
			tempf = *(float *)(&tempi);

			switch (rm) {
			case 0b000://FSGNJ.S
			d_reg[rd] = tempf | ((float)d_reg[rs2] & (1 << 31));
			break;
			case 0b001://FSGNJN.S
			d_reg[rd] = tempf | (~(((float)d_reg[rs2] & (1 << 31)) >> 31) & 1) << 31;
			break;
			case 0b010://FSGNJX.S
			d_reg[rd] = tempf | ((float)d_reg[rs2] & (1 << 31)) ^ ((float)d_reg[rs1] & (1 << 31));
			break;
			}
			break;
			case 0b0010001:
			double tempd = d_reg[rs1] & 0x7FFFFFFF;
			switch (rm) {
			case 0b000://FSGNJ.D
			d_reg[rd] = tempd | (d_reg[rs2] & (1 << 63));
			break;
			case 0b001://FSGNJN.D
			d_reg[rd] = tempd | (~((d_reg[rs2] & (1 << 63)) >> 63) & 1) << 63;
			break;
			case 0b010://FSGNJX.D
			d_reg[rd] = tempd | (d_reg[rs2] & (1 << 63)) ^ (d_reg[rs1] & (1 << 63))
			break;
			}
			break;
			*/

		case 0b0010100://FMIN,MAX.S
			d_reg[rd] = (float)(rm == 0b000 ? (d_reg[rs1] < d_reg[rs2] ? d_reg[rs1] : d_reg[rs2]) : (d_reg[rs1] < d_reg[rs2] ? d_reg[rs1] : d_reg[rs2]));
			break;

		case 0b0010101://FMIN,MAX.D
			d_reg[rd] = rm == 0b000 ? (d_reg[rs1] < d_reg[rs2] ? d_reg[rs1] : d_reg[rs2]) : (d_reg[rs1] < d_reg[rs2] ? d_reg[rs1] : d_reg[rs2]);
			break;

		case 0b1100000:
			switch (rs2) {
			case 0b00000://FCVT.W.S
				i_reg[rd] = (int)(float)d_reg[rs1];
				break;
			case 0b00001://FCVT.WU.S
				i_reg[rd] = (unsigned int)(float)d_reg[rs1];
				break;
			case 0b00010://FCVT.L.S
				i_reg[rd] = (long long)(float)d_reg[rs1];
				break;
			case 0b00011://FCVT.LU.S
				i_reg[rd] = (unsigned long long)(float)d_reg[rs1];
				break;
			}
			break;

		case 0b1100001:
			switch (rs2) {
			case 0b00000://FCVT.W.D
				i_reg[rd] = (int)d_reg[rs1];
				break;
			case 0b00001://FCVT.WU.D
				i_reg[rd] = (unsigned int)d_reg[rs1];
				break;
			case 0b00010://FCVT.L.D
				i_reg[rd] = (long long)d_reg[rs1];
				break;
			case 0b00011://FCVT.LU.D
				i_reg[rd] = (unsigned long long)d_reg[rs1];
				break;
			}
			break;

		case 0b1110000:
			if (rm == 0b000)//FMV.X.S
			{
				i_reg[rd] = *(int *)(d_reg + rs1);
				i_reg[rd] = (i_reg[rd] >> 31) & 1 ? i_reg[rd] : i_reg[rd] | 0xFFFF0000;
			}
			else//FCLASS.S不写了
			{
			}
			break;

		case 0b1110001:
			if (rm == 0b000)//FMV.X.D
			{
				i_reg[rd] = *(long long *)(d_reg + rs1);
			}
			else//FCLASS.D不写了
			{
			}
			break;

		case 0b1010000:
			switch (rm) {
			case 0b000://FLE.S
				break;
			case 0b001://FLT.S
				break;
			case 0b010://FEQ.S
				break;
			}
			break;

		case 0b1010001:
			switch (rm) {
			case 0b000://FLE.D
				break;
			case 0b001://FLT.D
				break;
			case 0b010://FEQ.D
				break;
			}
			break;

		case 0b1101000://FCVT.S.W&WU
			switch (rs2) {
			case 0b0000://FCVT.S.W
				d_reg[rd] = (float)(i_reg[rs1]);
				break;
			case 0b0001://FCVT.S.WU
				d_reg[rd] = (float)(unsigned)(i_reg[rs1]);
				break;
			case 0b00010://FCVT.S.L
				d_reg[rd] = (float)(i_reg[rs1]);
				break;
			case 0b00011://FCVT.S.LU
				d_reg[rd] = (float)(unsigned long long)(d_reg[rs1]);
				break;
			}
			break;

		case 0b1101001://FCVT.D
			switch (rs2) {
			case 0b00000://FCVT.D.W
				d_reg[rd] = i_reg[rs1];
				break;
			case 0b0001://FCVT.D.WU
				d_reg[rd] = (unsigned)(i_reg[rs1]);
				break;
			case 0b00010://FCVT.D.L
				d_reg[rd] = i_reg[rs1];
				break;
			case 0b00011://FCVT.D.LU
				d_reg[rd] = (unsigned long long)(d_reg[rs1]);
				break;
			}
			break;

		case 0b1111000://FMV.S.X
			tempf;
			*(int *)(&tempf) = LOW_32(i_reg[rs1]);
			d_reg[rd] = tempf;
			break;

		case 0b1111001://FMV.D.X
			*(long long *)(d_reg + rd) = i_reg[rs1];
			break;
		}

	case 0b0110111:			// LUI
		i_reg[rd] = u_imm;
		break;

	case 0b0010111:			//AUIPC
		i_reg[rd] = PC + u_imm;
		break;

	case 0b1101111:			//JAL
		i_reg[rd] = PC + 4;
		PC += j_imm;
		break;

	case 0b1100111:			//JALR
		i_reg[rd] = PC + 4;
		PC = i_reg[rs1] + i_imm;
		break;

	case 0b1100011:			//BRANCH series
		switch (funct3)
		{
		case 0b000:			//BEQ
			if (i_reg[rs1] == i_reg[rs2])
				PC += b_imm;
			break;

		case 0b001:			//BNE
			if (i_reg[rs1] != i_reg[rs2])
				PC += b_imm;
			break;

		case 0b100:			//BLT
			if (i_reg[rs1]<i_reg[rs2])
				PC += b_imm;
			break;

		case 0b101:			//BGE
			if (i_reg[rs1] >= i_reg[rs2])
				PC += b_imm;
			break;

		case 0b110:			//BLTU
			if ((unsigned)(i_reg[rs1])<(unsigned)(i_reg[rs2]))
				PC += b_imm;
			break;

		case 0b111:			//BGEU
			if ((unsigned)(i_reg[rs1]) >= (unsigned)(i_reg[rs2]))
				PC += b_imm;
		}
		break;

	case 0b0010011:			//REG-IMM series
		switch (funct3)
		{
		case 0b000:			//ADDI
			i_reg[rd] = i_reg[rs1] + i_imm;
			break;

		case 0b010:			//SLTI
			if (i_reg[rs1]<i_imm)
				i_reg[rd] = 1;
			else
				i_reg[rd] = 0;
			break;

		case 0b011:			//SLTIU
			ui_tmp1 = i_imm;
			ul_tmp1 = ui_tmp1;
			if ((unsigned)(i_reg[rs1])<ul_tmp1)
				i_reg[rd] = 1;
			else
				i_reg[rd] = 0;
			break;

		case 0b100:			//XORI
			i_reg[rd] = i_reg[rs1] ^ i_imm;
			break;

		case 0b110:			//ORI
			i_reg[rd] = i_reg[rs1] | i_imm;
			break;

		case 0b111:			//ANDI
			i_reg[rd] = i_reg[rs1] & i_imm;
			break;

		case 0b001:			//SLLI
			i_reg[rd] = i_reg[rs1] << shamt;
			break;

		case 0b101:
			if (funct7 == 0)	//SRLI
				i_reg[rd] = ((unsigned)(i_reg[rs1])) >> shamt;
			else				//SRAI
				i_reg[rd] = i_reg[rs1] >> shamt;
		}
		break;

	case 0b0110011:			//CAL
		switch (funct7)
		{
		case 1:
			switch (funct3)
			{
			case 0b100:			//DIV
				i_reg[rd] = i_reg[rs1] / i_reg[rs2];
				break;

			case 0b101:			//DIVU
				i_reg[rd] = ((unsigned)(i_reg[rs1])) / ((unsigned)(i_reg[rs2]));
				break;

			case 0b110:			//REM
				i_reg[rd] = i_reg[rs1] % i_reg[rs2];
				break;

			case 0b111:			//REMU
				i_reg[rd] = ((unsigned)(i_reg[rs1])) % ((unsigned)(i_reg[rs2]));
				break;

			default:
				mul_unsigned_64(rs1, rs2, rd, funct3);
			}
			break;

		default:
			switch (funct3)
			{
			case 0:
				//ADD
				if (funct7 == 0)
					i_reg[rd] = i_reg[rs1] + i_reg[rs2];
				//SUB
				else
					i_reg[rd] = i_reg[rs1] - i_reg[rs2];
				break;

				//SLL
			case 1:
				i_reg[rd] = i_reg[rs1] << i_reg[rs2];
				break;

				//SLT
			case 2:
				i_reg[rd] = i_reg[rs1] < i_reg[rs2] ? 1 : 0;
				break;

				//SLTU
			case 3:
				i_reg[rd] = (unsigned)i_reg[rs1] < (unsigned)i_reg[rs2] ? 1 : 0;
				break;

				//XOR
			case 4:
				i_reg[rd] = i_reg[rs1] ^ i_reg[rs2];
				break;

			case 5:
				//SRL
				if (funct7 == 0)
					i_reg[rd] = ((unsigned)i_reg[rs1]) >> i_reg[rs2];
				//SRA
				else
					i_reg[rd] = i_reg[rs1] >> i_reg[rs2];
				break;

				//OR
			case 6:
				i_reg[rd] = i_reg[rs1] | i_reg[rs2];
				break;

				//AND
			case 7:
				i_reg[rd] = i_reg[rs1] & i_reg[rs2];
			}
		}
		break;

	case 0b0011011:
		switch (funct3)
		{
		case 0b000:			//ADDIW
			si_tmp1 = I_IMM(instruction);
			si_tmp2 = i_reg[rs1];
			i_reg[rd] = si_tmp1 + si_tmp2;
			break;

		case 0b001:			//SLLIW
			si_tmp1 = i_reg[rs1];
			i_reg[rd] = si_tmp1 << shamt;
			break;

		case 0b101:
			if (funct7 == 0)		//SRLIW
			{
				ui_tmp1 = LOW_32(i_reg[rs1]);
				i_reg[rd] = ui_tmp1 >> shamt;
			}
			else				//SRAIW
			{
				si_tmp1 = LOW_32(i_reg[rs1]);
				i_reg[rd] = si_tmp1 >> shamt;
			}
		}
		break;

	case 0b0111011:
		si_tmp1 = LOW_32(i_reg[rs1]);
		si_tmp2 = LOW_32(i_reg[rs2]);
		ui_tmp3 = LOW_32(i_reg[rs1]);
		ui_tmp4 = LOW_32(i_reg[rs2]);
		switch (funct3)
		{
		case 0b000:
			if (funct7 == 0)		//ADDW
				i_reg[rd] = si_tmp1 + si_tmp2;

			else if (funct7 == 1)	//MULW
				i_reg[rd] = si_tmp1*si_tmp2;

			else				//SUBW
				i_reg[rd] = si_tmp1 - si_tmp2;
			break;

		case 0b100:			//DIVW
			i_reg[rd] = si_tmp1 / si_tmp2;
			break;

		case 0b001:			//SLLW
			i_reg[rd] = si_tmp1 << si_tmp2;
			break;

		case 0b110:			//REMW
			i_reg[rd] = si_tmp1%si_tmp2;
			break;

		case 0b111:			//REMUW
			si_tmp3 = ui_tmp3%ui_tmp4;
			i_reg[rd] = si_tmp3;
			break;

		case 0b101:
			if (funct7 == 0)		//SRLW
				i_reg[rd] = ui_tmp3 >> ui_tmp4;

			else if (funct7 == 1)	//DIVUW
			{
				si_tmp3 = ui_tmp3 / ui_tmp4;
				i_reg[rd] = si_tmp3;
			}

			else				//SRAW
				i_reg[rd] = si_tmp1 >> si_tmp2;
		}
		break;

	case 0b1110011:
		ecall();
		break;

	default:
		printf("Undefined opcode %lld\r\n", opcode);
		exit(0);
	}
}
