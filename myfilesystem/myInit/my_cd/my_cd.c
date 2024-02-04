//Importing necessary header files
#include<stdio.h>
#include<unistd.h>


int main(int argc, const char *argv[])
{ 
    //Changing the current working directory 
    chdir(argv[1]);
	return 0;
}