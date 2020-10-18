#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main()
{
	int nmbr = 0;
	while(1)
	{
		printf("Hello, %d!\n", nmbr++);
		Sleep(1000);
	}
}