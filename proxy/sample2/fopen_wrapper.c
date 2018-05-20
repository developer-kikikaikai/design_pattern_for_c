#include <stdio.h>
#define __USE_GNU
#include <dlfcn.h>

FILE *fopen(const char *path, const char *mode) {

	printf("=====proxy update: open file %s=====\n", path);
	void *handle = dlsym(RTLD_NEXT, "fopen");
	if(!handle) {
		return NULL;
	}

	FILE* (*func)(const char *, const char*) = (FILE* (*)(const char *, const char*))handle;
	return func(path, mode);
}
