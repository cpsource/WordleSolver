/* wps.c - WordleSolutions */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "version.h"
#include "freq.h"
#include "nytwords.h"
#include "best_guess.h"

static int nyt_word_count = 0;
static unsigned char vowels[256];
static unsigned char *vowel_cnt;

static void init(void) {
  char *c;
  
  if ( !nyt_word_count ) {
    vowels['a'] = 1;
    vowels['e'] = 1;
    vowels['i'] = 1;
    vowels['o'] = 1;
    vowels['u'] = 1;

    while ( get_nytword(nyt_word_count++));

    vowel_cnt = malloc(nyt_word_count+8);
    if ( !vowel_cnt ) {
      printf("malloc failed\n");
      exit(0);
    }
    memset(vowel_cnt,0,nyt_word_count+8);
    
    for ( int i = 0 ; (c=get_nytword(i)) ; i++ ) {
      while ( *c ) {
	if ( vowels[*c] ) {
	  vowel_cnt[i] += 1;
	}
	c += 1;
      } // while
    } // for
  } // if !nyt....
}

// 4 vowels and no repeated letters in nyt list
int best_first_guess(FILE *outf) {
  char *c,*d;
  int i;

  init();

  for ( i = 0 ; (c=get_nytword(i)) ; i++ ) {
    if ( vowel_cnt[i] >= 4 ) {
      d = c;
      while ( d[1] ) {
	// no two letters the same
	if ( strchr(&d[1],*d) ) {
	  goto onward;
	}
	d += 1;
      }
      // good best guess
      fprintf(outf,"Best First Guess: vowel cnt: %0d , word: %s\n", vowel_cnt[i], c);
    }
  onward:;
  } // for

  //exit(0);
  return 0;
}

int best_second_guess(FILE *outf, char *first_guess) {
  int i;
  char *c, *d, *e;

  init();

  //printf("best_second_guess: first_word = %s\n",first_guess);
  
  for ( i = 0 ; (c=get_nytword(i)) ; i++ ) {
    d = c;;
    while ( *d ) {
      if ( strchr(first_guess,*d) ) {
	goto onward;
      }
      d += 1;
    } // while

    //printf("best_second_guess: trial %s\n",c);

    d = c;
    while ( *d ) {
      if ( vowels[*d] ) {

	// no two letters the same
	e = c;
	while ( e[1] ) {
	  // no two letters the same
	  if ( strchr(&e[1],*e) ) {
	    goto onward;
	  }
	  e += 1;
	}

	// a good match
	fprintf(outf,"Best Second Guess: %s\n",c);
	goto onward;
      }
      d += 1;
    } // while
  onward:;
  } // for

  //exit(0);
  
  return 1;
}

