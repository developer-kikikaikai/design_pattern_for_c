#ifndef PROXYFUNF_H_
#define PROXYFUNF_H_
int testfunc_(const char *filename, const char *funcname, const int line, int data);
#define testfunc(data) testfunc_(__FILE__, __FUNCTION__, __LINE__, data)
#endif

