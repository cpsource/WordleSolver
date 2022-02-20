/* build sbs_words.txt from words.txt */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"

unsigned char work_buffer[256];
unsigned char ok_buf[256];
unsigned char symbols[256];

char uniq[256];
int is_uniq(char *str)
{
  int u = 0;
  char *c;
  int i;

  c = str;
  memset(uniq,0,256);
  while (*c) {
    uniq[*c] = 1;
    c += 1;
  }

  for ( i = u = 0; i < 256 ; i++ ) {
    if ( uniq[i] ) u += 1;
  }

  return u;
}

int main1(int argc, char *argv[])
{
  FILE *inf, *outf;
  char *c;
  int ok_flag;
  int symbol_count;
  int i;
  char *work_buffer_ptr;
  
  for ( i = 'a' ; i <= 'z' ; i++ ) {
    ok_buf[i] = 1;
  }

  inf = fopen("sbs_words.txt", "r");
  if ( !inf ) { printf("sbs_words.txt not found\n"); exit(0); }
  outf = fopen("wos_words.txt","w+");
  if ( !outf) { printf("could not create wos_words.txt\n"); exit(0); }

  while ( fgets(work_buffer,sizeof(work_buffer), inf) ) {
    int len;

    work_buffer_ptr = work_buffer;
    
    if ( strlen(work_buffer) == 0 ) continue;
    c = strchr(work_buffer,'\n'); if ( c ) *c = 0;

    len = strlen(work_buffer_ptr);
    if ( len != 5 ) continue;

    /* make all lower case */
    c = work_buffer_ptr;
    while ( *c != 0 ) {
      *c = tolower(*c);
      c += 1;
    }

    /* only take these symbols */
    c = work_buffer_ptr;
    while ( *c != 0 ) {
      if ( !ok_buf[*c] ) goto next;
      c += 1;
    }

    fprintf(outf,"%s\n",work_buffer_ptr);

  next:;
  } /* while inf */
  fclose(inf); fclose(outf);
  return 0;
}

int main(int argc, char *argv[] )
{
  main1(argc,argv);
  return 0;
}
