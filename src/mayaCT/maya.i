%module(directors="1") MojoMaya

%include "../swig.i"

%{
    #include "mayaConnector.h" 
%}

%feature("director") MayaConnector;

%include "mayaConnector.h"

