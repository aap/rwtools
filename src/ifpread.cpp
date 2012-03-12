#include <cstdlib>
#include "renderware.h"
using namespace std;

#ifdef DEBUG
	#define READ_SECTION(x)\
		fourcc = readUInt32(ifp);\
		if (fourcc != x) {\
			cerr << "error: no x found " << hex << ifp.tellg() << endl;\
			exit(1);\
		}\
		size = readUInt32(ifp);
#endif

namespace rw {

void AnimPackage::read(ifstream &ifp)
{
	uint32 fourcc, size;
	READ_SECTION(ANPK);

	READ_SECTION(INFO);

	uint32 numAnims = readUInt32(ifp);
	animList.resize(numAnims);
	size -= sizeof(uint32);
	size = (size+0x3) & ~0x3;
	char *buf = new char[size];
	ifp.read(buf, size);
	name = buf;
	delete[] buf;

	for (uint32 i = 0; i < numAnims; i++) {
		animList[i].read(ifp);
	}
}

void AnimPackage::clear(void)
{
	animList.resize(0);
	name = "";
}

void Animation::read(ifstream &ifp)
{
	uint32 fourcc, size;

	READ_SECTION(NAME);

	size = (size+0x3) & ~0x3;
	char *buf = new char[size];
	ifp.read(buf, size);
	name = buf;
	delete[] buf;

	READ_SECTION(DGAN);
	uint32 end = size + ifp.tellg();

	READ_SECTION(INFO);

	uint32 numObjs = readUInt32(ifp);
	size -= sizeof(uint32);
	size = (size+0x3) & ~0x3;
	ifp.seekg(size, ios::cur);


	for (uint32 i = 0; i < numObjs && ifp.tellg() < end; i++) {
		objList.resize(objList.size()+1);
		objList[i].read(ifp);
	}
}

void Animation::clear(void)
{
	objList.resize(0);
	name = "";
}

void AnimObj::read(std::ifstream &ifp)
{
	uint32 fourcc, size;

	READ_SECTION(CPAN);

	READ_SECTION(ANIM);
	char *buf = new char[28];
	ifp.read(buf, 28);
	name = buf;
	delete[] buf;
	frames = readInt32(ifp);
	unknown = readInt32(ifp);
	next = readInt32(ifp);
	prev = readInt32(ifp);
	ifp.seekg(size-28-4*sizeof(int32), ios::cur);

	// KFRM
	fourcc = readUInt32(ifp);
	size = readUInt32(ifp);
	uint32 end = size + ifp.tellg();
	for (uint32 i = 0; ifp.tellg() < end; i++) {
		frmList.resize(frmList.size()+1);
		frmList[i].read(fourcc, ifp);
	}
}

void KeyFrame::read(uint32 fourcc, ifstream &ifp)
{
	if (fourcc == KR00) {
		ifp.read((char*)&rot, 4*sizeof(float32));
	} else if (fourcc == KRT0) {
		ifp.read((char*)&rot, 4*sizeof(float32));
		ifp.read((char*)&pos, 3*sizeof(float32));
	} else if (fourcc == KRTS) {
		ifp.read((char*)&rot, 4*sizeof(float32));
		ifp.read((char*)&pos, 3*sizeof(float32));
		ifp.read((char*)&scale, 3*sizeof(float32));
	}
	timeKey = readFloat32(ifp);
	type = fourcc;
}

}
