#include "VirtualDisk.h"

using namespace std;
using namespace boost; 
using namespace program_options; 
using namespace filesystem;

int main(int argc, char *argv[]) try
{
	options_description desc ("Dozwolone operacje"); 
	desc.add_options() 
	("file,f", value<string>(), "nazwa pliku z wirtualnym dyskiem (wymagana dla wszystkich opcji oprocz -h)")
	("create,c", value<unsigned>()->required(), "stworz nowy (pusty) wirtualny dysk o zadanym rozmiarze (w kB)") 
	("upload,u", value<string>()->required(), "skopiuj plik o nazwie arg na wirtualny dysk") 
	("download,d", value<string>()->required(), "skopiuj plik o nazwie arg z wirtualnego dysku")
	("remove,r", value<string>()->required(), "usun plik o nazwie arg z wirtualnego dysku")
	("move,m", value<vector<string> >()->required()->multitoken(), "zmien nazwe pliku znajdujacego sie na wirtualnym dysku\n"
	  "Skladnia: \n"
	  "  -m stara_nazwa nowa_nazwa"
	)
	("list,l", "wyswietl liste plikow")
	("map,M", "wyswietl mape wolnych i zajetych blokow")
	("help,h", "wyswietl pomoc")
	;

	variables_map vm; 
	store(command_line_parser(argc, argv).options(desc).run(), vm); 

	if(vm.count("help") || !vm.count("file")) { 
		std::cout << "Uzycie: " << argv[0] << " -f plik operacja (arg) \n" << desc << std::endl;
		return 0; 
	}
	string file = vm["file"].as<string>();
	VirtualDisk FD(file);
	if(vm.count("create")) {
		unsigned size = vm["create"].as<unsigned>();
		FD.CreateFile(size);
	}
	else {
		FD.OpenFile();
		if(vm.count("upload"))
		{
		  string source_file = vm["upload"].as<string>();
		  FD.CopyToDisk(source_file);
		}
		else if(vm.count("download")) {
		  string dest_file = vm["download"].as<string>();
		  FD.CopyFromDisk(dest_file);
		}
		else if(vm.count("move")) {
		  string old_name = vm["move"].as<vector<string> >()[0];
		  string new_name = vm["move"].as<vector<string> >()[1];
		  FD.Rename(old_name, new_name);
		}
		else if(vm.count("remove")) {
		  string file_name = vm["remove"].as<string>();
		  FD.RemoveFile(file_name);
		}
		else if(vm.count("list"))
		{
		  FD.ListFiles();
		}
		else if(vm.count("map"))
		{
		  FD.Map();
		}
	}
	return 0;
}
catch(std::exception &e)
{
  cout << e.what() << endl;
  cout << "Aby uzyskac pomoc, wpisz:\n  " << argv[0] <<" -h" << endl;
  return -1;
}
