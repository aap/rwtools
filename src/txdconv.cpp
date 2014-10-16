#include <fstream>
#include <renderware.h>

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
		cerr << "usage: " << argv[0] << " in.txd out.txd " <<
		        "([-v version_string] or [-V version])\n";
		cout << "Flags must be set in the above order.\n";
		cerr << "-v: Known versions: GTA3, GTAVC_1, GTAVC_2, GTASA\n";
		cerr << "-V: Set any version you like in hexadecimal.\n";
		return 1;
	}
	filename = argv[1];
	ifstream rw(argv[1], ios::binary);
	TextureDictionary *txd = new TextureDictionary;
	txd->read(rw);
	rw.close();
	for (uint32 i = 0; i < txd->texList.size(); i++) {
		if (txd->texList[i].platform == PLATFORM_PS2)
			txd->texList[i].convertFromPS2(0x40);
		if (txd->texList[i].platform == PLATFORM_XBOX)
			txd->texList[i].convertFromXbox();
		if (txd->texList[i].dxtCompression)
			txd->texList[i].decompressDxt();
		txd->texList[i].convertTo32Bit();
	}

	ofstream out(argv[2], ios::binary);
	
	int curArg = 3;

	version = VCPC;

	string arg;
	if (argc > curArg + 1) {
		arg = argv[curArg];
		if (arg == "-v") {
			string verstring = argv[curArg+1];
			curArg += 2;
			if (verstring == "GTA3")
				version = GTA3_3;
			else if (verstring == "GTAVC_1")
				version = VCPS2;
			else if (verstring == "GTAVC_2")
				version = VCPC;
			else if (verstring == "GTASA")
				version = SA;
			else
				cout << "unknown version\n";
		}
		if (arg == "-V") {
			sscanf(argv[curArg+1], "%x", &version);
			curArg += 2;
		}
		cout << "writing version: " << hex << version << endl;
	}

	txd->write(out);
	out.close();
	delete txd;
}
