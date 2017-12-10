#include "bufor.h"
#include "monitor.h"
#include "list.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <thread>

#define CYCLES 5

void consumerTask(std::vector<BufferMonitor<char> > &bufMon, ListMonitor &listMon, int M);
void producerTask(std::vector<BufferMonitor<char> > &bufMon, ListMonitor &listMon, int id);
int main(int argc, char *argv[])
{
	int K=1;
	int M=1;
	
	if (argc<=2)
	{
		std::cout << "Podaj 2 argumenty:\n   1) K - ilosc producentow\n   2) M - ilosc buforow\n\n";
		return 0;
	}
	else
	{
		K = atoi(argv[1]);
		M = atoi(argv[2]);
	}

	std::cout << M << std::endl;
	ListMonitor listMon(M);
	std::vector<BufferMonitor<char> > bufMon(M);
	int i=0;
	for (std::vector<BufferMonitor<char> >::iterator it=bufMon.begin(); it!=bufMon.end(); ++it)
	{
		it->setIndex(i);
		i++;
	}
	
	std::srand(std::time(0));
		
	
	std::thread producer[K];
	
	std::thread consumer(consumerTask,std::ref(bufMon),std::ref(listMon),M);
	for (int i=0; i<K; i++)
		producer[i] = std::thread(producerTask,std::ref(bufMon),std::ref(listMon),i);
	
	
	
	for (int i=0; i<K; i++)
		producer[i].join();
	consumer.join();
	
	return 0;
}

void consumerTask(std::vector<BufferMonitor<char> > &bufMon, ListMonitor &listMon, int M)
{
	for (int i=0; i<CYCLES; i++)
	{
		//std::cout << "cons starts\n";
		int buffer = listMon.consume(rand()%(M-1)+1);
		//std::vector<BufferMonitor<char> > &buf = *bufMon;
		char message = bufMon[buffer].get(); 
		//std:: cout << "cons ends, got " << message << " \n";
	}
}

void producerTask(std::vector<BufferMonitor<char> > &bufMon, ListMonitor &listMon, int id)
{
	for (int i=0; i<CYCLES; i++)
	{
		//std::cout << "prod nr " << id << " starts\n";
		int buffer = listMon.produce(id);
		char message = 'p';
	   // std::vector<BufferMonitor<char> > &buf = *bufMon;
		bufMon[buffer].put(message,id); 
		//std::cout << "prod nr " << id << " ends\n";
	}
}
