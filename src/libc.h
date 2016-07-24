#pragma once
#include "va_list.h"
#include "stdint.h"
#include "limits.h"

#define STRSTR(str) #str
#define STR(str) STRSTR(str)

#define FCAST(rettype, ...) (rettype (*)(__VA_ARGS__))
#define FCASTF(func, rettype, ...) ((rettype (*)(__VA_ARGS__))func)
#define FDECL(_typename, rettype, ...) typedef rettype (*_typename)(__VA_ARGS__)
#define FDECLV(ptrname, _typename, rettype, ...) FDECL(_typename, rettype, __VA_ARGS__); static _typename ptrname;

static inline int strcmp(const char * l, const char * r) {
	for (; *l == *r && *l; l++, r++);
	return *(unsigned char *)l - *(unsigned char *)r;
}

#ifndef MODULES

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

#define BITOP(A, B, OP) \
 ((A)[(size_t)(B)/(8*sizeof *(A))] OP (size_t)1<<((size_t)(B)%(8*sizeof *(A))))

#define strfmt(buffer, fmt) do { va_list args; va_start(args, fmt); vasprintf(buffer, fmt, args); va_end(args); } while(0)

int sprintf(char * buf, const char *fmt, ...);
size_t vasprintf(char * buf, const char * fmt, va_list args);

extern "C" { void * memset(void * dest, int c, size_t n); }
unsigned short * memsetw(unsigned short * dest, unsigned short val, int count);
void * memcpy(void * __restrict__  dest, const void * __restrict__ src, size_t n);
int memcmp(const void * vl, const void * vr, size_t n);
void * memchr(const void * src, int c, size_t n);
void * memrchr(const void * m, int c, size_t n);
void * memmove(void * dest, const void * src, size_t n);

char * strcat(char * dst, const char * src);
size_t strlen(const char * s);
char * strdup(const char * s);
char * stpcpy(char * __restrict__ d, const char * __restrict__ s);
char * strcpy(char * __restrict__ dest, const char * __restrict__ src);
char * strncpy(char * dest, const char * src, size_t n);
size_t strspn(const char * s, const char * c);
char * strchrnul(const char * s, int c);
char * strchr(const char * s, int c);
char * strrchr(const char * s, int c);
size_t strcspn(const char * s, const char * c);
char * strpbrk(const char * s, const char * b);
static char *strstr_2b(const unsigned char * h, const unsigned char * n);
static char *strstr_3b(const unsigned char * h, const unsigned char * n);
static char *strstr_4b(const unsigned char * h, const unsigned char * n);
static char *strstr_twoway(const unsigned char * h, const unsigned char * n);
char *strstr(const char * h, const char * n);
int isdigit(int ch);
int isxdigit (int ch);
int isspace(int ch);
int atoi(const char * s);
size_t lfind(const char * str, const char accept);
size_t rfind(const char * str, const char accept);
uint8_t startswith(const char * str, const char * accept);
char * strtok_r(char * str, const char * delim, char ** saveptr);
uint32_t __attribute__((pure)) krand(void);
int tokenize(char * str, char * sep, char **buf);
int tolower(int c);
int toupper(int c);

#endif
