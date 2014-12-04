#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <renderware.h>
#include "args.h"

using namespace std;
using namespace rw;

char *argv0;

void
usage(void)
{
	cerr << "usage: " << argv0 <<
	        " [-d] [-dd] " <<
	        " [-c] [-v version_string] [-V version] " <<
	        " in_dff out_dff\n";
	cerr << "-c: Clean up geometries; advised for PS2 dffs.\n";
	cerr << "-v: Known versions: GTA3, GTAVC_1, GTAVC_2, GTASA\n";
	cerr << "-V: Set any version you like in hexadecimal.\n";
	cerr << "-d: Dump dff data.\n";
	cerr << "-dd: Dump dff data detailed.\n";
	exit(1);
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

	string verstring;
	version = VCPC;
	int cleanflag = 0;
	int dumpflag = 0;
	ARGBEGIN{
	case 'v':
		verstring = EARGF(usage());
		if(verstring == "GTA3")
			version = GTA3_3;
		else if(verstring == "GTAVC_1")
			version = VCPS2;
		else if(verstring == "GTAVC_2")
			version = VCPC;
		else if(verstring == "GTASA")
			version = SA;
		else
			cout << "unknown version\n";
		break;
	case 'V':
		sscanf(EARGF(usage()), "%x", &version);
		break;
	case 'c':
		cleanflag++;
		break;
	case 'd':
		dumpflag++;
		break;
	default:
		usage();
	}ARGEND;

	if(argc < 2)
		usage();

	filename = argv[0];
	ifstream in(argv[0], ios::binary);
	if(in.fail()){
		cerr << "cannot open " << argv[0] << endl;
		return 1;
	}

	ofstream out(argv[1], ios::binary);
	if(out.fail()){
		cerr << "cannot open " << argv[1] << endl;
		return 1;
	}

	Clump *clump = new Clump;
	clump->read(in);
	in.close();

	if(cleanflag)
		for (uint32 i = 0; i < clump->geometryList.size(); i++)
			clump->geometryList[i].cleanUp();

	if(dumpflag)
		clump->dump(dumpflag > 1);

	clump->write(out);
	delete clump;
	out.close();

	return 0;
}
