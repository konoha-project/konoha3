#ifndef KONOHA_LKM_H_
#define KONOHA_LKM_H_

#ifndef __KERNEL__
#error
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/syslog.h>
#include <asm/uaccess.h>

#define KNH_EXT_QSORT  1
#define KNH_EXT_SETJMP 1

/* stdint.h */
#ifndef _STDINT_H
#define _STDINT_H
typedef long intptr_t;
#endif

/* /usr/include/inttypes.h */
#define PRIdPTR "d"
#define PRIuPTR "u"

typedef intptr_t FILE;

#define TODO_LKM
//#define getenv_i(a) NULL
#define stdin  ((FILE*)NULL)
#define stdout KERN_INFO
#define stderr KERN_ALERT
#define EXIT_FAILURE   1

#define calloc(x,y) kcalloc(x,y,GFP_KERNEL)
#define realloc(x,y) krealloc(x,y,GFP_KERNEL)

static inline void *malloc(size_t size)
{
	return kmalloc(size,GFP_KERNEL);
}
static inline void free(void *p)
{
	kfree(p);
}

#define isrange(c,a,z) (c >= a && z >= c)

static inline long long int _strtoll(const char *nptr, char **endptr, int base)
{
	if(*nptr == '0') {
		nptr++;
		if(*nptr == 'x' || *nptr == 'X') {
			base = 16;
			nptr++;
		} else {
			base = 8;
		}
	} else if(*nptr == '+'){
		nptr++;
	}
	if(base == 0)
		base = 10;

	long long int tmp = 0;
	char c = 0;
	for(tmp = 0;(c =  *nptr) != '\0';nptr++) {
		c = isrange(c,'0','9') ? c - '0':
			(isrange(c,'A','Z') ? c - 'A' + 10 :
			 (isrange(c,'a','z') ? c - 'a' + 10:-1));
		if(c == -1)
			break;
		tmp = tmp * base + c;
	}
	if(endptr != NULL) {
		*endptr = (char *)nptr;
	}
	return tmp;
}

static inline long long int strtoll(const char *nptr, char **endptr, int base)
{
	while(*nptr == ' ')
		nptr++;

	if(*nptr == '-'){
		return -1 * _strtoll(nptr+1,endptr,base);
	}
	return _strtoll(nptr,endptr,base);
}

#define bzero(x,y) memset(x,0x00,y)

static inline void *fopen(const char *a,const char *b)
{
	(void)a;(void)b;
	return NULL;
}

static inline int fclose(void *fp)
{
	return 0;
}

static inline int feof(void *fp)
{
	return -1;
}

#define dlopen(a,b) NULL
#define dlsym(a,b) NULL

static inline const char *shortFilePath(const char *path)
{
	(void)path;
	return "kernel";
}

#define LOG_INFO 0
static inline void syslog(int i,const char *msg, ...)
{
	(void)i;(void)msg;
}

static inline void kdebugPrintf(const char *file, const char *func, int line, const char *fmt, ...)
{
	(void)file;(void)func;
}

#define fprintf(out,fmt, arg...) printk(KERN_ALERT fmt , ##arg )
#define fputs(prompt, fp) 

static inline int fgetc(void *fp)
{
	return -1;
}

#define EOF -1
#define fflush(x)
static inline void kexit(int i)
{
	printk(KERN_EMERG "KONOHA_exit!!!");
}
#define assert(x) BUG_ON(!(x))
#define abort() BUG_ON(1)

/* setjmp.S */
#if defined(__i386__)
/* return_addr, ebx, esp, ebp, esi, edi */
#define JMP_BUFFSIZE 6

#elif defined(__x86_64__)
/* return_addr, rbx, rsp, rbp, r12, r13, r14, r15 */
#define JMP_BUFFSIZE 8
#endif

typedef struct {
	unsigned long __jmp_buf[JMP_BUFFSIZE];
} jmp_buf;
#define jmpbuf_i jmp_buf

static inline int setjmp(jmp_buf env)
{
	(void)env;
	return 0;
}
static inline void longjmp(jmp_buf env, int val)
{
	(void)env;(void)val;
}

/* ../../src/ext/qsort.c */
void qsort (void *const pbase, size_t total_elems, size_t size,
				int (*cmp)(const void*,const void*));

/* ------------------------------------------------------------------------ */

#endif /* KONOHA_LKM_H_ */
