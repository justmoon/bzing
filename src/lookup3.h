#include "api/bzing_common.h"

static inline uint32_t
lookup3( const uint32_t *key, uint32_t len, uint32_t seed ) {
	#if defined(_MSC_VER)
	#define rot(x,k) _rotl(x,k)
	#else
	#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
	#endif

	#define mix(a,b,c) \
	{ \
	  a -= c;  a ^= rot(c, 4);  c += b; \
	  b -= a;  b ^= rot(a, 6);  a += c; \
	  c -= b;  c ^= rot(b, 8);  b += a; \
	  a -= c;  a ^= rot(c,16);  c += b; \
	  b -= a;  b ^= rot(a,19);  a += c; \
	  c -= b;  c ^= rot(b, 4);  b += a; \
	}

	#define final(a,b,c) \
	{ \
	  c ^= b; c -= rot(b,14); \
	  a ^= c; a -= rot(c,11); \
	  b ^= a; b -= rot(a,25); \
	  c ^= b; c -= rot(b,16); \
	  a ^= c; a -= rot(c,4);  \
	  b ^= a; b -= rot(a,14); \
	  c ^= b; c -= rot(b,24); \
	}

	uint32_t a, b, c;
	a = b = c = 0xdeadbeef + len + seed;

	const uint32_t *k = (const uint32_t *)key;

	while ( len > 12 ) {
		a += k[0];
		b += k[1];
		c += k[2];
		mix(a,b,c);
		len -= 12;
		k += 3;
	}

	switch( len ) {
		case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
		case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
		case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
		case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
		case 8 : b+=k[1]; a+=k[0]; break;
		case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
		case 6 : b+=k[1]&0xffff; a+=k[0]; break;
		case 5 : b+=k[1]&0xff; a+=k[0]; break;
		case 4 : a+=k[0]; break;
		case 3 : a+=k[0]&0xffffff; break;
		case 2 : a+=k[0]&0xffff; break;
		case 1 : a+=k[0]&0xff; break;
		case 0 : return c;
	}

	final(a,b,c);
	return c;
}


static inline uint32_t
CrapWow( const uint8_t *key, uint32_t len, uint32_t seed ) {
#if !defined(__LP64__) && !defined(_MSC_VER) && ( defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) )
	// esi = k, ebx = h
	uint32_t hash;
	asm(
		"leal 0x5052acdb(%%ecx,%%esi), %%esi\n"
		"movl %%ecx, %%ebx\n"
		"cmpl $8, %%ecx\n"
		"jb DW%=\n"
	"QW%=:\n"
		"movl $0x5052acdb, %%eax\n"
		"mull (%%edi)\n"
		"addl $-8, %%ecx\n"
		"xorl %%eax, %%ebx\n"
		"xorl %%edx, %%esi\n"
		"movl $0x57559429, %%eax\n"
		"mull 4(%%edi)\n"
		"xorl %%eax, %%esi\n"
		"xorl %%edx, %%ebx\n"
		"addl $8, %%edi\n"
		"cmpl $8, %%ecx\n"
		"jae QW%=\n"
	"DW%=:\n"
		"cmpl $4, %%ecx\n"
		"jb B%=\n"
		"movl $0x5052acdb, %%eax\n"
		"mull (%%edi)\n"
		"addl $4, %%edi\n"
		"xorl %%eax, %%ebx\n"
		"addl $-4, %%ecx\n"
		"xorl %%edx, %%esi\n"
	"B%=:\n"
		"testl %%ecx, %%ecx\n"
		"jz F%=\n"
		"shll $3, %%ecx\n"
		"movl $1, %%edx\n"
		"movl $0x57559429, %%eax\n"
		"shll %%cl, %%edx\n"
		"addl $-1, %%edx\n"
		"andl (%%edi), %%edx\n"
		"mull %%edx\n"
		"xorl %%eax, %%esi\n"
		"xorl %%edx, %%ebx\n"
	"F%=:\n"
		"leal 0x5052acdb(%%esi), %%edx\n"
		"xorl %%ebx, %%edx\n"
		"movl $0x5052acdb, %%eax\n"
		"mull %%edx\n"
		"xorl %%ebx, %%eax\n"
		"xorl %%edx, %%esi\n"
		"xorl %%esi, %%eax\n"
		: "=a"(hash), "=c"(len), "=S"(len), "=D"(key)
		: "c"(len), "S"(seed), "D"(key)
		: "%ebx", "%edx", "cc" 
	);
	return hash;
#else
	#define cwfold( a, b, lo, hi ) { p = (uint32_t)(a) * (uint32_t)(b); lo ^= (uint32_t)p; hi ^= (uint32_t)(p >> 32); }
	#define cwmixa( in ) { cwfold( in, m, k, h ); }
	#define cwmixb( in ) { cwfold( in, n, h, k ); }

	const uint32_t m = 0x57559429, n = 0x5052acdb, *key4 = (const uint32_t *)key;
	uint32_t h = len, k = len + seed + n;
	uint64_t p;

	while ( len >= 8 ) { cwmixb(key4[0]) cwmixa(key4[1]) key4 += 2; len -= 8; }
	if ( len >= 4 ) { cwmixb(key4[0]) key4 += 1; len -= 4; }
	if ( len ) { cwmixa( key4[0] & ( ( 1 << ( len * 8 ) ) - 1 ) ) }
	cwmixb( h ^ (k + n) )
	return k ^ h;
#endif
}
