#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
int main()
{
	srand(time(NULL));
	fstream file;
	string fn = "file2.txt";
	file.open(fn, ios::out);
	if(!file.is_open())
	{
		return -1;
	}
	int i = 0;
	while(i < 10500)
	{
		if(rand() % 12 == 0)
			file << "printf" << endl;
		else if (rand() % 9 == 0)
			file << "cout" << endl;
		else if (rand() % 3 == 0)
			file << "Bonjour";
		else if (rand() % 2 == 0)
			file << "Bonjour ";
		else
			file << " ";
		i++;
	}
	file.close();
	fn = "file3.txt";
	file.open(fn, ios::out);
	if(!file.is_open())
	{
		return -1;
	}
	i = 0;
	while(i < 1500)
	{
		if(rand() % 3 == 0)
			file << "cout" << endl;
		else
			file << endl;
		i++;
	}
	file.close();
	fn = "file4.txt";
	file.open(fn, ios::out);
	if(!file.is_open())
	{
		return -1;
	}
	i = 0;
	while(i < 1500)
	{
		if (rand() % 3 != 0)
			file << "cout" << endl;
		else if (rand() % 2 != 0)
			file << "printf" << endl;
		else
			file << endl;
		i++;
	}
	file.close();
	fn = "file1.txt";
	file.open(fn, ios::out);
	if(!file.is_open())
	{
		return -1;
	}
	i = 0;
	while(i < 1500)
	{
		if (rand() % 4 == 0)
			file << "printf" << endl;
		else
			file << endl;
		i++;
	}
	file.close();
}