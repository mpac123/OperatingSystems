#ifndef _BUFOR_H_
#define _BUFOR_H_

#include <unistd.h>
#include <queue>
#include <stdio.h>
#include <iostream>
#include"monitor.h"

template <typename T>
class BufferMonitor : private Monitor
{
public:
	BufferMonitor<T>() : size(0) {};
	
	void put(T val, int id)
	{
		//printf("B%d P%d[%d] before monitor\n",index,id,size);
		enter();
		//printf("B%d P%d[%d] entered monitor\n",index,id,size);
		if (size == MAX_SIZE)
			wait(full);
		buf.push(val);
		size++;
		//std::cout << "B" << index << ": P" << id << "[" << size << "]\n" << std::flush;
		printf("B%d P%d[%d]\n",index,id,size);
		fflush(stdout);
		if (size == 1)
			signal(empty);
		
		leave();
		
	};
	
	T get()
	{
		//std::cout << "Cons is before monitor\n";
		enter();
		//std::cout << "COns is in monitor\n";
		if (size == 0)
			wait(empty);
		//std::cout << "Cons doesn't wait anymore\n";
		T res = buf.front();
		buf.pop();
		size--;
		//std::cout << "B" << index << ": C[" << size << "]\n" << std::flush;
		printf("B%d C[%d]\n",index,size);
		fflush(stdout);
		if (size == MAX_SIZE-1)
			signal(full);
		
		leave();
		return res;
	};
	
	void setIndex(int id)
	{
		index=id;
	}
	
private:
	static const int MAX_SIZE=10;
	int size;
	int index;
	std::queue<T> buf;
	Condition full, empty;
};

#endif
