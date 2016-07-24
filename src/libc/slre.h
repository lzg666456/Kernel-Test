/*
 * Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (c) 2013 Cesanta Software Limited
 * All rights reserved
 *
 * This library is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this library under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this library under a commercial
 * license, as set out in <http://cesanta.com/products.html>.
 */

/*
 * This is a regular expression library that implements a subset of Perl RE.
 * Please refer to README.md for a detailed reference.
 */

#ifndef SRC_LIBC_SLRE_H_
#define SRC_LIBC_SLRE_H_

#include <libc/list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct slre_cap {
  char *ptr;
  int len;
};

int slre_match(char *regexp, char *buf, int buf_len,
               struct slre_cap *caps, int num_caps, int flags);

char *slre_replace(char *regex, char *buf, char *sub);

/* Possible flags for slre_match() */
enum { SLRE_IGNORE_CASE = 1 };

/* slre_match() failure codes */
#define SLRE_NO_MATCH               -1
#define SLRE_UNEXPECTED_QUANTIFIER  -2
#define SLRE_UNBALANCED_BRACKETS    -3
#define SLRE_INTERNAL_ERROR         -4
#define SLRE_INVALID_CHARACTER_SET  -5
#define SLRE_INVALID_METACHARACTER  -6
#define SLRE_CAPS_ARRAY_TOO_SMALL   -7
#define SLRE_TOO_MANY_BRANCHES      -8
#define SLRE_TOO_MANY_BRACKETS      -9

/* Library added */
typedef struct {
	list_t * matches;
	list_t * matches_whole;
	int matchcount;
	int wordcount;
} regex_t;

extern regex_t rexmatch(char * patt, char * text);
extern void free_regex(regex_t reg);

#ifdef __cplusplus
}
#endif

#endif /* SRC_LIBC_SLRE_H_ */