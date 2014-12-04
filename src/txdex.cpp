#include <fstream>
#include <renderware.h>

using namespace std;
using namespace rw;

int
main(int argc, char *argv[])
{
	if (sizeof(uint32) != 4 || sizeof(int32) != 4 ||
	    sizeof(uint16) != 2 || sizeof(int16) != 2 ||
	    sizeof(uint8)  != 1 || sizeof(int8)  != 1 ||
	    sizeof(float32) != 4) {
		cerr << "type size not correct\n";
		return 1;
	}
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " txd\n";
		return 1;
	}
	filename = argv[1];
	ifstream rw(argv[1], ios::binary);
	TextureDictionary txd;
	txd.read(rw);
	rw.close();
	for (uint32 i = 0; i < txd.texList.size(); i++) {
		NativeTexture &t = txd.texList[i];
		cout << i << " " << t.name << " " << t.maskName << " "
			<< " " << t.width[0] << " " << t.height[0] << " "
			<< " " << t.depth << " " << hex << t.rasterFormat << endl;
		if (txd.texList[i].platform == PLATFORM_PS2)
			txd.texList[i].convertFromPS2(0x40);
		if (txd.texList[i].platform == PLATFORM_XBOX)
			txd.texList[i].convertFromXbox();
		if (txd.texList[i].dxtCompression)
			txd.texList[i].decompressDxt();
		txd.texList[i].convertTo32Bit();
		txd.texList[i].writeTGA();
	}
}
