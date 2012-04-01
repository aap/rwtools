#include <cstdlib>
#include "renderware.h"
using namespace std;

#ifdef DEBUG
	#define READ_SECTION(x)\
		fourcc = readUInt32(ifp);\
		if (fourcc != x) {\
			cerr << "error: no " << hex << x << " found " << hex << ifp.tellg() << endl;\
			exit(1);\
		}\
		size = readUInt32(ifp);
#else
	#define READ_SECTION(x)\
		fourcc = readUInt32(ifp);\
		size = readUInt32(ifp);
#endif

namespace rw {

void AnimPackage::read(ifstream &ifp)
{
	uint32 fourcc, size;
//	READ_SECTION(ANPK);
	fourcc = readUInt32(ifp);
	size = readUInt32(ifp);

	if (fourcc == ANPK) {
		READ_SECTION(INFO);

		uint32 numAnims = readUInt32(ifp);
		animList.resize(numAnims);
		size -= sizeof(uint32);
		size = (size+0x3) & ~0x3;
		char *buf = new char[size];
		ifp.read(buf, size);
		name = buf;
		delete[] buf;

		for (uint32 i = 0; i < numAnims; i++)
			animList[i].read_1(ifp);
	} else if (fourcc == ANP3) {
		char buf[24];
		ifp.read(buf, 24);
		name = buf;
		uint32 numAnims = readUInt32(ifp);
		animList.resize(numAnims);

		for (uint32 i = 0; i < numAnims; i++)
			animList[i].read_3(ifp);
	} else {
		cout << "no known ifp file\n";
	}
}

void AnimPackage::clear(void)
{
	animList.resize(0);
	name = "";
}

void Animation::read_1(ifstream &ifp)
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
		objList[i].read_1(ifp);
	}
}

void Animation::read_3(ifstream &ifp)
{
	char buf[24];
	ifp.read(buf, 24);
	name = buf;
	uint32 numObjs = readUInt32(ifp);
	uint32 frameSize = readUInt32(ifp);
	ifp.seekg(4, ios::cur);

	objList.resize(numObjs);
	for (uint32 i = 0; i < numObjs; i++)
		objList[i].read_3(ifp);
}

void Animation::clear(void)
{
	objList.resize(0);
	name = "";
}

void AnimObj::read_1(std::ifstream &ifp)
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
		frmList[i].read_1(fourcc, ifp);
	}
}

void AnimObj::read_3(std::ifstream &ifp)
{
	char buf[24];
	ifp.read(buf, 24);
	name = buf;
	uint32 frmType = readUInt32(ifp);
	frames = readUInt32(ifp);
	uint32 boneId = readUInt32(ifp);

	frmList.resize(frames);
	for (uint32 i = 0; i < frames; i++)
		frmList[i].read_3(ifp, frmType);
}

void KeyFrame::read_1(uint32 fourcc, ifstream &ifp)
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

void KeyFrame::read_3(ifstream &ifp, uint32 frmType)
{
	int16 data[5];
	ifp.read((char*)&data, 5*sizeof(int16));
	rot[0] = -data[0]/4096.0f;
	rot[1] = -data[1]/4096.0f;
	rot[2] = -data[2]/4096.0f;
	rot[3] = data[3]/4096.0f;
	timeKey = data[4]/60.0f;
	type = KR00;

	if (frmType == 4) {
		ifp.read((char*)&data, 3*sizeof(int16));
		pos[0] = data[0]/4096.0f;
		pos[1] = data[1]/4096.0f;
		pos[2] = data[2]/4096.0f;
		type = KRT0;
	}
}

}
