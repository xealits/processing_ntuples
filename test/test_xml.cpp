/** testing xml parsing

printout all dtag names and their cross sections from dsets_info file

with: libxml

compile  with \c -I/usr/include/libxml2
and link with \c -lxml2
*/

#include <stdio.h>
#include <string.h>
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

/* we don't need this because the XML is structured better now
xmlNodePtr xmlNode_findChild_by_name(xmlNodePtr xml_node, const xmlChar* name)
	{
	// loop through children and look for the name
	xmlNodePtr child=xml_node->children;
	for (; child!=NULL; child=child->next)
		{
		//printf("%s\n", child->name);
		//if (strcmp((const char*) child->name, (const char*) name)) return child;
		if (xmlStrcmp(child->name, name)) return child;
		}
	return child;
	}
*/

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
//xmlDocPtr doc = xmlParseFile(dsets_info_filename); // this does not allow options, which are needed to parse more than 256 elements....xml is really a pain
xmlDocPtr doc = xmlReadFile(dsets_info_filename, NULL, XML_PARSE_HUGE);
Stopif(!doc, return -1, "Error: unable to parse file %s\n", dsets_info_filename);

// we'll search for tags with XPath
// set up the XPath system:
xmlXPathContextPtr xpath_context = xmlXPathNewContext(doc);
Stopif(!xpath_context, return -2, "Error: unable to create the XPath context\n");

// get the nodes with a given tag name
const xmlChar* xpath_hard_proc = (xmlChar*)"//hard_process";
xmlXPathObjectPtr hard_procs = xmlXPathEvalExpression(xpath_hard_proc, xpath_context);
Stopif(!hard_procs, return -3, "Error: could not XPath //hard_process");

// loop over found tags
for (int i=0; i<hard_procs->nodesetval->nodeNr; i++)
	{
	// dumping all content
	//printf("%s\n", xmlNodeGetContent(hard_procs->nodesetval->nodeTab[i]));
	// print their names
	//printf("%s\n", hard_procs->nodesetval->nodeTab[i]->name);
	xmlNodePtr hp = hard_procs->nodesetval->nodeTab[i];

	// get xsec and dtags
	if (xmlHasProp(hp, (const xmlChar*)"xsec"))
		printf("xsec = %10.3g\t", atof((const char*)xmlGetProp(hp, (const xmlChar*)"xsec")));
	else
		printf("xsec = %10s", "NAN");

	// the dsets are easy: the children are dsets + some random "text" files?? -- some internal XML crap
	xmlNodePtr dset=hp->children;
	for (; dset!=NULL; dset=dset->next)
		{
		if (xmlStrcmp(dset->name, (const xmlChar*)"dset") != 0) continue; // watch out how this comparison works!
		printf(" %20s", (const char*)xmlGetProp(dset, (const xmlChar*)"dtag"));
		}
	printf("\n");
	}

// free all xml resources
xmlXPathFreeObject(hard_procs);
xmlXPathFreeContext(xpath_context);
xmlFreeDoc(doc);
return 0;
}
