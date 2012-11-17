// sox free-software-song.new2.ogg -u -t raw -r 44100 -c1 - lowpass 1000 > song.raw

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sndfile.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#define APPNAME "codesinger"

typedef enum {
  in_code,
  in_single_quote,
  in_double_quote,
  in_single_line_comment,
  in_multi_line_comment
} State;

typedef enum {
  false,
  true
} bool;

inline bool space_at(int c)
{
  return c < (1 << 15) + (1 << 10);
}

inline int target_getbyte(FILE *file)
{
  int c1 = fgetc(file);
  int c2 = fgetc(file);
  if(c1 == EOF || c2 == EOF) {
    rewind(file);
    c1 = fgetc(file);
    c2 = fgetc(file);
  }

  return (c2 << 8) + c1;
}

void usage()
{
  fprintf(stderr, "Usage: %s -t TARGET\n", APPNAME);
}

int main(int argc, char *argv[])
{
  int c;
  int d;
  int prevc = 0;
  State state = in_code;
  int t;
  char *target_filename = NULL;
  FILE *target;
  int opt;

  while((opt = getopt(argc, argv, "t:")) != -1) {
    switch(opt) {
    case 't':
      target_filename = strdup(optarg);
      break;
    default:
      usage();
      exit(1);
    }
  }

  if(!target_filename) {
    usage();
    exit(1);
  }

  if(!(target = fopen(target_filename, "r"))) {
    fprintf(stderr, "No such file: %s\n", target_filename);
    exit(1);
  }

  while((c = fgetc(stdin)) != EOF) {

    t = target_getbyte(target);

    switch(state) {
    case in_code:
      if(c == '\'') {
        state = in_single_quote;
      }
      else if(c == '"') {
        state = in_double_quote;
      }
      else if(c == '#' && prevc == '\n') {
        // pass
      }
      else if(c == '*' && prevc == '/') {
        state = in_multi_line_comment;
      }
      else if(c == '/' && prevc == '/') {
        state = in_single_line_comment;
      }
      else if(!isspace(c) && isspace(prevc)) {
        while(space_at(t)) {
          putchar(' ');
          t = target_getbyte(target);
        }
      }
      else if(isspace(c) && !space_at(t)) {
        while((d = fgetc(stdin)) != EOF && isspace(d)) {
          // if there's a newline, we should output that rather than
          // other spaces. this is for preprocessor statements.
          if(d == '\n')
            c = d;
        }
        ungetc(d, stdin);
      }
      break;

    case in_single_quote:
      if(c == '\'' && prevc != '\\')
        state = in_code;
      break;

    case in_double_quote:
      if(c == '"' && prevc != '\\')
        state = in_code;
      break;

    case in_multi_line_comment:
      if(c == '/' && prevc == '*')
        state = in_code;
      break;

    case in_single_line_comment:
      if(c == '\n')
        state = in_code;
      break;
    }

    putchar(c);

    if((state == in_single_quote || state == in_double_quote) && c == '\\' && prevc == '\\')
      prevc = 0;
    else
      prevc = c;
  }

  return 0;
}
