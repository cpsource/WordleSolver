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
#include "freq.h"
#include "nytwords.h"

// -a flag - use aspell
int use_a = 0;

unsigned char work_buffer[256];
unsigned char ok_buf[256];
unsigned char yellow_list[256];

/*
 * Keep track of our play board
 */
#define LB_FLAG_NONE   0
#define LB_FLAG_GREY   1
#define LB_FLAG_YELLOW 2
#define LB_FLAG_GREEN  3

typedef struct letter_bit {
  char letter;
  int flag;
} LB;

#define LB_COLS 5
#define LB_ROWS 6

int current_play_row = 0; // the row to guess

LB lb[LB_COLS][LB_ROWS];

void show_lb(FILE *outf) {
    // word can't have yellow in certain columns
  int tcol = 0;
  int trow = 0;

  fprintf(outf,"Game Board\n");
  
  while ( trow < current_play_row ) {
    for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
      switch ( lb[tcol][trow].flag ) {
      case LB_FLAG_GREY:
	fprintf(outf,"%c:%s   ",lb[tcol][trow].letter,"GREY");
	break;
      case LB_FLAG_GREEN:
	fprintf(outf,"%c:%s  ",lb[tcol][trow].letter,"GREEN");
	break;
      case LB_FLAG_YELLOW:
	fprintf(outf,"%c:%s ",lb[tcol][trow].letter,"YELLOW");
	break;
      }
    } // for
    fprintf(outf,"\n");
    trow += 1;
  } // while
}

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

// of the form p , o , w:y , e , r
int load_play ( char *str ) {
  char *c = str;
  char letter;
  int color;
  int i;

  if ( current_play_row >= LB_ROWS ) {
    printf("load_play: game full\n");
    return 0;
  }
  
  for ( i = 0 ; i < 5 ; i++ ) {
    color = LB_FLAG_GREY; // set default color
    if ( *c == 0 ) {
      printf("load_play: end of file ???\n");
      return 0;
    }
    skip_space_tab(&c);
    if ( *c == ',' ) {
      c += 1;
    }
    skip_space_tab(&c);

    letter = *c;

     if ( lb[i][current_play_row-1].flag == LB_FLAG_GREEN &&
	 lb[i][current_play_row-1].letter == letter ) {
       goto is_ok;
    }

    if ( !ok_buf[letter] ) {
      printf("load_play: invalid letter at <%s>\n",c);
      return 0;
    }
  is_ok:;
    if ( *c ) c += 1;
    skip_space_tab(&c);
    if ( *c == ':' ) {
      if ( *c ) c += 1;
      switch ( *c ) {
      case 'g':
	color = LB_FLAG_GREEN;
	break;
      case 'y':
	color = LB_FLAG_YELLOW;
	break;
      default:
	printf("load_play: unknown color at <%s>\n",c);
	return 0;
      } // switch
      c += 1;
    } // if  ':'
    if ( LB_FLAG_GREY == color ) {
      // never play this letter again
      ok_buf[letter] = 2;
    }
    // save on game board
    lb[i][current_play_row].letter = letter;
    lb[i][current_play_row].flag   = color;
  } // for i

  current_play_row += 1;

  // change any 2's to 0;s
  for ( i = 'a' ; i <= 'z' ; i += 1 ) {
    if ( ok_buf[i] == 2 ) {
      ok_buf[i] = 0;
    }
  }
  
  return 1;
}

int load_game_txt ( void )
{
  char wbuf[256];
  FILE *inf;
  char *c;
  inf = fopen("game.txt","r");
  if ( !inf ) {
    printf("load_game_txt: new game, no game.txt\n");
    return 0;
  }

  while ( fgets(wbuf,sizeof(wbuf), inf) ) {
    c = strchr(wbuf,'\n'); if ( c ) *c = 0;
    if ( wbuf[0] == '#' ) continue;
    if ( strlen(wbuf) == 0 ) continue;

    load_play(wbuf);
  }

  fclose(inf);
  return 1;
 }

int save_game_txt ( void )
{
  int col,row;
  FILE *outf;
  char color = 'x';
  
  unlink("game.txt");
  outf = fopen("game.txt","w");
  if ( !outf ) {
    printf("save_game_txt: can't create fame.txt\n");
    exit(0);
  }
  fprintf(outf,"#\n# game.txt - save the state of the game\n#\n");
  
  for ( row = 0 ; row < LB_ROWS ; row += 1 ) {
    for ( col = 0 ; col < LB_COLS ; col += 1 ) {
      color = 'x';
      switch ( lb[col][row].flag ) {
      case LB_FLAG_GREY:
	break;
      case LB_FLAG_GREEN:
	color = 'g';
	break;
      case LB_FLAG_YELLOW:
	color = 'y';
	break;
      default:
	goto finis;
	break;
      }
      if ( col > 0 ) {
	fprintf(outf," , ");
      }
      if ( lb[col][row].flag == LB_FLAG_GREY ) {
	fprintf(outf,"%c", lb[col][row].letter);
      } else {
	fprintf(outf,"%c:%c", lb[col][row].letter, color);
      }
    } // for col
    fprintf(outf,"\n");
  } // for row
 finis:;
  fclose(outf);
  return 1;
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
      printf("Unknown color %c\n",*c);
      exit(0);
      break;
    }
    skip_space_tab(&c);

    lb[col][current_play_row].letter = letter;
    lb[col][current_play_row].flag = flag;
    
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

  // make sure not already on list
  new = root_ans_txt;
  while ( new ) {
    if ( 0 == strcmp(new->word,word) ) {
      return;
    }
    new = new->next;
  }
  
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
// but green letters override grey ones
int letters_valid ( char *buf )
{
  int i;
  
  //printf("letters_valid: word: <%s>\n",buf);

  for ( i = 0 ; i < 5 ; i += 1 ) {
    if ( lb[i][current_play_row-1].flag == LB_FLAG_GREEN &&
	 lb[i][current_play_row-1].letter == buf[i] ) {
      continue;
    }

    if ( !ok_buf[buf[i]] ) {
      //printf("letters_valid: invalid letter at <%s>\n",c);
      return 0;
    }
  }

  return 1;
#if 0  
  /* can only use these letters */
  c = (char *)buf;
  while ( *c != 0 ) {
    if ( !ok_buf[(int)*c] ) {
      //printf("letters_valid: invalid letter at <%s>\n",c);
      return 0;
    }
    c += 1;
  }
  return 1;
#endif
}
    
int main(int argc, char *argv[])
{
  FILE *inf, *outf;
  char *c;
  int l_index = 1;
  char filename[64];
  char system_cmd[64];
  ANS_TXT *tmp;
  int approved_words = 0;
  int other_words = 0;
  int i;
  char letter;
  
  printf("%s.%s\n",VERSION_STR,GIT_VERSION);
  printf("Build Date: %s\n",BUILD_DATE);

  if ( argc < 1 || argc > 2 ) {
    printf("usage: ./wos [-n]/[a play]\n");
    exit(0);
  }
  use_a = 1;
  
  if ( argv[1][0] == '-' && argv[1][1] == 'n' ) {
    printf("new game\n");
    unlink("game.txt");
    unlink("ans.txt");
    exit(0);
  }

  /* allow only these letters to start */
  for ( i = 'a' ; i <= 'z' ; i++ ) {
    ok_buf[i] = 1;
  }
  
  // load game
  load_game_txt();
  show_lb(stdout);

  /* get right dictionary */
  strcpy(filename,"wos_words.txt");
  inf = fopen(filename, "r");
  if ( !inf ) {
    printf("%s not found\n",filename);
    exit(0);
  }
  // print some stats
  printf("Opened %s\n",filename);

  // make the play
  if ( ! load_play(argv[1]) ) {
    printf("can't make the play <%s>\n",argv[1]);
    exit(0);
  }

  show_lb(stdout);

  //
  // check certain placement rules for this play
  //
  
  // yellows can't be in last row at known positions
  int tcol = 0;
  int trow = 0;
  while ( tcol < LB_COLS ) {
    letter = '#';  // set to invalid
    for ( trow = 0 ; trow < current_play_row-1 ; trow += 1 ) {
      if ( lb[tcol][trow].flag == LB_FLAG_YELLOW ) {
	letter = lb[tcol][trow].letter;
      }
    } // for trow
    if ( current_play_row > 1 ) {
      if ( letter != '#' ) {
	if ( lb[tcol][trow].letter == letter ) {
	  printf("Yellow letter can't be in column %d\n",tcol + 1 );
	  exit(0);
	}
      }
    }
    tcol += 1;
  }

  // last row must contain all yellows
  tcol = 0;
  trow = 0;
  memset(yellow_list,0,sizeof(yellow_list));
  while ( tcol < LB_COLS ) {
    for ( trow = 0 ; trow < current_play_row ; trow += 1 ) {
      if ( lb[tcol][trow].flag == LB_FLAG_YELLOW ) {
	yellow_list[lb[tcol][trow].letter] = 1;
      }
    } // for trow
    tcol += 1;
  }
  for ( i = 'a' ; i <= 'z' ; i += 1 ) {
    if ( yellow_list[i] ) {
      // must contain this one
      tcol = 0;
      trow = current_play_row -1;
      if ( trow > 0 ) {
	for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	  if ( (char)i == lb[tcol][trow].letter ) {
	    yellow_list[i] = 0;
	  }
	}
      }
    }
  }
  for ( i = 'a' ; i <= 'z' ; i += 1 ) {
    if ( yellow_list[i] ) {
      printf("missing yellow character %c in last row\n",i);
    }
  }

  // greens must be in the right place
  tcol = 0;
  trow = 0;
  while ( tcol < LB_COLS ) {
    letter = '#';  // set to invalid
    for ( trow = 0 ; trow < current_play_row-1 ; trow += 1 ) {
      if ( lb[tcol][trow].flag == LB_FLAG_GREEN ) {
	letter = lb[tcol][trow].letter;
      }
    } // for trow
    if ( current_play_row > 1 ) {
      if ( letter != '#' ) {
	if ( lb[tcol][trow].letter != letter ) {
	  printf("Green letter missing in column %d\n",tcol + 1 );
	  exit(0);
	}
      }
    }
    tcol += 1;
  }
  
  //
  // now, show what we can do next by scanning our word list
  //

  //printf("Scanning word list\n");
  
  // scan entire word list *.txt
  while ( fgets((char * restrict)work_buffer,sizeof(work_buffer), inf) ) {
    int len;
    
    // get rid of obviously invalid records
    if ( strlen((const char *)work_buffer) == 0 ) continue;
    
    // get rid of new-line
    c = strchr((const char *)work_buffer,'\n'); if ( c ) *c = 0;
    c = strchr((const char *)work_buffer,' '); if ( c ) *c = 0;
    
    // save length of string
    len = strlen((const char *)work_buffer);
    
    // reject any words not 5 in length
    if ( len != 5 ) continue;

    // all letters must be valid
    if ( !letters_valid((char *)work_buffer) ) continue;

    //printf("%s: all letters valid\n",work_buffer);
    
#if 0
    // already done by setup
    c = (char *)work_buffer;
    while ( *c != 0 ) {
      *c = tolower(*c) & 127;
      c += 1;
    }
#endif // 1

#if 0    
    // proposed word can't contain grey letters
    c = (char *)work_buffer;
    while ( *c ) {
      if ( !ok_buf[*c] ) {
	goto is_not_ok;
      }
      c += 1;
    }
#endif
    
    /* green letters must be in a particular word column */
    tcol = 0;
    trow = 0;
    c = (char *)work_buffer;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( lb[tcol][trow].flag == LB_FLAG_GREEN &&
	     c[tcol] != lb[tcol][trow].letter ) {
	  goto is_not_ok;
	}
      }
      trow += 1;
    }

    // word can't have yellow in certain columns
    tcol = 0;
    trow = 0;
    c = (char *)work_buffer;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( lb[tcol][trow].flag == LB_FLAG_YELLOW ) {
	  if ( c[tcol] == lb[tcol][trow].letter ) {
	    goto is_not_ok;
	  }
	}
      }
      trow += 1;
    }

    // word must contain yellow letters
    tcol = 0;
    trow = 0;
    c = (char *)work_buffer;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( lb[tcol][trow].flag == LB_FLAG_YELLOW ) {
	  if ( NULL == strchr(c,lb[tcol][trow].letter) ) {
	    goto is_not_ok;
	  }
	}
      }
      trow += 1;
    }

    // finally, add the word
    save_word((char *)work_buffer, 0);

  is_not_ok:;
  } // while fgets

  fclose(inf);

  // scan entire nytwords
  int idx = 0;
  while ( (c=get_nytword(idx++)) ) {
    
    // get rid of obviously invalid records
    if ( strlen((const char *)work_buffer) == 0 ) continue;
    
    // all letters must be valid
    if ( !letters_valid((char *)work_buffer) ) continue;

    /* green letters must be in a particular word column */
    tcol = 0;
    trow = 0;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( lb[tcol][trow].flag == LB_FLAG_GREEN &&
	     c[tcol] != lb[tcol][trow].letter ) {
	  goto is_not_ok_1;
	}
      }
      trow += 1;
    }

    // word can't have yellow in certain columns
    tcol = 0;
    trow = 0;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( lb[tcol][trow].flag == LB_FLAG_YELLOW ) {
	  if ( c[tcol] == lb[tcol][trow].letter ) {
	    goto is_not_ok_1;
	  }
	}
      }
      trow += 1;
    }

    // word must contain yellow letters
    tcol = 0;
    trow = 0;
    while ( trow < current_play_row ) {
      for ( tcol = 0 ; tcol < LB_COLS ; tcol += 1 ) {
	if ( lb[tcol][trow].flag == LB_FLAG_YELLOW ) {
	  if ( NULL == strchr(c,lb[tcol][trow].letter) ) {
	    goto is_not_ok_1;
	  }
	}
      }
      trow += 1;
    }

    // finally, add the word
    save_word(c, 0);

  is_not_ok_1:;
  } // while get_nytword

  //
  // so we have a possible list stored at root_ans_list
  //

#if 1
  // generate a prototype ans.txt
  unlink(ANS_TXT_STR);
  outf = fopen(ANS_TXT_STR,"w");
  if ( !outf ) {
    fprintf(stderr, "%d: %s can't be created\n",__LINE__,ANS_TXT_STR);
    exit(0);
  }
  tmp = root_ans_txt;
  while ( tmp ) {
    fprintf(outf,"%s : %d\n",tmp->word,calc_weight(tmp->word));
    tmp = tmp->next;
  }
  fclose(outf);
#endif
  
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
  show_lb(outf);

  show_lb(stdout);
    
  tmp = root_ans_txt;
  while ( tmp ) {
    if ( !(tmp->flags & FLAGS_duplicate) )  {
      if ( !(tmp->flags & FLAGS_bad) )  {

	// must by an nyt word
	if ( !is_nytword(tmp->word) ) {
	  goto forward;
	}
	
	approved_words += 1;
	if ( tmp->flags & FLAGS_pangram )  {
	  fprintf(outf,"* %s\n",tmp->word);
	  fprintf(stdout,"* %s\n",tmp->word);
	} else {
	  fprintf(outf,"%s : %4d\n",tmp->word, calc_weight(tmp->word));
	  printf("%s : %4d\n",tmp->word, calc_weight(tmp->word));
	}
      forward:;
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

  // save our game state
  save_game_txt();
  
#if defined(USE_DEBUG)
  dump_list();
#endif
  
  // done. Note - let exit do all memory cleanup
  return 0;
}
