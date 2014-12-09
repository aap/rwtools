#include <cstdlib>
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
	        " [-9] [-v version_string] [-V version] " <<
	        " in.txd out.txd\n";
	cerr << "-9: Write Direct3D 9 TXD (for San Andreas).\n";
	cerr << "-v: Known versions: GTA3, GTAVC_1, GTAVC_2, GTASA\n";
	cerr << "-V: Set any version you like in hexadecimal.\n";
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

	int dx9 = 0;
	version = VCPC;
	string verstring;
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
		else{
			cerr << "unknown version\n";
			return 1;
		}
		break;
	case 'V':
		sscanf(EARGF(usage()), "%x", &version);
		break;
	case '9':
		dx9++;
		break;
	default:
		usage();
	}ARGEND;

	if(argc < 2)
		usage();

	filename = argv[0];
	ifstream rw(argv[0], ios::binary);
	TextureDictionary *txd = new TextureDictionary;
	txd->read(rw);
	rw.close();
	for(uint32 i = 0; i < txd->texList.size(); i++){
		if(txd->texList[i].platform == PLATFORM_PS2)
			txd->texList[i].convertFromPS2(0x40);
		if(txd->texList[i].platform == PLATFORM_XBOX)
			txd->texList[i].convertFromXbox();
		if(txd->texList[i].dxtCompression)
			txd->texList[i].decompressDxt();
		txd->texList[i].convertTo32Bit();
	}

	ofstream out(argv[1], ios::binary);
	
	if(dx9)
		for(uint32 i = 0; i < txd->texList.size(); i++){
			txd->texList[i].platform = PLATFORM_D3D9;
//			txd->texList[i].filterFlags = 0x1101;
		}

	txd->write(out);
	out.close();
	delete txd;
}
