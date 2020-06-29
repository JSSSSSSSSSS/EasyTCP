#include<iostream>

void free(void *p)
{
	delete p;
}
int main()
{
	char* p = new char[1024 * 1024* 1024];
	free(p);

	getchar();
	return 0;
}