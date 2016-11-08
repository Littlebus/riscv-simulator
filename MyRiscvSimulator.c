#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include "riscv.h"

int MODE = 0;
FILE* fp_elf = NULL;

int load_fromelf();
int excv_program();
int mode_debuger();
int load_program(long long phoff, int phsize, int phnum);
int load_section(long long shoff, int shsize, int shnum);

int main(int argc, char* argv[])
{
	if( argc<=2 )
	{
		printf("MyRiscvSimulator: fatal error: no input option or file\r\n");
		// printf("MyRiscvSimulator: invalid option -- 'y'\r\n");
		printf("Usage: ./MyRiscvSimulator <option> <file>\r\n");
		printf(" Load and excute program from object <file>.\r\n");
		printf(" One of the following options must be given:\r\n");
		printf("  -d, --debug mode  Excute an instruction each time and wait for user's command\r\n");
		printf("  -r, --run mode    Run the whole program without pause\r\n");
		exit(0);
	}

	if( argv[1][0]!='-' )
	{
		// printf("MyRiscvSimulator: fatal error: no input option or file\r\n");
		// printf("MyRiscvSimulator: invalid option -- 'y'\r\n");
		printf("Usage: ./MyRiscvSimulator <option> <file>\r\n");
		printf(" Load and excute program from object <file>.\r\n");
		printf(" One of the following options must be given:\r\n");
		printf("  -d, --debug mode  Excute an instruction each time and wait for user's command\r\n");
		printf("  -r, --run mode    Run the whole program without pause\r\n");
		exit(0);
	}

	switch( argv[1][1] )
	{
		case 'r': MODE = 0; break;
		case 'd': MODE = 1; break;
		default:
			// printf("MyRiscvSimulator: fatal error: no input option or file\r\n");
			printf("MyRiscvSimulator: invalid option -- '%c'\r\n", argv[1][1]);
			printf("Usage: ./MyRiscvSimulator <option> <file>\r\n");
			printf(" Load and excute program from object <file>.\r\n");
			printf(" One of the following options must be given:\r\n");
			printf("  -d, --debug mode  Excute an instruction each time and wait for user's command\r\n");
			printf("  -r, --run mode    Run the whole program without pause\r\n");
			exit(0);
	}

	if( !(fp_elf = fopen(argv[2], "r")) )
	{
		printf("MyRiscvSimulator: error: %s: No such file or directory\r\n", argv[2]);
		exit(0);
	}

	load_fromelf();
	excv_program();

	return 0;
}

int load_fromelf()
{
	char buf_ehdr[70] = {0};
	Elf64_Ehdr* p_ehdr = (Elf64_Ehdr*)buf_ehdr;
	
	if( !fread(buf_ehdr, 1, 64, fp_elf) )
	{
		printf("MyRiscvSimulator: file format error\r\n");
		exit(0);
	}
	/*  TEST:  check for the former 64 bytes  */
	/* unsigned* num_str_ehdr = (unsigned*)buf_ehdr;
	for(int i=0; i<16; i++)
	{
		printf("%x\r\n", num_str_ehdr[i]);
	} */

	if( !load_program( (long long)p_ehdr->e_phoff, (int)p_ehdr->e_phentsize, (int)p_ehdr->e_phnum ) )
	{
		printf("MyRiscvSimulator: file segment error\r\n");
		exit(0);
	}

	if( !load_section( (long long)p_ehdr->e_shoff, (int)p_ehdr->e_shentsize, (int)p_ehdr->e_shnum ) )
	{
		printf("MyRiscvSimulator: file section error\r\n");
		exit(0);
	}

	if( fclose(fp_elf)==EOF )
	{
		printf("MyRiscvSimulator: file close error\r\n");
		exit(0);
	}

	PC = p_ehdr->e_entry;
	i_reg[2] = MEM>>1;

	return 0;
}

int load_program(long long phoff, int phsize, int phnum)
{
	int i = 0;
	char buf_phdr[70] = {0};
	Elf64_Phdr* p_phdr = (Elf64_Phdr*)buf_phdr;

	if( !fp_elf )
	{
		return 0;
	}

	for(i=0; i<phnum; i++)
	{
		if( fseek(fp_elf, phoff+i*phsize, SEEK_SET) )
		{
			return 0;
		}
		if( !fread(buf_phdr, 1, phsize, fp_elf) )
		{
			return 0;
		}

		if( p_phdr->p_type == PT_LOAD )
		{
			if( fseek(fp_elf, p_phdr->p_offset, SEEK_SET) )
			{
				return 0;
			}
			if( !fread(&mem[p_phdr->p_vaddr], 1, p_phdr->p_filesz, fp_elf) )
			{
				return 0;
			}
		}
	}

	return 1;
}

int load_section(long long shoff, int shsize, int shnum)
{
	int i = 0;
	char buf_shdr[70] = {0};
	Elf64_Shdr* p_shdr = (Elf64_Shdr*)buf_shdr;

	if( !fp_elf )
	{
		return 0;
	}

	for(i=0; i<shnum; i++)
	{
		if( fseek(fp_elf, shoff+i*shsize, SEEK_SET) )
		{
			return 0;
		}
		if( !fread(buf_shdr, 1, shsize, fp_elf) )
		{
			return 0;
		}

		if( p_shdr->sh_addr && p_shdr->sh_type!=SHT_NOBITS )
		{
			if( fseek(fp_elf, p_shdr->sh_offset, SEEK_SET) )
			{
				return 0;
			}
			if( !fread(&mem[p_shdr->sh_addr], 1, p_shdr->sh_size, fp_elf) )
			{
				return 0;
			}
		}
	}

	return 1;
}

int excv_program()
{
	/*  TEST:  check for memory bytes  */
	/* for( ; PC<=0x131ec; PC+=4)
	{
		printf( "   0x%llx:       %08x\r\n", PC, *(int*)(&mem[PC]) );
	} */

	int turn = 0;
	for( ; turn<0x1000 && !END; turn++)
	{
		long long tmp_pc = PC;

		printf("0x%llx\r\n", PC);
		if( PC>MEM-4 )
		{
			printf("MyRiscvSimulator: segmentation fault: PC(0x%llx) is out of range\r\n", PC);
			exit(0);
		}

		if(MODE==1)
		{
			mode_debuger();
		}
							
		instruction_execute( *(int*)(&mem[PC]) );		if(PC==0x10488)  printf("%llx  %lld\r\n", i_reg[1], *(long long*)&mem[i_reg[2]+40]);

		if( PC==tmp_pc )
		{
			PC += 4;
		}
		i_reg[0] = 0;
	}

	return 0;
}

int mode_debuger()
{
	// while()
}





