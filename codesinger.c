#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sndfile.h>
#include <ctype.h>
#include <stdbool.h>

#define APPNAME "codesinger"
#define MAX_CHANNELS 2

typedef enum {
  in_code,
  in_single_quote,
  in_double_quote,
  in_single_line_comment,
  in_multi_line_comment
} State;

inline bool space_at(float sample, float threshold)
{
  return sample < threshold;
}

inline float get_sample(SNDFILE *file, int channels)
{
  static float s[MAX_CHANNELS];
  if(sf_read_float(file, s, channels) == 0) {
    sf_seek(file, 0, SEEK_SET);
    sf_read_float(file, s, channels);
  }
  return s[0];
}

void usage()
{
  fprintf(stderr, "Usage: %s [-t threshold] target\n", APPNAME);
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
  float threshold = 0;

  while((opt = getopt(argc, argv, "t:")) != -1) {
    switch(opt) {
    case 't':
      threshold = atof(optarg);
      break;
    default:
      usage();
      exit(1);
    }
  }

  if(argc - optind != 1) {
    usage();
    exit(1);
  }

  target_filename = argv[optind];

  if(!(target = sf_open(target_filename, SFM_READ, &sf_info))) {
    fprintf(stderr, "No such file: %s\n", target_filename);
    exit(1);
  }

  while((c = fgetc(stdin)) != EOF) {

    sample = get_sample(target, sf_info.channels);

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
        while(space_at(sample, threshold)) {
          putchar(' ');
          sample = get_sample(target, sf_info.channels);
        }
      }
      else if(isspace(c) && !space_at(sample, threshold)) {
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
