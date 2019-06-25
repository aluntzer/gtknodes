#!/bin/bash

touch NEWS
if [[ $(uname) == "Darwin" ]]; then
	glibtoolize
else
	libtoolize
fi

aclocal
autoconf
gtkdocize
automake --add-missing
