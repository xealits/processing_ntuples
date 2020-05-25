#ifndef HANDYMACROS_H
#define HANDYMACROS_H

#include <stdio.h>
#include <stdlib.h> //abort

#define cout_expr(Name)   cout << #Name << " = " << Name << std::endl
#define cerr_expr(Name)   cerr << #Name << " = " << Name << std::endl

/**
The mode of `Stopif` behavior: on a stop execute `error_action` or abort the program
Set this to \c 's' to stop the program on error. Otherwise execure error actions. */
char error_mode = 'a';

/** where to write errors? if this is \c NULL, write to \c stderr. */
FILE * error_log;

/** \brief Macro to handle exceptions: if assertion true print an error, then execute a user action or abort the program.

\param assertion any expression
\param error_action any other expression
\param ... a printf-like coma-separated input: "foo %s bar", string

Set the error_mode to \c 's' to stop the program on error. Otherwise execure error actions.

Example: `Stopif(!TFile::Open(input_file), continue, "cannot Open the file %s", input_file);` in a loop over filenames, to continue if a file is not found.*/

#define Stopif(assertion, error_action, ...) {                           \
	if (assertion){                                                  \
		fprintf(error_log ? error_log : stderr, __VA_ARGS__);    \
		fprintf(error_log ? error_log : stderr, "\n");           \
	if (error_mode=='s') abort();                                    \
	else                {error_action;}                              \
	}                                                                \
}


#endif /* HANDYMACROS_H */

