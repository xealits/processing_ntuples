/** testing the dsets_info function
*/

#include <stdio.h>
#include "UserCode/proc/interface/DsetsInfo.hpp"

#include "UserCode/proc/interface/Stopif.h"

int main (int argc, char *argv[])
{
// skip the program name
argc--;
const char* exec_name = argv[0];
argv++;

if (argc < 1)
	{
	printf("Usage:" " filename" "\n");
	return -113;
	}

// the filename with the data, histograms or TTrees
const char* data_filename = argv[0];

DsetsInfo_setup("./dsets_info_3.xml");

const char* dtag = DsetsInfo_match_filename_to_dtag(data_filename);

Stopif(dtag==NULL, return -1, "Error: could not find a matching dtag for the filename %s\n", data_filename);

double xsec = DsetsInfo_xsec_per_dtag(dtag);

printf("xsec = %g\n", xsec);

DsetsInfo_free();
return 0;
}

