#ifndef _LIST_H_
#define _LIST_H_

#include <list>
#include <iostream>
#include"monitor.h"

class ListMonitor : private Monitor
{
public:
	ListMonitor(int size)
	{
			for (int i=0; i<size; i++)
				bufferIndeces.push_back(i);
	};
	
	int consume(int los) 
	{
		enter();
		//std::cout << "Consumer should eat from " << los << std::endl;	
		std::list<int>::iterator it=bufferIndeces.begin();
		for (int i=0; i<(los); i++)
		{
			it++;
		}
		int res=(*it);	
		bufferIndeces.splice(bufferIndeces.begin(),bufferIndeces,it);
		//std::cout << "consumer eats from " << res << std::endl << std::flush;
		
		leave();
		//std::cout << "A na poczatu jest teraz " << *(bufferIndeces.begin()) << std::endl;
		return res;
	};
	
	
	int produce(int id)
	{
		enter();
		std::list<int>::iterator it=bufferIndeces.begin();
		int res = (*it);
		bufferIndeces.splice(bufferIndeces.end(),bufferIndeces,it);
		//std::cout << "producer " << id << " produces to " << res << std::endl << std::flush;
		leave();
		
		return res;	
	};
	
private:
	std::list<int> bufferIndeces;
};

#endif
