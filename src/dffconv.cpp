#include <cstdlib>
#include <cstdio>
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
		cerr << "usage: " << argv[0] << " in_dff out_dff " <<
		        "[-c] ([-v version_string] or [-V version]) " <<
		        " ([-d] or [-dd])\n";
		cout << "Flags must be set in the above order.\n";
		cerr << "-c: Clean up geometries; advised for PS2 dffs.\n";
		cerr << "-v: Known versions: GTA3, GTAVC_1, GTAVC_2, GTASA\n";
		cerr << "-V: Set any version you like in hexadecimal.\n";
		cerr << "-d: Dump dff data.\n";
		cerr << "-dd: Dump dff data detailed.\n";
		return 1;
	}
	filename = argv[1];
	ifstream dff(argv[1], ios::binary);
	Clump *clump = new Clump;
	clump->read(dff);
	dff.close();

	ofstream out(argv[2], ios::binary);

	int curArg = 3;

	string arg;
	// -c flag cleans up geometries
	if (argc > curArg) {
		arg = argv[curArg];
		if (arg == "-c") {
			for (uint32 i = 0; i < clump->geometryList.size(); i++)
				clump->geometryList[i].cleanUp();
			curArg++;
		}
	}

	version = VCPC;

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

	clump->write(out);
	out.close();

	bool detailed;
	if (argc > curArg) {
		string flag = argv[curArg];
		if (flag == "-d")
			detailed = false;
		else if (flag == "-dd")
			detailed = true;
		else
			return 0;
		clump->dump(detailed);
	}

	delete clump;

	return 0;
}
