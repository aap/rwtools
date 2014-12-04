#include <cstring>

#include <renderware.h>

using namespace std;

namespace rw {

/* You can write ONE header per C BLOCK using these macros */
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

uint32 TextureDictionary::write(ostream &rw)
{
	HeaderInfo header;
	header.build = version;
	uint32 writtenBytesReturn;

	/*
	 * writtenBytesReturn will always contain the number of bytes
	 * written in a sub-block, it is used like a return value.
	 */

	// Texture Dictionary
	SKIP_HEADER();

	// Struct
	{
		SKIP_HEADER();
		bytesWritten += writeUInt16(texList.size(), rw);
		// TODO, wtf is that?
		bytesWritten += writeUInt16(0, rw);
		WRITE_HEADER(CHUNK_STRUCT);
	}
	bytesWritten += writtenBytesReturn;

	// Texture Natives
	for (uint32 i = 0; i < texList.size(); i++) {
		if (texList[i].platform == PLATFORM_D3D8 ||
		    texList[i].platform == PLATFORM_D3D9) {
			bytesWritten += texList[i].writeD3d(rw);
		} else {
			cerr << "can't write platform " <<
				texList[i].platform << endl;
		}
	}

	// Extension
	{
		SKIP_HEADER();
		WRITE_HEADER(CHUNK_EXTENSION);
	}
	bytesWritten += writtenBytesReturn;

	WRITE_HEADER(CHUNK_TEXDICTIONARY);

	return bytesWritten;
}

uint32 NativeTexture::writeD3d(ostream &rw)
{
	HeaderInfo header;
	header.build = version;
	uint32 writtenBytesReturn;

	if (platform != PLATFORM_D3D8 &&
	    platform != PLATFORM_D3D9)
		return 0;

	// Texture Native
	SKIP_HEADER();

	// Struct
	{
		SKIP_HEADER();
		bytesWritten += writeUInt32(platform, rw);
		bytesWritten += writeUInt32(filterFlags, rw);

		char buffer[32];
		strncpy(buffer, name.c_str(), 32);
		rw.write(buffer, 32);
		strncpy(buffer, maskName.c_str(), 32);
		rw.write(buffer, 32);
		bytesWritten += 2*32;

		bytesWritten += writeUInt32(rasterFormat, rw);
/*
if(rasterFormat == RASTER_8888 && !hasAlpha ||
   rasterFormat == RASTER_888 && hasAlpha)
	cout << "mismatch " << hex << rasterFormat << endl;
*/
		if (platform == PLATFORM_D3D8) {
			bytesWritten += writeUInt32(hasAlpha, rw);
		} else {
			if (dxtCompression) {
				char fourcc[5] = "DXT0";
				fourcc[3] += dxtCompression;
				rw.write(fourcc, 4);
				bytesWritten += 4;
			} else {
				// 0x15 or 0x16
				bytesWritten += writeUInt32(0x16-hasAlpha, rw);
			}
		}
		bytesWritten += writeUInt16(width[0], rw);
		bytesWritten += writeUInt16(height[0], rw);
		bytesWritten += writeUInt8(depth, rw);
		bytesWritten += writeUInt8(mipmapCount, rw);
		bytesWritten += writeUInt8(0x4, rw);
		if (platform == PLATFORM_D3D8)
			bytesWritten += writeUInt8(dxtCompression, rw);
		else
			bytesWritten += writeUInt8(
				(dxtCompression ? 8 : 0) | hasAlpha, rw);

		/* Palette */
		if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4) {
			uint32 paletteSize = 1 << depth;
			rw.write(reinterpret_cast <char *> (palette),
				paletteSize*4*sizeof(uint8));
			bytesWritten += paletteSize*4*sizeof(uint8);
		}

		/* Texels */
		for (uint32 i = 0; i < mipmapCount; i++) {
/*
			uint32 dataSize = width[i]*height[i];
			if (!dxtCompression)
				dataSize *= depth/8;
			else if (dxtCompression == 1)
				dataSize /= 2;
*/
			uint32 dataSize = dataSizes[i];

			bytesWritten += writeUInt32(dataSize, rw);
			rw.write(reinterpret_cast <char *> (&texels[i][0]),
				dataSize*sizeof(uint8));
			bytesWritten += dataSize*sizeof(uint8);
		}

		WRITE_HEADER(CHUNK_STRUCT);
	}
	bytesWritten += writtenBytesReturn;

	// Extension
	{
		SKIP_HEADER();
		WRITE_HEADER(CHUNK_EXTENSION);
	}
	bytesWritten += writtenBytesReturn;

	WRITE_HEADER(CHUNK_TEXTURENATIVE);

	return bytesWritten;
}

}
