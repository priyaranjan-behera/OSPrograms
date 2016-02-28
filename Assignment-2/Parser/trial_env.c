#include <stdio.h>
#include <stdlib.h>

void main()
{
	char *str = "ABC=xyz";
	printf("Puting Env Variable: %s", str);

	putenv(str);

	char *quer = "ABC";
	printf("Values is: %s", getenv(quer));
	
}