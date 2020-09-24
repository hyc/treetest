#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "kbtree.h"

typedef struct berval {
	size_t bv_len;
	char *bv_val;
} BerValue;

#define ber_bvcmp(v1,v2) \
    ((v1)->bv_len < (v2)->bv_len \
        ? -1 : ((v1)->bv_len > (v2)->bv_len \
            ? 1 : memcmp((v1)->bv_val, (v2)->bv_val, (v1)->bv_len) ))

#define ber_bvchr(bv,c) \
    ((char *) memchr( (bv)->bv_val, (c), (bv)->bv_len ))

/* Session log data */
typedef struct slog_entry {
    struct berval se_uuid;
    struct berval se_csn;
    int se_sid;
    int   se_tag;
} slog_entry;

static int
sessionlog_cmp( const void *l, const void *r )
{
	const slog_entry *left = l, *right = r;
	int ret = ber_bvcmp( &left->se_csn, &right->se_csn );
	if ( !ret )
		ret = ber_bvcmp( &left->se_uuid, &right->se_uuid );
	return ret;
}

#define SLAP_SYNC_SID_MAX	4095

int
slap_parse_csn_sid( struct berval *csnp )
{
	char *p, *q;
	struct berval csn = *csnp;
	int i;

	p = ber_bvchr( &csn, '#' );
	if ( !p )
		return -1;
	p++;
	csn.bv_len -= p - csn.bv_val;
	csn.bv_val = p;

	p = ber_bvchr( &csn, '#' );
	if ( !p )
		return -1;
	p++;
	csn.bv_len -= p - csn.bv_val;
	csn.bv_val = p;

	q = ber_bvchr( &csn, '#' );
	if ( !q )
		return -1;

	csn.bv_len = q - p;

	i = strtol( p, &q, 16 );
	if ( p == q || q != p + csn.bv_len || i < 0 || i > SLAP_SYNC_SID_MAX ) {
		i = -1;
	}

	return i;
}

KBTREE_INIT(slog, slog_entry*, sessionlog_cmp)

#ifndef BTSIZE
#define	BTSIZE	KB_DEFAULT_SIZE
#endif

int main() {
	char inbuf[80];
	kbtree_t(slog) *b;
	int rc, num=0, size=1000000;
	slog_entry *se;
	struct berval bv;

	bv.bv_val = inbuf;
	b = kb_init(slog, BTSIZE);

	/* test inserts, expiration */
	while (fgets(inbuf, sizeof(inbuf), stdin)) {
		bv.bv_len = strlen(inbuf);
		se = malloc( sizeof(slog_entry) + bv.bv_len );
		se->se_tag = 1;
		se->se_uuid.bv_val = (char *)((&se[1]));
		se->se_uuid.bv_len = 0;
		se->se_csn.bv_val = se->se_uuid.bv_val + 0;
		memcpy( se->se_csn.bv_val, bv.bv_val, bv.bv_len);
		se->se_csn.bv_len = bv.bv_len-1;
		se->se_csn.bv_val[se->se_csn.bv_len] = '\0';
		se->se_sid = slap_parse_csn_sid( &se->se_csn );
		kb_putp(slog, b, (const slog_entry **)&se);
		assert(1);
		num++;
		if (num > size) {
			kbitr_t itr;
			kb_itr_first(slog, b, &itr);
			while (num > size) {
				int i;
				se = kb_itr_key(slog_entry*, &itr);
				kb_itr_next(slog, b, &itr);
				kb_delp(slog, b, (const slog_entry **)&se);
				free(se);
				num--;
			}
		}
	}

#if 0
	/* test approximate search */
	{
		char *vals[] = {
			"20200924041137.412289Z#000000#004#000000",
			"20200924041137.415761Z#000000#004#000000",
			"20200924041137.415761Z#000001#004#000000",
			"20200924041137.417213Z#000000#004#000000",
			NULL };
		slog_entry **lo, **hi, te = {0};
		kbitr_t itr;
		int i;

		te.se_csn.bv_val = vals[2];
		te.se_csn.bv_len = strlen(vals[2]);

		kb_itr_first(slog, b, &itr );
		se = kb_itr_key(slog_entry *, &itr);

		se = &te;
		kb_itr_get(slog, b, (const slog_entry **)&se, &itr );
		i = 0;
		do {
			kb_itr_next(slog, b, &itr);
			se = kb_itr_key(slog_entry *, &itr);
			i++;
		} while( strcmp( se->se_csn.bv_val, vals[1] ));

		kb_interval(slog, b, &te, &lo, &hi );
		if ( lo )
			kb_itr_get(slog, b, (const slog_entry **)lo, &itr );
		else
			kb_itr_first(slog, b, &itr );
		se = kb_itr_key(slog_entry *, &itr);
		kb_itr_next(slog, b, &itr);
		se = kb_itr_key(slog_entry*, &itr);
	}
#endif
}
