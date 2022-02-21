// Note: taken from https://www3.nd.edu/~busiforc/handouts/cryptography/letterfrequencies.html

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "freq.h"

// weight table
int weights[26];

void init_weight( char letter, int weight) {
  weights[tolower(letter)-'a'] = weight;
}

void init_all_weights(void) {
  init_weight('E', 568);
  init_weight('A', 433);
  init_weight('R', 386);
  init_weight('I', 384);
  init_weight('O', 365);
  init_weight('T', 354);
  init_weight('N', 339);
  init_weight('S', 292);
  init_weight('L', 279);
  init_weight('C', 231);
  init_weight('U', 185);
  init_weight('D', 172);
  init_weight('P', 161);
  init_weight('M', 153);
  init_weight('H', 153);
  init_weight('G', 125);
  init_weight('B', 105);
  init_weight('F', 92);
  init_weight('Y', 90);
  init_weight('W', 65);
  init_weight('K', 56);
  init_weight('V', 51);
  init_weight('X', 14);
  init_weight('Z', 13);
  init_weight('J', 10);
  init_weight('Q', 10);
}

// calculate weights
int calc_weight ( char *str ) {
  int weight = 0;
  char *c = str;
  int idx;
  static int init_flag = 0;

  if ( !init_flag ) {
    init_all_weights();
    init_flag = 1;
  }
  
  while ( *c ) {
    idx = tolower(*c) - 'a';
    if ( idx < 0 || idx > 25 ) {
      printf("calc_weight: index out of range for letter <%c>\n",*c);
      exit(0);
    }
    weight += weights[idx];
    c += 1;
  }

  return weight;
}
