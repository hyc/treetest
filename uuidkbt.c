#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <uuid/uuid.h>

#include "kbtree.h"

typedef struct berval {
	size_t bv_len;
	char *bv_val;
} BerValue;

#define UUIDLEN	16

typedef struct kuuid {
	char uuid[16];
} kuuid;

static int
syncuuid_cmp( const kuuid u1, const kuuid u2 )
{
	return memcmp( u1.uuid, u2.uuid, UUIDLEN );
}

KBTREE_INIT(pres, kuuid, syncuuid_cmp)

typedef struct syncinfo_s {
	kbtree_t(pres) *si_presentlist;
} syncinfo_t;

#ifndef BTSIZE
#define BTSIZE	KB_DEFAULT_SIZE
#endif

static int
presentlist_insert( syncinfo_t *si, struct berval *syncUUID )
{
	if (!si->si_presentlist)
		si->si_presentlist = kb_init(pres, BTSIZE);

	kb_putp(pres, si->si_presentlist, (const kuuid *)syncUUID->bv_val);
	return 1;
}

#define SIZE	2500000

uuid_t uus[SIZE];

int main() {
	syncinfo_t si = {};
	int rc, i;
	struct berval bv;
	struct timeval beg, end;

	read(0, uus, sizeof(uus));

	bv.bv_len = sizeof(uuid_t);

	for (i=0; i<SIZE; i++) {
		bv.bv_val = (char *)&uus[i];
		presentlist_insert( &si, &bv );
	}
}
