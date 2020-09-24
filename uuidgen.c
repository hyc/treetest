
#include <stdio.h>
#include <unistd.h>
#include <uuid/uuid.h>

#define SIZE	2500000

uuid_t uus[SIZE];

int main() {
	int i;

	fprintf(stderr, "generating %d random UUIDs...\n", SIZE);

	for (i=0; i<SIZE; i++)
		uuid_generate_random((unsigned char *)&uus[i]);

	write(1, uus, sizeof(uus));
}
