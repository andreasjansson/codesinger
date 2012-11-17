The Code Singer
===============

The Code Singer is a sort of code obfuscator that inserts and removes insignificant white space in order to match the bytes of a target audio file, whilst not breaking or changing the semantics of the source code.

Conceived during Music Hackday London 2012.

Usage
-----

    ./codesinger -t threshold target < infile > outfile

`threshold` is a number in the range [-1, 1] that defines at what target amplitude spaces will be inserted. The higher the value, the sparser the output waveform. Typically, `threshold` should be between [0, .3].

`target` is an audio file, in a format understood by libsndfile, e.g. WAV, AIFF, AU, etc. The sampling rate of `target` will be matched in `outfile`.

`infile` is the input source code; `outfile` is the modified source code.

Syntax considerations
---------------------

The Code Singer only works (as in, doesn't break the code) with C-like languages. The following syntactic constructs are considered:

 * Single quote enclosed strings
 * Double quote enclosed strings
 * C-style, multi-line comments (`/* */`)
 * C++-style, single-line comments (`//`)
 * C preprocessor constructs (where `#` is the first character on the line)

Dependencies
------------

The only dependency is [libsndfile](http://www.mega-nerd.com/libsndfile).

Bugs
----

Definitely!
