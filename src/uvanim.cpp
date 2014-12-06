#include <renderware.h>
using namespace std;

namespace rw {

/* You can write ONE header per C BLOCk using these macros */
#define SKIP_HEADER()\
	uint32 bytesWritten = 0;\
	uint32 headerPos = rw.tellp();\
	rw.seekp(0x0C, ios::cur);

#define WRITE_HEADER(chunkType)\
	uint32 oldPos = rw.tellp();\
	rw.seekp(headerPos, ios::beg);\
	header.type = (chunkType);\
	header.length = bytesWritten;\
	bytesWritten += header.write(rw);\
	writtenBytesReturn = bytesWritten;\
	rw.seekp(oldPos, ios::beg);

void
UVAnimation::read(istream &rw)
{
	HeaderInfo header;
	READ_HEADER(CHUNK_ANIMANIMATION);
	data.resize(header.length);
	rw.read((char*)&data[0], header.length);
}

uint32
UVAnimation::write(ostream &rw)
{
	HeaderInfo header;
	header.build = version;
	uint32 writtenBytesReturn;

	SKIP_HEADER();
	rw.write((char*)&data[0], data.size());
	bytesWritten += data.size();
	WRITE_HEADER(CHUNK_ANIMANIMATION);
	return bytesWritten;
}

void
UVAnimDict::read(istream &rw)
{
	HeaderInfo header;
	uint32 end;
	header.read(rw);
	end = header.length + rw.tellg();

	READ_HEADER(CHUNK_STRUCT);

	uint32 n = readUInt32(rw);
	animList.resize(n);
	for(uint32 i = 0; i < n; i++)
		animList[i].read(rw);
}

uint32
UVAnimDict::write(ostream &rw)
{
	HeaderInfo header;
	header.build = version;
	uint32 writtenBytesReturn;

	SKIP_HEADER();
	{
		SKIP_HEADER();
		bytesWritten += writeUInt32(animList.size(), rw);
		WRITE_HEADER(CHUNK_STRUCT);
	}
	bytesWritten += writtenBytesReturn;
	for(uint32 i = 0; i < animList.size(); i++)
		bytesWritten += animList[i].write(rw);
	WRITE_HEADER(CHUNK_UVANIMDICT);
	return bytesWritten;
}

void
UVAnimDict::clear(void)
{
	animList.clear();
}

UVAnimDict::~UVAnimDict(void)
{
	animList.clear();
}

}
