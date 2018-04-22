#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem[64*1024];
word reg[8];


#define pc reg[7]
#define LO(x) ((x) & 0xFF)
#define HI(x) (((x)>>8) & 0xFF)
#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1<<1)
#define HAS_XX (1<<2)
#define HAS_NN (1<<3)

struct ModReg
{
	word adress; 
	word value;
} ss, dd, nn;

byte b_read  (adr a);            // читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);  // пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);            // читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);  // пишет значение val в "старую память" mem в слово с "адресом" a.
void load_file();                //читает из стандартного потока ввода данные исполняемого файла
void f_load_file();
void mem_dump(adr start, word n);//печатает на стандартный поток вывода n байт, начиная с адреса start                               
void test_mem();
void do_unknown();
void do_add();
void do_mov();
void do_halt();
void do_sob();
void do_clr();
void print_registers();
void run(adr pc0);
struct ModReg get_dd(word w);
struct ModReg get_nn(word w);

struct Command 
{
	word opcode;
	word mask;
	char * name;
	void (*func)();
	byte param;
} commands[] = {
	{0000000, 0177777, "HALT", do_halt, NO_PARAM}, // 0xFFFF 
	{0010000, 0170000, "MOV",  do_mov, HAS_SS | HAS_DD}, 
	{0060000, 0170000, "ADD",  do_add, HAS_SS | HAS_DD}, 
	{0077000, 0177000, "SOB",  do_sob, HAS_NN},
	{0050000, 0177700, "CLR",  do_clr, HAS_DD},
	{0000000, 0000000, "UNCNOWN", do_unknown}   // MUST BE THE LAST
};

int main() 
{
	f_load_file();
	load_file();
	run(0x200);
	return 0;
}

byte b_read  (adr a) 
{
	return mem[a];
}

void b_write (adr a, byte val) 
{
	mem[a] = val;
}

word w_read  (adr a)
{
	assert(a % 2 == 0);
	return ((word)mem[a])|(((word)mem[a + 1])<<8);
}

void w_write (adr a, word val) 
{
	assert(a % 2 == 0);
	// val = 0x0b0a
	// a = 2
	// mem[2] = 0x0a
	mem[a] = (byte)(val & 0xFF);    // (byte)val;
	// mem[3] = 0x0b
	mem[a+1] = (byte)((val>>8) & 0xFF);
}

void load_file() 
{
	unsigned int start, n, x, i;
	while (scanf("%x%x", &start, &n) == 2) {
		for(i = 0; i < n; i++) {
			scanf("%x", &x);
			b_write(start, x);
			start++;
		}
	}
}

void f_load_file() 
{
	unsigned int start, n, x, i;
	FILE * f;
	char * s = (char *)malloc(100 * sizeof(char));
	scanf("%s", s);
	f = fopen(s, "r");
	free(s);
	if (f == NULL)
	{
		printf("Can't open file.\n");
	}
	while(fscanf(f, "%x%x", &start, &n) == 2) 
	{
		for(i = 0; i < n; i++) 
		{
			fscanf(f, "%x", &x);
			b_write(start, x);
			start++;
		}
	}
	fclose(f);
}

void mem_dump(adr start, word n) 
{
	load_file();
	unsigned int i;
	for(i = 0; i < n; i+=2) 
	{
		printf("%06o : %06o\n", start, w_read(start));
		start +=2;
	}
}

void test_mem() 
{
	byte b0, b1;
	word w;
	w = 0x0d0c;
	w_write(4, w);
	b0 = b_read(4);
	b1 = b_read(5);
	printf("%04x = %02x%02x\n", w, b1, b0);
	assert(b0 == 0x0c);
	assert(b1 == 0x0d);
}

void do_halt() 
{
	printf("THE END!!!!\n");
	exit(0);
}

void do_add() 
{
	if(dd.adress < 8)
		reg[dd.adress] = ss.value + dd.value;
	else
		w_write(dd.adress, ss.value + dd.value);
}
void do_mov() 
{
	if((dd.adress < 8) && (dd.adress > 0))
		reg[dd.adress] = ss.value;
	else
		w_write(dd.adress, ss.value);
}

void do_sob() 
{
	reg[nn.value] -= 1;
	if(reg[nn.value] != 0)
	{
		reg[7] -= 2 * nn.adress;
	}
	else
		exit(1);
}

void do_clr()
{
	reg[dd.adress] = 0;
}

void do_unknown() 
{
	exit(1);
}

struct ModReg get_dd(word w) 
{
	int n = (w & 7);	// register number
	int mode = (w >> 3) & 7;	// mode
	struct ModReg res;
	switch(mode) 
	{
		case 0:
				res.adress = n;
				res.value = reg[n];
				printf("r%d ", n);
				break;
		case 1:
				res.adress = reg[n];
				res.value = w_read(res.adress);	// TODO: byte varant
				printf("(r%d) ", n);
				break;
		case 2:
				res.adress = reg[n];
				res.value = w_read(res.adress);	// TODO: byte variant
				reg[n] += 2; 				// TODO: +1 if 
				if (n == 7)
					printf("#%o ", res.value);
				else
					printf("(r%d)+ ", n);
				break;
		default:
				res.adress = n;
				res.value = 0;
				printf("MODE %d NOT IMPLEMENTED YET!\n", mode);
				exit(1);
	}
	return res;
}

struct ModReg get_nn(word w)
{
	int LOOP = (w & 077);
	int n = ((w >> 6) & 07);
	printf("%o %o\n", LOOP, n);
	struct ModReg res;
	res.adress = LOOP;
	res.value = n;
	printf("r%d ", n);
	return res; 
}

void run(adr pc0) {
	pc = pc0;
	int i;
	while(1) 
	{
		word w = w_read(pc);
		printf("%06o:%06o ", pc, w);
		pc += 2;
		for(i = 0; i < 4; ++i)
		{
			struct Command cmd = commands[i];
			if((w & cmd.mask) == cmd.opcode)
			{
				printf("%s ", cmd.name);
				//args
				if(cmd.param & HAS_NN)
				{
					nn = get_nn(w);
				}
				if(cmd.param & HAS_SS)
				{
					ss = get_dd(w >> 6);
				}
				if(cmd.param & HAS_DD)
				{
					dd = get_dd(w);
				}
				cmd.func();
				printf("\n");
				print_registers();
				break;
			}
		}
		printf("\n");
	}
}

void print_registers()
{
	int i;
	for(i = 0; i < 8; i++)
	{
		printf("r%d = %o\n", i, reg[i]);
	}
}
















