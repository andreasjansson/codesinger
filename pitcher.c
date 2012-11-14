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

typedef struct {
  double pitch;
  double dur;
} Note;

inline bool space_at(double phase)
{
  return phase > M_PI;
}

inline void phase_add(double *phase, double incr)
{
  *phase += incr;
  if(*phase > TWOPI)
    *phase -= TWOPI;
}

inline double pitch_to_phase_incr(double pitch, double sr)
{
  return C_0 * pow(2, pitch / 12) * TWOPI / sr;
}

int main(int argc, char *argv[])
{
  double sr = 22050;
  double phase = 0;
  double phase_incr;// = freq * TWOPI / sr;
  double tempo = 120 * 2;
  int c;
  int d;
  int prevc = 0;
  State state = in_code;

  Note melody[] = {
    {55, 1},
    {53, .5},
    {52, 1},
    {50, 1},
    {52, 1},
    {53, .5},
    {52, .5},
    {50, .5},
    {48, 1},
    {48, 1.5},
    {50, 1},
    {52, 1},
    {53, 1.5},
    {52, 1},
    {55, 1},
    {50, 1.5},
    {50, 1},
    {50, 1},
    {55, 1},
    {53, .5},
    {52, 2}
  };

  int note_count = 21;
  int curnote = 0;
  double t = 0;
  double t_incr = (tempo / 60) / sr;

  phase_incr = pitch_to_phase_incr(melody[curnote].pitch, sr);

  while((c = fgetc(stdin)) != EOF) {

    if(t >= melody[curnote].dur) {
      t -= melody[curnote].dur;
      curnote ++;
      if(curnote >= note_count)
        curnote = 0;
      phase_incr = pitch_to_phase_incr(melody[curnote].pitch, sr);
    }
    t += t_incr;

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
        while(space_at(phase)) {
          putchar(' ');
          phase_add(&phase, phase_incr);
        }
      }
      else if(isspace(c) && !space_at(phase)) {
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
    phase_add(&phase, phase_incr);

    if((state == in_single_quote || state == in_double_quote) && c == '\\' && prevc == '\\')
      prevc = 0;
    else
      prevc = c;
  }
}
