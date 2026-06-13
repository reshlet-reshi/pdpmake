/*
 * Parse a makefile
 */
#include "make.h"
#include <glob.h>

int lineno;	// Physical line number in file
int dispno;	// Line number for display purposes
