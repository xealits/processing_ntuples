#ifndef STOPIF_H
#define STOPIF_H

#include <stdio.h>
#include <stdlib.h> //abort

char error_mode = 'a';
FILE * error_log;

#define Stopif(assertion, error_action, ...) {                           \
	if (assertion){                                                  \
		fprintf(error_log ? error_log : stderr, __VA_ARGS__);    \
		fprintf(error_log ? error_log : stderr, "\n");           \
	if (error_mode=='s') abort();                                    \
	else                {error_action;}                              \
	}                                                                \
}

#endif /* STOPIF_H */

