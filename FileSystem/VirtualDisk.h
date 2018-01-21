#ifndef _VIRTUALDISK_H_
#define _VIRTUALDISK_H_

#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

class VirtualDisk {
	static const unsigned BLOCK_SIZE = 256; // B
	static const unsigned BMAP_SIZE = 1;
	static const unsigned INODES_SIZE = 43;
	char bitmap[BMAP_SIZE*BLOCK_SIZE];
	char* inode_buf;
	unsigned N;							/* number of blocks of data */
	unsigned* start_of_data;			/* NR_OF_BLOCKS*(1+sizeof(inode)*8) */
	struct inode {
		char name[32];
		unsigned dataSize;
		int next;
	};
	std::vector<inode> inodes;
	typedef char buffer[BLOCK_SIZE];
	
	std::string name;
	unsigned size; 

	/*unsigned alloc(unsigned);
	void defragment();
	void close();
	inode *find_inode_by_name(std::string);
	static int cmp_inodes(inode, inode);*/
	unsigned CountFreeBlocks();
	

public:
	int FindInodeByName(std::string s);
	VirtualDisk(std::string name);
	~VirtualDisk();
	void CreateFile(unsigned);
	void OpenFile();
	void CopyToDisk(std::string);
	void CopyFromDisk(std::string);
	void Rename(std::string,std::string);
	void RemoveFile(std::string);
	void ListFiles();
	/*void download_file(std::string);
	void rename_file(std::string, std::string);
	void remove_file(std::string);
	void list_files();*/
	void Map();
	/*void open();
	static std::string get_title()
	{
	return std::string
	(
	  "\033[1mWirtualny system plikow\n\033[m"
	  "Autor: Michal Artur Krawczak\n"
	);
	}*/
	class NotEnoughSpace: public std::exception
	{
	const char* what() const throw() 
	{ return "Brak miejsca na wirtualnym dysku."; }
	};

	class FileDoesntExist: public std::exception
	{
	const char* what() const throw() 
	{ return "Nie istnieje plik o podanej nazwie.\nAby przejrzec liste plikow, uzyj polecenia -l."; }
	};

	class FileNameNotUnique: public std::exception
	{
	const char* what() const throw() 
	{ return "Plik o takiej samej nazwie juz istnieje na wirtualnym dysku.\nAby zmienic nazwe pliku, uzyj polecenia -m."; }
	};

	class DiscTooSmall: public std::exception
	{
	const char* what() const throw() 
	{ return "Podany rozmiar wirtualnego dysku jest za maly."; }
	};
	
	class DiscTooBig: public std::exception
	{
	const char* what() const throw() 
	{ return "Podany rozmiar wirtualnego dysku jest za duzy.."; }
	};

	class CantOpenFile: public std::exception
	{
	const char* what() const throw() 
	{ return "Nie mozna otworzyc pliku."; }
	};
};

#endif
