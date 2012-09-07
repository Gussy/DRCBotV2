#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "../src/drc.h"

#define CSI "\x1B["
#define RESET CSI "0m"
#define GREEN CSI "92m"
#define YELLOW CSI "93m"
#define BLUE  CSI "94m"
#define RED   CSI "91m"
#define WHITE CSI "97m"

int main(int argc, char** argv)
{
    printf(BLUE "Hello World" RESET "\n");
}

// HACK!
jmp_buf env;

bool mstats[40];
char * messages[40];
int mint = 0;

char * testname;
bool test_ok = true;
bool __test_assert(bool v, char * str)
{
	test_ok = test_ok && v;
	if (!v)
	{
		mstats[mint] = v;
		messages[mint] = str;
		mint++;
	}
	return v;
}