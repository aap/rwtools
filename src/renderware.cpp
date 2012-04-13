#include <cstdlib>
#include "renderware.h"
using namespace std;

namespace rw {

void HeaderInfo::read(ifstream& rw)
{
	type = readUInt32(rw);
	length = readUInt32(rw);
	version = readUInt32(rw);
}

uint32 HeaderInfo::write(ofstream &rw)
{
	writeUInt32(type, rw);
	writeUInt32(length, rw);
	writeUInt32(version, rw);
	return 3*sizeof(uint32);
}

void ChunkNotFound(CHUNK_TYPE chunk, uint32 address)
{
	cerr << "chunk " << hex << chunk << " not found at 0x";
	cerr << hex << address << endl;
	exit(1);
}

uint32 writeInt8(int8 tmp, ofstream &rw)
{
	rw.write(reinterpret_cast <char *> (&tmp), sizeof(int8));
	return sizeof(int8);
}

uint32 writeUInt8(uint8 tmp, ofstream &rw)
{
	rw.write(reinterpret_cast <char *> (&tmp), sizeof(uint8));
	return sizeof(uint8);
}

uint32 writeInt16(int16 tmp, ofstream &rw)
{
	rw.write(reinterpret_cast <char *> (&tmp), sizeof(int16));
	return sizeof(int16);
}

uint32 writeUInt16(uint16 tmp, ofstream &rw)
{
	rw.write(reinterpret_cast <char *> (&tmp), sizeof(uint16));
	return sizeof(uint16);
}

uint32 writeInt32(int32 tmp, ofstream &rw)
{
	rw.write(reinterpret_cast <char *> (&tmp), sizeof(int32));
	return sizeof(int32);
}

uint32 writeUInt32(uint32 tmp, ofstream &rw)
{
	rw.write(reinterpret_cast <char *> (&tmp), sizeof(uint32));
	return sizeof(uint32);
}

uint32 writeFloat32(float32 tmp, ofstream &rw)
{
	rw.write(reinterpret_cast <char *> (&tmp), sizeof(float32));
	return sizeof(float32);
}

int8 readInt8(ifstream &rw)
{
	int8 tmp;
	rw.read(reinterpret_cast <char *> (&tmp), sizeof(int8));
	return tmp;
}

uint8 readUInt8(ifstream &rw)
{
	uint8 tmp;
	rw.read(reinterpret_cast <char *> (&tmp), sizeof(uint8));
	return tmp;
}

int16 readInt16(ifstream &rw)
{
	int16 tmp;
	rw.read(reinterpret_cast <char *> (&tmp), sizeof(int16));
	return tmp;
}

uint16 readUInt16(ifstream &rw)
{
	uint16 tmp;
	rw.read(reinterpret_cast <char *> (&tmp), sizeof(uint16));
	return tmp;
}

int32 readInt32(ifstream &rw)
{
	int32 tmp;
	rw.read(reinterpret_cast <char *> (&tmp), sizeof(int32));
	return tmp;
}

uint32 readUInt32(ifstream &rw)
{
	uint32 tmp;
	rw.read(reinterpret_cast <char *> (&tmp), sizeof(uint32));
	return tmp;
}

float32 readFloat32(ifstream &rw)
{
	float32 tmp;
	rw.read(reinterpret_cast <char *> (&tmp), sizeof(float32));
	return tmp;
}

}
