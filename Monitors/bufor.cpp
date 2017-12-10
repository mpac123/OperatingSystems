#include "bufor.h"

Bufor::Bufor<T>() : size(0) {}

void Bufor::put(T val)
{
	if (size>=MAX_SIZE)
		full.wait();
	buf.push(val);
	size++;
	empty.signal();
}

T Bufor::get()
{
	if (size<=0)
		empty.wait();
	T res = buf.pop();
	size--;
	full.signal();
}
