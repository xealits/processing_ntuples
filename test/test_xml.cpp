/** testing xml parsing

printout all dtag names and their cross sections from dsets_info file

with: libxml

compile  with \c -I/usr/include/libxml2
and link with \c -lxml2
*/

#include <stdio.h>
#include <iostream>
#include <libxml2/libxml/xpath.h>


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


int main (int argc, char *argv[])
{
// skip the program name
argc--;
const char* exec_name = argv[0];
argv++;

if (argc < 1)
	{
	std::cout << "Usage:" << " path/to/dsets_info.xml" << std::endl;
	exit(0);
	}

// the dsets_info filename
const char* dsets_info_filename = argv[0];

// xml document
xmlDocPtr doc = xmlParseFile(dsets_info_filename);
Stopif(!doc, return -1, "Error: unable to parse file %s\n", dsets_info_filename);

// we'll search for tags with XPath
// set up the XPath system:
xmlXPathContextPtr xpath_context = xmlXPathNewContext(doc);
Stopif(!xpath_context, return -2, "Error: unable to create the XPath context\n");

// free all xml resources
xmlFreeDoc(doc);
return 0;
}
