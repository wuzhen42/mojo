%module(directors="1") MojoBlender

%include "../swig.i"

%{
    #include "pencil.h"
%}

namespace mojo {
	%ignore Pencil::to_words;
	%ignore Pencil::set;
	%ignore Pencil::build;
	%ignore Pencil::begin;
	%ignore Pencil::end;
}

%include "../pencil.h"

