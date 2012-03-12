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
	if (argc < 2) {
		cerr << "usage: " << argv[0] << " in_ifp\n";
		return 1;
	}
	filename = argv[1];
	ifstream ifp(argv[1], ios::binary);
	AnimPackage anpk;
	anpk.read(ifp);
	ifp.close();

	for (uint32 i = 0; i < anpk.animList.size(); i++) {
		Animation &a = anpk.animList[i];
		cout << a.name << endl;
	}

	if (argc < 3)
		return 0;

	cout << endl << endl;

	string name = argv[2];

	uint32 ai;
	for (ai = 0; ai < anpk.animList.size(); ai++) {
		Animation &a = anpk.animList[ai];
		if (a.name == name)
			break;
	}
	if (ai >= anpk.animList.size())
		return 0;
	Animation &a = anpk.animList[ai];
	cout << a.name << endl;

	for (uint32 i = 0; i < a.objList.size(); i++) {
		AnimObj &ao = a.objList[i];
		cout << i << " "
		     << ao.frames << " "
		     << ao.unknown << " "
		     << ao.next << " "
		     << ao.prev << "\t"
		     << ao.name << "\n";
		for (uint32 j = 0; j < ao.frmList.size(); j++) {
			KeyFrame &kf = ao.frmList[j];
/*
			cout << kf.rot[0] << " "
			     << kf.rot[1] << " "
			     << kf.rot[2] << " "
			     << kf.rot[3] << "\n";
			if (kf.type == KRT0 || kf.type == KRTS) {
				cout << kf.pos[0] << " "
				     << kf.pos[1] << " "
				     << kf.pos[2] << "\n";
			}
			if (kf.type == KRTS) {
				cout << kf.scale[0] << " "
				     << kf.scale[1] << " "
				     << kf.scale[2] << "\n";
			}
*/
			cout << kf.timeKey << endl;
		}
	}

	return 0;
}
