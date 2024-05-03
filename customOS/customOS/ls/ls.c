#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

void ls(const char *dir, int op_a, int op_l)
{
	// Here we will list the directory
	struct dirent *d;
	DIR *dh = opendir(dir);
	if (!dh)
	{
		if (errno = ENOENT)
			perror("Directory doesn't exist");
		else
			perror("Unable to read directory");
		exit(EXIT_FAILURE);
	}
	// While the next entry is not readable we will print directory files
	while ((d = readdir(dh)) != NULL)
	{
		// If hidden files are found we continue
		if (!op_a && d->d_name[0] == '.')
			continue;
		printf("%s  ", d->d_name);
		if (op_l)
			printf("\n");
	}
	if (!op_l)
		printf("\n");
}

int main(int argc, const char *argv[])
{
	int op_a = 0, op_l = 0;
	if (argc == 1)
	{
		ls(".", 0, 0);
	}
	else if (argc == 2)
	{
		if (argv[1][0] == '-')
		{
			// Checking if option is passed
			// Options supporting: a, l
			char *p = (char *)(argv[1] + 1);
			while (*p)
			{
				if (*p == 'a')
					op_a = 1;
				else if (*p == 'l')
					op_l = 1;
				else
				{
					perror("Option not available");
					exit(EXIT_FAILURE);
				}
				p++;
			}
			ls(".", op_a, op_l);
		}
	}
	return 0;
}