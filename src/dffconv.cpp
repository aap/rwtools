#include "renderware.h"
using namespace std;

using namespace rw;

int main(int argc, char *argv[])
{
	if (sizeof(uint32) != 4 || sizeof(int32) != 4 ||
	    sizeof(uint16) != 2 || sizeof(int16) != 2 ||
	    sizeof(uint8)  != 1 || sizeof(int8)  != 1 ||
	    sizeof(float32) != 4) {
		cerr << "type size not correct\n";
	}
	if (argc < 3) {
		cerr << "usage: " << argv[0] << " in_dff out_dff\n";
		return 1;
	}
	filename = argv[1];
	ifstream dff(argv[1], ios::binary);
	Clump clump;
	clump.read(dff);
	dff.close();

	for (uint32 i = 0; i < clump.geometryList.size(); i++)
		clump.geometryList[i].cleanUp();

	version = VCPC;
	ofstream out(argv[2], ios::binary);
	clump.write(out);
	out.close();

	return 0;
}
