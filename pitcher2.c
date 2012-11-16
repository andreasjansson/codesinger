#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#define TWOPI 6.283185307179586
#define C_0   16.351597831287414

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
  return c < 100;
}

inline void phase_add(double *phase, double incr)
{
  *phase += incr;
  if(*phase > TWOPI)
    *phase -= TWOPI;
}

inline int target_getc(FILE *file)
{
  int c = fgetc(file);
  if(c == EOF) {
    rewind(file);
    c = fgetc(file);
  }
  return c;
}

int main(int argc, char *argv[])
{
  double sr = 22050;
  double phase = 0;
  double phase_incr;// = freq * TWOPI / sr;
  double tempo = 140;
  int c;
  int d;
  int prevc = 0;
  State state = in_code;
  int t;
  FILE *target = fopen("song.raw", "r");

  while((c = fgetc(stdin)) != EOF) {

    t = target_getc(target);

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
          t = target_getc(target);
        }
      }
      else if(isspace(c) && !space_at(t)) {
        while((d = fgetc(stdin)) != EOF && isspace(d))
          ;
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
}