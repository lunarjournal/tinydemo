#ifndef _MEM_H
#define _MEM_H
#include "../../sys/sys.h"
#include <sys/mman.h>

void memcopy( void * dest, void *source, unsigned long size )
{
	unsigned long i;
	unsigned char * d,*s;

	d=(unsigned char*)dest;
	s=(unsigned char*)source;
	for(i=0;i<size;i++)
	{
		d[i]=s[i];
	}
}

void memclear( void * dest, unsigned char value, unsigned long size )
{
	unsigned long i;
	unsigned char * d;

	d = (unsigned char*)dest;
	for(i=0;i<size;i++)
	{
		d[i]=value;
	}
}

void *mallocate(int size){
	return _mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1);
}
#endif