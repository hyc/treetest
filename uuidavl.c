#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <uuid/uuid.h>

#include "avl.h"

typedef struct berval {
	size_t bv_len;
	char *bv_val;
} BerValue;

typedef struct syncinfo_s {
	Avlnode *si_presentlist;
} syncinfo_t;

#define UUIDLEN	16

#define HASHUUID

static int
syncuuid_cmp( const void* v_uuid1, const void* v_uuid2 )
{
#ifdef HASHUUID
	return ( memcmp( v_uuid1, v_uuid2, UUIDLEN-2 ));
#else
	return ( memcmp( v_uuid1, v_uuid2, UUIDLEN ));
#endif
}

static int
presentlist_insert( syncinfo_t *si, struct berval *syncUUID )
{
	char *val;
#ifdef HASHUUID
	Avlnode **av;
	unsigned short s;

	if (!si->si_presentlist)
		si->si_presentlist = calloc(65536, sizeof(Avlnode *));

	av = (Avlnode **)si->si_presentlist;
	val = malloc(UUIDLEN-2);
	memcpy(&s, syncUUID->bv_val, 2);
	memcpy(val, syncUUID->bv_val+2, UUIDLEN-2);

	if (avl_insert( &av[s], val,
		syncuuid_cmp, avl_dup_error))
	{
		free(val);
		return 0;
	}
#else
	val = malloc(UUIDLEN);
	memcpy( val, syncUUID->bv_val, UUIDLEN );
	if ( avl_insert( &si->si_presentlist, val,
		syncuuid_cmp, avl_dup_error ))
	{
		free( val );
		return 0;
	}
#endif
	return 1;
}

#define SIZE	2500000

uuid_t uus[SIZE];

int main() {
	syncinfo_t si = {};
	int rc, i;
	struct berval bv;
	uuid_t uu;

	read(0, uus, sizeof(uus));

	bv.bv_len = sizeof(uuid_t);
	for (i=0; i<SIZE; i++) {
		bv.bv_val = (char *)&uus[i];
		presentlist_insert( &si, &bv );
	}
}
