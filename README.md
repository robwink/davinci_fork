davinci
=======
[![Build Status](https://travis-ci.org/robwink/davinci.svg?branch=master)](https://travis-ci.org/robwink/davinci)


This is a fork of [davinci.asu.edu](davinci.asu.edu).  It has dozens of bug
fixes and 100's of warning fixes (hard to believe because there are still so
many in this terrible ancient monstrosity of a project).

It also adds many features, including all the missing types
(int8, uint16, uint32, uint64 and int64) and most noticably and helpfully,
tab completion.

Personally, I think davinci should have been retired well over a decade ago
and replaced with Python.  Any functionality isn't readily
available in Python or various librares (numpy, scipy, plotting libs etc.)
could easily be added as C modules adapted from davinci code if performance
is an issue, or rewritten in python.  The davinci standard library would likewise
be rewritten in python.  Aside from how terribly it's implemented and organized,
davinci is a terrible language so that would be a huge plus for davinci users.
Not to mention python is used everywhere now, esp. academia, so they probably
already know it and can now more efficiently/effectively share with
other scientists.  Also it'd be far cheaper and easier to maintain a "davinci"
python library than an entire separate language.  [Anaconda](https://www.anaconda.com/distribution/)
is probably a good start.

## Compiling
Davinci requries a C99 compliant compiler.  A configuration script has been
included to make compilation as straight forward as possible.  In
most instances you should be able to run configure, and then run make:

	% ./configure
	% make
	% make install

If that doesn't work check [the wiki page](http://davinci.asu.edu/index.php?title=Compiling_Davinci)
for OS specific details.  It may help, but remember that version may have diverged enough
that it may be wrong


## Documentation
Documentation can be found on the wiki at [davinci.asu.edu](davinci.asu.edu) however
the same caveats as above.  This versions has functions that may not be there, or
if they are there, there may be more functionality than is documented or work
slightly differently.

You're better off using the file dv.gih, which is distributed with davinci and can be
accessed from within the interpreter, documents all the functions as they are in
this codebase.

