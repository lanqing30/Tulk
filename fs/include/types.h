
#ifndef INCLUDE_TYPES_H_
#define INCLUDE_TYPES_H_

#ifndef NULL
    #define NULL ((void*) 0)
#endif // NULL

#ifndef TRUE
	#define TRUE  1
	#define FALSE 0
#endif



// Represents true-or-false values
typedef _Bool bool;
enum { false, true };

typedef unsigned int   uint32_t;
typedef          int   int32_t;
typedef unsigned short uint16_t;
typedef          short int16_t;
typedef unsigned char  uint8_t;
typedef          char  int8_t;

typedef int32_t pid_t;
//! typedef int32_t size_t; // TODO: conflict bug fix
typedef int32_t physaddr_t;
typedef uint32_t pde_t;
typedef uint32_t pte_t;
typedef uint16_t envid_t;


enum {
	SYS_cputc = 1,
	SYS_cgetc,
	SYS_getenvid,
	SYS_destroy,
	SYS_malloc,
	SYS_free,
	SYS_clear_buffer,
	NSYSCALLS
};


// Efficient min and max operations
#define MIN(_a, _b)						    \
({								            \
	typeof(_a) __a = (_a);					\
	typeof(_b) __b = (_b);					\
	__a <= __b ? __a : __b;					\
})
#define MAX(_a, _b)						    \
({								            \
	typeof(_a) __a = (_a);					\
	typeof(_b) __b = (_b);					\
	__a >= __b ? __a : __b;					\
})

// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)						\
({								            \
	uint32_t __a = (uint32_t) (a);				\
	(typeof(a)) (__a - __a % (n));				\
})
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)						    \
({								                \
	uint32_t __n = (uint32_t) (n);				\
	(typeof(a)) (ROUNDDOWN((uint32_t) (a) + __n - 1, __n));	\
})


#define MODULE(x) __attribute__((section("." x ".text")))

#endif 	// INCLUDE_TYPES_H_
