/* sbs.c - SpellingBeeSolutions */
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

// -a flag - use aspell
int use_a = 0;

unsigned char work_buffer[256];
unsigned char ok_buf[256];

/*
 * Keep track of our play board
 */
#define LB_FLAG_NONE 0
#define LB_FLAG_GREY 1
#define LB_FLAG_YELLOW 2
#define LB_FLAG_GREEN 3

typedef struct letter_bit {
  char letter;
  int flag;
} LB;
#define LB_COLS 5
#define LB_ROWS 6
int current_play_row 0 // the row to guess
LB[LB_COLS][LB_ROWS];

void skip_space_tab(char **c) {
  char *d = *c;
  while ( *d != 0 && (*d == ' ' || *d == '\t') ) {
    d += 1;
  }
  *c = d;
}

void skip_to_tab(char **c) {
  char *d = *c;
  while ( *d != 0 && !isspace(*d) ) {
    d += 1;
  }
  *c = d;
}

/*
 * str ==> letter , color ...
 */
void fill_in_board( char *str )
{
  int col = 0;
  char *c = str;
  char letter;
  int flag;
  
  while ( col < 5 ) {

    skip_space_tab(&c);
    letter = *c;
    c += 1;
    skip_space_tab(&c);
    switch ( *c ) {
    case 'g':
      flag = LB_FLAG_GREY;
      break;
    case 'y':
      flag = LB_FLAG_YELLOW;
      break;
    case 'r':
      flag = LB_FLAG_GREEN;
      break;
    default:
      printf("Unknown color %s\n"c);
      exit(0);
      break;
    }
    skip_to_space_tab(&c);

    LB[col][current_play_row].letter = letter;
    LB[col][current_play_row].flag = flag;
    
    col += 1;
  } // while

  current_play_row += 1;
}

#define FLAGS_none    0
#define FLAGS_pangram 1
#define FLAGS_bad     2
#define FLAGS_duplicate 4
#define FLAGS_printed 8

#define WORD_MAX 32
typedef struct ans_txt {
  struct ans_txt *next;
  int flags;
  char word[WORD_MAX];
} ANS_TXT;

ANS_TXT *root_ans_txt = NULL;

#define ANS_TXT_STR "ans.txt"
#define BADS_TXT_STR "bads.txt"

int fd = 0;
char *bad_buf = NULL;

// return true if word exists from search_point on
void flag_all_duplicates ( ANS_TXT *search_point, char *word )
{
  ANS_TXT *next = search_point;
  
  while ( next ) {
    if ( 0 == strcmp(next->word,word) ) {
      next->flags |= FLAGS_duplicate;
    }
    next = next->next;
  }
  return;
}
void walk_for_duplicates ( void )
{
  ANS_TXT *next = root_ans_txt;

  while ( next ) {
    flag_all_duplicates(next->next,next->word);
    next = next->next;
  }
  return;
}

void open_fd_ro ( void )
{
  struct stat sb;
  int sts;

  fd = open ( BADS_TXT_STR, O_RDONLY );
  if ( !fd ) {
    fprintf(stderr,"%d: can't open %s\n", __LINE__,BADS_TXT_STR);
    exit(0);
  }
  sts = fstat ( fd, &sb );
  if ( sts ) {
    fprintf(stderr,"%d: fstat failed for %s\n",__LINE__,BADS_TXT_STR);
    exit(0);
  }
  
  bad_buf = malloc( sb.st_size + 1 );
  if ( !bad_buf ) {
    fprintf(stderr,"%d: can't malloc bad_buf\n",__LINE__);
    exit(0);
  }
  
  sts = read ( fd, bad_buf, sb.st_size );
  if ( sts != sb.st_size ) {
    fprintf(stderr,"%d: read failed for %s\n",__LINE__,BADS_TXT_STR);
    exit(0);
  }

  // don't forget last byte must be 0
  bad_buf [ sb.st_size - 1 ] = 0;

  // done
  close(fd);
}

// search root_ans_txt for bad_word and mark it FLAGS_bad
void mark_bads(char *bad_word )
{
  ANS_TXT *next = root_ans_txt;

  while ( next ) {

    if ( 0 == strcmp(next->word,bad_word )) {
      next->flags |= FLAGS_bad;
    }
    //onward:
    next = next->next;
  }
}

// walk *'s in bad_buf and mark them as bad
void walk_bad_buf (void)
{
  char *c, *tgt;
  char my_bad_word[WORD_MAX];

  c = strchr(bad_buf,'*');
  while ( c ) {
    // get a bad word
    tgt = my_bad_word;
    c += 1; // skip leading *
    while ( *c != '*' ) {
      *tgt++ = *c++;
    }
    *tgt = 0;

    // housekeeping
    c += 1; // skip trailing star
    c = strchr(c,'*');

    // mark my_bad_word as bad
    mark_bads(my_bad_word);
  }
}

// save a possible word
void save_word ( char *word, int flags )
{
  ANS_TXT *new;

  new = malloc(sizeof(ANS_TXT));
  if ( !new ) {
    fprintf(stderr,"%d: malloc failed\n",__LINE__);
    exit(0);
  }

  new->flags = flags;
  strcpy(new->word,word);
  new->next = root_ans_txt;
  root_ans_txt = new;
}

#if defined(USE_DEBUG)
void dump_list(void)
{
  ANS_TXT *next = root_ans_txt;

  while ( next ) {
    printf("%s : %08x\n",
	   next->word, next->flags);
    next = next->next;
  }
}
#endif

// return true if letters are valid
int letters_valid ( char *buf, int cl )
{
  char *c;
  
  /* can only use these letters */
  c = (char *)buf;
  while ( *c != 0 ) {
    if ( !ok_buf[(int)*c] ) return 0;
    c += 1;
  }
  if ( !strchr(buf,cl) ) return 0;
  return 1;
}
    
int main(int argc, char *argv[])
{
  FILE *inf, *outf;
  char *c;
  int use_popular = 0;
  int l_index = 1;
  int use_ita = 0;
  char filename[64];
  char system_cmd[64];
  ANS_TXT *tmp;
  int approved_words = 0;
  int other_words = 0;
  
  printf("%s.%s\n",VERSION_STR,GIT_VERSION);
  printf("Build Date: %s\n",BUILD_DATE);
  
  if ( argc < 1 || argc > 2 ) {
    printf("usage: ./wos [-a]\n");
    exit(0);
  }
  if ( argv[l_index][0] == '-' && argv[l_index][1] == 'a' ) {
    l_index += 1;
    use_a = 1;
  }
  
  /* allow only these letters */
  c = &argv[l_index][0];
  while ( *c != 0 ) {
    ok_buf[(int)*c] = 1;
    c += 1;
  }
  
  /* get right dictionary */
  strcpy(filename,"wos_words.txt");
  inf = fopen(filename, "r");
  if ( !inf ) {
    printf("%s not found\n",filename);
    exit(0);
  }
  
  // print some stats
  printf("Letters: %s\n",argv[l_index]);
  printf("Open: %s\n",filename);
  
  // scan entire word list *.txt
  while ( fgets((char * restrict)work_buffer,sizeof(work_buffer), inf) ) {
    int len;
    
    // get rid of obviously invalid records
    if ( strlen((const char *)work_buffer) == 0 ) continue;
    
    // get rid of new-line
    c = strchr((const char *)work_buffer,'\n'); if ( c ) *c = 0;
    // c = strchr((const char *)work_buffer,'\m'); if ( c ) *c = 0;
    c = strchr((const char *)work_buffer,'\n'); if ( c ) *c = 0;
    c = strchr((const char *)work_buffer,' '); if ( c ) *c = 0;
    
    // save length of string
    len = strlen((const char *)work_buffer);
    
    // reject any words not 5 in length
    if ( len != 5 ) continue;
#if 1
    c = (char *)work_buffer;
    while ( *c != 0 ) {
      *c = tolower(*c) & 127;
      c += 1;
    }
#endif // 1

    /* can't contain grey letters */
    int tcol = 0;
    int trow = 0;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	c = (char *)work_buffer;
	while ( *c != 0 ) {
	  if ( *c == LB[tcol][trow].letter && LB[tcol][trow].flag == LB_FLAG_GREY ) {
	    goto is_not_ok;
	  }
	  c += 1;
	} // while c
      } // for
      trow += 1;
    } // while
    
    /* red letters must be in a particular word column */
    int tcol = 0;
    int trow = 0;
    c = (char *)work_buffer;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( LB[tcol][trow].flag == LB_FLAG_GREEN &&
	     c[tcol] != LB[tcol][trow].letter ) {
	  goto is_not_ok;
	}
      }
      trow += 1;
    }

    /* word must contain yellow letters */
    int tcol = 0;
    int trow = 0;
    c = (char *)work_buffer;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( LB[tcol][trow].flag == LB_FLAG_YELOW ) {
	  if ( NULL == strchr(c,LB[tcol][trow].letter) ) {
	    goto is_not_ok;
	  }
	}
      }
      trow += 1;
    }

    /* word can't have yellow in a column */
    int tcol = 0;
    int trow = 0;
    c = (char *)work_buffer;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( LB[tcol][trow].flag == LB_FLAG_YELOW ) {
	  if ( c[tcol] == LB[tcol][trow].letter ) {
	    goto is_not_ok;
	  }
	}
      }
      trow += 1;
    }

    // finally, add the word
    save_word((char *)work_buffer, 0, (char *)work_buffer);

  is_not_ok:;
  } // while fgets
  
  fclose(inf);

  //
  // so we have a possible list stored at root_ans_list
  //
  
  // generate a prototype ans.txt
  unlink(ANS_TXT_STR);
  outf = fopen(ANS_TXT_STR,"w");
  if ( !outf ) {
    fprintf(stderr, "%d: %s can't be created\n",__LINE__,ANS_TXT_STR);
    exit(0);
  }
  tmp = root_ans_txt;
  while ( tmp ) {
    fprintf(outf,"%s\n",tmp->word);
    tmp = tmp->next;
  }
  fclose(outf);

  // lets run aspell against it
#define SYSTEM_CALL "aspell -c %s > %s"
  
  if ( use_a ) {
    sprintf(system_cmd,SYSTEM_CALL, ANS_TXT_STR, BADS_TXT_STR);
  } else {
    sprintf(system_cmd,"rm -f %s; touch %s", BADS_TXT_STR, BADS_TXT_STR);
  }

  //printf("%d: system command <%s>\n",__LINE__,system_cmd);
  
  if ( system(system_cmd) ) {
    fprintf(stderr,"%d: system call %s failed\n",__LINE__,system_cmd);
    exit(0);
  }

  
  // lets load bads.txt into memory
  open_fd_ro ( );
  
  // mark any bads
  walk_bad_buf();
  
  // mark duplicates
  walk_for_duplicates();
  
  // dump list root_ans_txt if not bad
  // generate corrected ans.txt
  unlink(ANS_TXT_STR);
  outf = fopen(ANS_TXT_STR,"w");
  if ( !outf ) {
    printf("%d: %s can't be created\n",__LINE__,ANS_TXT_STR);
    exit(0);
  }
  fprintf(outf,"sbs %s.%s\n",VERSION_STR,GIT_VERSION);
  fprintf(outf,"Build Date: %s\n",BUILD_DATE);
  fprintf(outf,"Letters: %s\n",argv[l_index]);
  fprintf(outf,"Open: %s\n",filename);
  tmp = root_ans_txt;
  while ( tmp ) {
    if ( !(tmp->flags & FLAGS_duplicate) )  {
      if ( !(tmp->flags & FLAGS_bad) )  {
	approved_words += 1;
	if ( tmp->flags & FLAGS_pangram )  {
	  fprintf(outf,"* %s\n",tmp->word);
	  fprintf(stdout,"* %s\n",tmp->word);
	} else {
	  fprintf(outf,"%s\n",tmp->word);
	}
	tmp->flags |= FLAGS_printed;
      }
    }
    // onward
    tmp = tmp->next;
  }

  // dump stats
  fprintf(stdout,"Approved Words: %d\n",approved_words);

#if 0  
  // now lets dump the entire list just in case
  if ( use_a ) {
    fprintf(outf,"\n");
    fprintf(outf,"Entire Remaining List:\n");
    fprintf(outf,"\n");
    tmp = root_ans_txt;
    while ( tmp ) {
      if ( !(tmp->flags & FLAGS_duplicate) )  {
	if ( tmp->flags & FLAGS_bad )  {
	  if ( !(tmp->flags & FLAGS_printed) )  {
	    other_words += 1;
	    if ( tmp->flags & FLAGS_pangram )  {
	      fprintf(outf,"* %s\n",tmp->word);
	      fprintf(stdout,"* %s\n",tmp->word);
	    } else {
	      fprintf(outf,"%s\n",tmp->word);
	    }
	    tmp->flags |= FLAGS_printed;
	  } // FLAGS_printed
	} // FLAGS_bad
      } // FLAGS_duplicate
      // onward
      tmp = tmp->next;
    } // while
  } // use_a
#endif // 0
  
  // dump stats
  fprintf(stdout,"Other Words: %d\n",other_words);
  
  // close output file
  fclose(outf);

#if defined(USE_DEBUG)
  dump_list();
#endif
  
  // done. Note - let exit do all memory cleanup
  return 0;
}
