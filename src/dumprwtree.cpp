#include <fstream>
#include <renderware.h>

using namespace std;
using namespace rw;

void
readsection(HeaderInfo &rwh, uint32 build, uint32 level, istream &rw)
{
	for(uint32 i = 0; i < level; i++)
		cout << "  ";
	string name = getChunkName(rwh.type);
	cout << name << " (" << hex << rwh.length << " bytes @ 0x" <<
	  hex << rw.tellg()-(streampos)12 << "/0x" << hex << rw.tellg() <<
	  ") - [0x" << hex << rwh.type << "] " << rwh.build << endl;

	streampos end = rw.tellg() + (streampos)rwh.length;

	while(rw.tellg() < end){
		HeaderInfo newrwh;
		newrwh.read(rw);

		if(newrwh.build == build){
			readsection(newrwh, build, level+1, rw);

			/* Native Data PLG has the wrong length */
			if(rwh.type == 0x510)
				rw.seekg(end, ios::beg);
		}else{
			streampos sp = rw.tellg()+(streampos)rwh.length;
			sp -= 12;
			rw.seekg(sp, ios::beg);
			break;
		}
	}
}

int
main(int argc, char *argv[])
{
	if(sizeof(uint32) != 4 || sizeof(int32) != 4 ||
	    sizeof(uint16) != 2 || sizeof(int16) != 2 ||
	    sizeof(uint8)  != 1 || sizeof(int8)  != 1 ||
	    sizeof(float32) != 4){
		cerr << "type size not correct\n";
		return 1;
	}
	if(argc < 2){
		cerr << "need a file\n";
		return 1;
	}
	filename = argv[1];
	ifstream rw(argv[1], ios::binary);
	HeaderInfo rwh;
	int build = 0, vers = 0;
	while(rwh.read(rw) && rwh.type != CHUNK_NAOBJECT){
		build = rwh.build;
		vers = rwh.version;
		readsection(rwh, build, 0, rw);
	}
	cout << "RW build: " << hex << build <<
	        " version: " << hex << vers << " " << filename << endl;
	rw.close();
	return 0;
}
