#include <stdio.h>
#include <assert.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem[64*1024];

#define LO(x) ((x) & 0xFF)
#define HI(x) (((x)>>8) & 0xFF)

byte b_read  (adr a);            // читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);  // пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);            // читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);  // пишет значение val в "старую память" mem в слово с "адресом" a

byte b_read  (adr a) 
{
	return mem[a];
}
void b_write (adr a, byte val) {
	mem[a] = val;
}
void w_write (adr a, word val) {
	assert(a%2 == 0);
	// val = 0x0b0a
	// a = 2
	// mem[2] = 0x0a
	mem[a] = (byte)(val & 0xFF);    // (byte)val;
	// mem[3] = 0x0b
	mem[a+1] = (byte)((val>>8) & 0xFF);
}
word w_read  (adr a){
	assert(a%2 == 0);
	return ((word)mem[a])|(((word)mem[a + 1])<<8);
}

void test_mem() {
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

void load_file() {
	unsigned int start, n, x, i;
	while (scanf("%x%x", &start, &n) == 2) {
		for(i = 0; i < n; i++) {
			scanf("%x", &x);
			b_write(start, x);
			start++;
		}
	}
}
void mem_dump(adr start, word n) {
	load_file();
	unsigned int i;
	for(i = 0; i < n; i+=2) {
		printf("%06o : %06o\n", start, w_read(start));
		start +=2;
	}
}

int main() {
	mem_dump(0x40, 4);
	return 0;
}
