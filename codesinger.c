// sox free-software-song.new2.ogg -t wav -r 44100 -c1 song.wav lowpass 1000

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

inline bool space_at(float sample)
{
  return sample < .1;
}

inline float get_sample(SNDFILE *file)
{
  float s;
  if(sf_read_float(file, &s, 1) == 0) {
    sf_seek(file, 0, SEEK_SET);
    sf_read_float(file, &s, 1);
  }
  return s;
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
  float sample;
  char *target_filename = NULL;
  SNDFILE *target;
  int opt;
  SF_INFO sf_info;

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

  if(!(target = sf_open(target_filename, SFM_READ, &sf_info))) {
    fprintf(stderr, "No such file: %s\n", target_filename);
    exit(1);
  }

  while((c = fgetc(stdin)) != EOF) {

    sample = get_sample(target);

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
        while(space_at(sample)) {
          putchar(' ');
          sample = get_sample(target);
        }
      }
      else if(isspace(c) && !space_at(sample)) {
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
