#include <stdio.h>
#include <sys/stat.h>

int main(int argc, const char *argv[])
{
	const char *new_dir = argv[1];
	mkdir(new_dir, 0755);
	return 0;
}
