#ifndef DSETSINFO_H
#define DSETSINFO_H

int DsetsInfo_setup(const char* dsets_info_filename="./dsets_info_3.xml");
int DsetsInfo_free();
double DsetsInfo_xsec_per_dtag(const char* target_dtag);
const char* DsetsInfo_match_filename_to_dtag(const char* filename);

#endif /* DSETSINFO_H */
