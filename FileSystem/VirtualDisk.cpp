#include "VirtualDisk.h"

using namespace std;
using namespace boost;
using namespace filesystem;

VirtualDisk::VirtualDisk(string name):
  name(name) {}

VirtualDisk::~VirtualDisk() { 
    /*Zapisanie tablicy bitów w pliku */
    fstream output;
    output.open(name.c_str(), ios::in|ios::out|ios::binary);
    output.write(bitmap,BLOCK_SIZE*BMAP_SIZE);
    /* Zapisanie tablicy inodów w pliku */
    buffer mem[INODES_SIZE];
    for (size_t i=0; i<INODES_SIZE; i++) {
        for (size_t j=0; j<BLOCK_SIZE; j++) {
            mem[i][j]='\0';
        }
    }
    unsigned index=0;
    for (size_t i=0; i<INODES_SIZE; i++) {
        for (size_t j=0; j<BLOCK_SIZE-sizeof(inode); j+=sizeof(inode), index++) {
            inode *ptri = reinterpret_cast<inode*>(mem[i] + j);
            if(index < inodes.size()) {
                *ptri = inodes[index];
            }
        }
    }
    for(size_t i=0; i < INODES_SIZE; ++i) {
        output.write(mem[i], BLOCK_SIZE);
    }
    output.close();
}

void VirtualDisk::CreateFile(unsigned _size) {
    if(_size <= ((BMAP_SIZE+INODES_SIZE)*BLOCK_SIZE/1024)) {
        throw DiscTooSmall();
    }
    if(_size > (BLOCK_SIZE*BLOCK_SIZE/1024)) {
        throw DiscTooBig();
    }
    size = _size;
    N=(int)(size*1024/BLOCK_SIZE)-BMAP_SIZE-INODES_SIZE;
    char mem[BLOCK_SIZE];
    for(size_t i=0; i<BLOCK_SIZE; ++i) {
        mem[i] = '\0';
    }
    char fullmem[BLOCK_SIZE];
    for(size_t i=0; i<BLOCK_SIZE; ++i) {
        fullmem[i] = '0';
    }
    fstream file;
    file.open(name.c_str(), ios::out|ios::binary);
    for(size_t i=0; i<BMAP_SIZE; i++) {
        file.write(fullmem, BLOCK_SIZE);
    }
    for(size_t i=0; i<N+INODES_SIZE; i++) {
        file.write(mem, BLOCK_SIZE);
    }
    for (size_t i=0; i<BMAP_SIZE*BLOCK_SIZE; i++)
        bitmap[i]='0';
    file.close();
}

void VirtualDisk::OpenFile() {
    ifstream file;
    file.open(name.c_str(), ios::in|ios::binary);
    size = filesystem::file_size(name);
    N=(int)(size/BLOCK_SIZE-BMAP_SIZE-INODES_SIZE);
    file.read(bitmap, BLOCK_SIZE*BMAP_SIZE);
    buffer mem[INODES_SIZE];
    for (size_t i=0; i<INODES_SIZE; i++) {
        file.read(mem[i],BLOCK_SIZE);
    }
    int x = int(BLOCK_SIZE/sizeof(inode));
    for (size_t i=0; i<N; i++) {
        inode new_inode = *reinterpret_cast<inode*>(mem[int(i/x)] + ((i%x)*sizeof(inode)));
        inodes.push_back(new_inode);
    }
    file.close();
}

unsigned VirtualDisk::CountFreeBlocks() {
    unsigned freeSpace = 0;
    for (size_t i=0; i<N; i++) {
        if (bitmap[i]!=0) freeSpace++;
    }
    return freeSpace;
}

int VirtualDisk::FindInodeByName(std::string s) {
    for (size_t i=0; i<N; i++) {
        if (bitmap[i]=='2') {
            if (inodes[i].name==s)
                return i;
        }
    }
    return -1;
}

void VirtualDisk::Rename(std::string old_name, std::string new_name)
{
    if(FindInodeByName(new_name)!=-1) {
        throw FileNameNotUnique();
    }
    int index=0;
    if((index=FindInodeByName(old_name))==-1) {
        throw FileDoesntExist();
    }
    do {
        strcpy(inodes[index].name,new_name.c_str());
        index=inodes[index].next;
    } while(index!=-1);
}

void VirtualDisk::RemoveFile(std::string file)
{
    int index;
    if ((index=FindInodeByName(file))==-1) {
        throw FileDoesntExist();
    }
    do {
        bitmap[index]='0';
        index=inodes[index].next;
    } while(index!=-1);
}

void VirtualDisk::Map() {
    std::cout << "bitmap: ";
    for (size_t i=0; i<N; i++)
        std::cout << bitmap[i];
    std::cout << "\n--------------Mapa zajętości---------------->" << std::endl;
    std::cout << "adres\tnazwa\trozmiar\n";
    for (size_t i=0; i<N; i++) {
        if (bitmap[i]!='0')
            std::cout << i*BLOCK_SIZE << "\t" << inodes[i].name << "\t" << inodes[i].dataSize << std::endl;
        else
            std::cout << i*BLOCK_SIZE << "\tBLOK PUSTY\n";
    }
}

void VirtualDisk::ListFiles()
{
    for (size_t i=0; i<N; i++) {
        if (bitmap[i]=='2') {
            std::cout << inodes[i].name <<"\t";
        }
    }
    std::cout<<endl;
}

void VirtualDisk::CopyFromDisk(std::string dest)
{
    int index;
    if ((index=FindInodeByName(dest))==-1) {
        throw FileDoesntExist();
    }
    /* Zebranie danych o pliku na dysku */
    unsigned src_blocks=0;
    unsigned last_data=0;
    do {
        src_blocks++;
        last_data=inodes[index].dataSize;
        index=inodes[index].next;
    } while(index!=-1);
    /* Pobranie danych z pliku na dysku */
    buffer buf[src_blocks];
    fstream src;
    src.open(name.c_str(), ios::in|ios::binary);
    index=FindInodeByName(dest);
    
    for (size_t i=0; i<src_blocks-1; i++) {
        src.seekg((BMAP_SIZE+INODES_SIZE+index)*BLOCK_SIZE);
        src.read(buf[i],BLOCK_SIZE);
        index=inodes[index].next;
    }
    src.seekg((BMAP_SIZE+INODES_SIZE+index)*BLOCK_SIZE);
    src.read(buf[src_blocks-1],last_data);
    src.close();
    /* Zapisanie danych do pliku poza dyskiem */
    fstream file;
    file.open(dest.c_str(), ios::out|ios::binary);
    for (size_t i=0; i<src_blocks-1; i++) {
        file.write(buf[i],BLOCK_SIZE);
    }
    file.write(buf[src_blocks-1],last_data);
    file.close();
}


void VirtualDisk::CopyToDisk(std::string file)
{ 
    if(FindInodeByName(file) != -1) {
        throw FileNameNotUnique();
    }
    unsigned file_size = filesystem::file_size(file);
    unsigned file_blocks = (file_size - 1)/BLOCK_SIZE + 1; 
    unsigned last_size = BLOCK_SIZE-(file_blocks*BLOCK_SIZE-file_size);
    if(file_blocks > CountFreeBlocks()) {
        throw NotEnoughSpace(); 
    }
    /* Pobranie indeksow bloków w ktorych zapiszemy plik */
    int j=0;
    int indeces[file_blocks];
    for (size_t i=0; i<file_blocks; i++) {
        while (bitmap[j]!='0') j++;
        indeces[i]=j;
        j++;
    }
    /* Zczytanie zawartosci pliku */
    buffer buffers[file_blocks];
    ifstream input;
    input.open(file.c_str(), ios::in|ios::binary);
    for(size_t i=0; i < file_blocks-1; ++i) {
        input.read(buffers[i], BLOCK_SIZE);
    }
    input.read(buffers[file_blocks-1],last_size);
    input.close();
    /* Wczytanie zawartosci pliku na dysk */
    fstream output;
    output.open(name.c_str(), ios::in|ios::out|ios::binary);
    for(size_t i=0; i < file_blocks-1; ++i) {
        output.seekp((BMAP_SIZE+INODES_SIZE+indeces[i])*BLOCK_SIZE);
        output.write(buffers[i], BLOCK_SIZE);
    }
    output.write(buffers[file_blocks-1],last_size);
    output.close();
    /* Modyfikacja bitmapy */
    bitmap[indeces[0]]='2';
    for (size_t i=1; i<file_blocks; i++) {
        bitmap[indeces[i]]='1';
    }
    /* Modyfikacja tablicy inodów */
    for (size_t i=0; i<file_blocks-1; i++) {
        strcpy(inodes[indeces[i]].name, file.c_str());
        inodes[indeces[i]].dataSize=BLOCK_SIZE;
        inodes[indeces[i]].next=indeces[i+1];
    }
    strcpy(inodes[indeces[file_blocks-1]].name,file.c_str());
    inodes[indeces[file_blocks-1]].dataSize=last_size;
    inodes[indeces[file_blocks-1]].next=-1;
}

