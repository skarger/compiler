#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
	char *str;
	char *str2;

	str = malloc(4);
	strcpy(str, "abc");
	printf("malloc_str: returning 0x%8x -> \"%s\"\n", str, str);

	str2 = malloc(4);
	strcpy(str2, "def");
	printf("malloc_str: returning 0x%8x -> \"%s\"\n", str2, str2);

	return 0;
}


