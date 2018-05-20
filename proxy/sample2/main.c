#include <stdio.h>

int main(int argc, char *argv[]) {
	if(argc<2) {
		printf("%s <filename>\n", argv[0]);
	}

	FILE *fp = fopen(argv[1], "r");
	char buffer[2048];
	while(fgets(buffer, sizeof(buffer), fp) != NULL) {
		printf("%s", buffer);
	}

	fclose(fp);
	return 0;
}
