#include <cstring>
#include <cstdlib>
#include <fstream>
#include <renderware.h>

using namespace std;

namespace rw {

static void unclut(uint8 *texels, uint32 width, uint32 height);
static void unswizzle8(uint8 *texels, uint8 *rawIndices,
                       uint32 width, uint32 height);

/*
 * Texture Dictionary
 */

void TextureDictionary::read(istream &rw)
{
	HeaderInfo header;

	header.read(rw);
	if (header.type != CHUNK_TEXDICTIONARY)
		return;

	READ_HEADER(CHUNK_STRUCT);
	uint32 textureCount = readUInt16(rw);
	rw.seekg(2, ios::cur);
	texList.resize(textureCount);

	for (uint32 i = 0; i < textureCount; i++) {
		READ_HEADER(CHUNK_TEXTURENATIVE);
		rw.seekg(0x0c, ios::cur);
		texList[i].platform = readUInt32(rw);
		rw.seekg(-0x10, ios::cur);

		if (texList[i].platform == PLATFORM_XBOX) {
			texList[i].readXbox(rw);
		} else if (texList[i].platform == PLATFORM_D3D8 ||
		           texList[i].platform == PLATFORM_D3D9) {
			texList[i].readD3d(rw);
		} else if (texList[i].platform == PLATFORM_PS2FOURCC) {
			texList[i].platform = PLATFORM_PS2;
			texList[i].readPs2(rw);
		}

		READ_HEADER(CHUNK_EXTENSION);
		uint32 end = header.length;
		end += rw.tellg();
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
			case CHUNK_SKYMIPMAP:
				rw.seekg(4, ios::cur);
				break;
			default:
				rw.seekg(header.length, ios::cur);
				break;
			}
		}
	}
}

void TextureDictionary::clear(void)
{
	texList.clear();
}

TextureDictionary::~TextureDictionary(void)
{
	texList.clear();
}

/*
 * Native Texture
 */

void NativeTexture::readD3d(istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	uint32 end = rw.tellg();
	end += header.length;

	uint32 platform = readUInt32(rw);
	// improve error handling
	if (platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9)
		return;

	filterFlags = readUInt32(rw);

	char buffer[32];
	rw.read(buffer, 32);
	name = buffer;
	rw.read(buffer, 32);
	maskName = buffer;

	rasterFormat = readUInt32(rw);
//cout << hex << rasterFormat << " ";

	hasAlpha = false;
	char fourcc[5];
	fourcc[4] = 0;
	if (platform == PLATFORM_D3D9) {
		rw.read(fourcc, 4*sizeof(char));
	} else {
		hasAlpha = readUInt32(rw);
//cout << hasAlpha << " ";
	}

	width.push_back(readUInt16(rw));
	height.push_back(readUInt16(rw));
	depth = readUInt8(rw);
	mipmapCount = readUInt8(rw);
//cout << dec << mipmapCount << " ";
	rw.seekg(sizeof(int8), ios::cur); // raster type (always 4)
	dxtCompression = readUInt8(rw);
//cout << dxtCompression << " ";

	if (platform == PLATFORM_D3D9) {
		hasAlpha = dxtCompression & 0x1;
		if (dxtCompression & 0x8)
			dxtCompression = fourcc[3] - '0';
		else
			dxtCompression = 0;
//cout << fourcc << " ";
	}
//	cout << hasAlpha << " " << maskName << " " << name << endl;


	if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4) {
		paletteSize = (rasterFormat & RASTER_PAL8) ? 0x100 : 0x10;
		palette = new uint8[paletteSize*4*sizeof(uint8)];
		rw.read(reinterpret_cast <char *> (palette),
			paletteSize*4*sizeof(uint8));
	}

	for (uint32 i = 0; i < mipmapCount; i++) {
		if (i > 0) {
			width.push_back(width[i-1]/2);
			height.push_back(height[i-1]/2);
			// DXT compression works on 4x4 blocks,
			// no smaller values allowed
			if (dxtCompression) {
				if (width[i] < 4 && width[i] != 0)
					width[i] = 4;
				if (height[i] < 4 && height[i] != 0)
					height[i] = 4;
			}
		}

		uint32 dataSize = readUInt32(rw);

		// There is no way to predict, when the size is going to be zero
		if (dataSize == 0)
			width[i] = height[i] = 0;

		dataSizes.push_back(dataSize);
		texels.push_back(new uint8[dataSize]);
		rw.read(reinterpret_cast <char *> (&texels[i][0]),
		        dataSize*sizeof(uint8));
	}
//cout << endl;
}

void NativeTexture::readXbox(istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	uint32 end = rw.tellg();
	end += header.length;

	uint32 platform = readUInt32(rw);
	// improve error handling
	if (platform != PLATFORM_XBOX)
		return;

	filterFlags = readUInt32(rw);

	char buffer[32];
	rw.read(buffer, 32);
	name = buffer;
	rw.read(buffer, 32);
	maskName = buffer;

	rasterFormat = readUInt32(rw);

	hasAlpha = readUInt32(rw);

	width.push_back(readUInt16(rw));
	height.push_back(readUInt16(rw));
	depth = readUInt8(rw);
	mipmapCount = readUInt8(rw);
	rw.seekg(sizeof(int8), ios::cur); // raster type (always 4)
	dxtCompression = readUInt8(rw);
//	uint32 size = readUInt32(rw);
	rw.seekg(sizeof(int32), ios::cur);

	paletteSize = (rasterFormat & RASTER_PAL8) ? 0x100 :
		      ((rasterFormat & RASTER_PAL4) ? 0x20 /* ! */ : 0);
	palette = new uint8[paletteSize*4];
	rw.read(reinterpret_cast <char *> (&palette[0]),
		paletteSize*4*sizeof(uint8));

	for (uint32 i = 0; i < mipmapCount; i++) {
		if (i != 0) {
			width.push_back(width[i-1]/2);
			height.push_back(height[i-1]/2);
			// DXT compression works on 4x4 blocks,
			// no smaller values allowed
			if (dxtCompression) {
				if (width[i] < 4)
					width[i] = 4;
				if (height[i] < 4)
					height[i] = 4;
			}
		}
		dataSizes.push_back(width[i]*height[i]);
		if (dxtCompression == 0)
			dataSizes[i] *= (depth/8);
		else if (dxtCompression == 0xC)	// DXT1 (?)
			dataSizes[i] /= 2;
		// else (0xe, 0xf) DXT3 (?)

		texels.push_back(new uint8[dataSizes[i]]);
		rw.read(reinterpret_cast <char *> (&texels[i][0]),
			dataSizes[i]*sizeof(uint8));
	}
}

void unswizzleXboxBlock(uint8 *out, uint8 *in, uint32 &outOff, uint32 inOff,
                        uint32 width, uint32 height, uint32 stride)
{
	if (width < 2 || height < 2) {
		memcpy(out+outOff, in+inOff, width*height);
		inOff += width*height;
	} else if (width == 2 && height == 2) {
		*(out+outOff) = *(in+inOff);
		*(out+outOff+1) = *(in+inOff+1);
		*(out+outOff+stride) = *(in+inOff+2);
		*(out+outOff+stride+1) = *(in+inOff+3);
		inOff += 4;
	} else {
		unswizzleXboxBlock(out, in, inOff, outOff,
		                   width/2, height/2, stride);
		unswizzleXboxBlock(out, in, inOff, outOff + (width/2),
		                   width/2, height/2, stride);
		unswizzleXboxBlock(out, in, inOff, outOff + (height/2)*stride,
		                   width/2, height/2, stride);
		unswizzleXboxBlock(out, in, inOff,
		                   outOff + (height/2)*stride + (width/2),
		                   width/2, height/2, stride);
	}
}

void NativeTexture::convertFromXbox(void)
{
	if (dxtCompression == 0xc) {
		dxtCompression = 1;
		rasterFormat &= ~RASTER_MASK;
		if (hasAlpha)
			rasterFormat |= RASTER_1555;
		else
			rasterFormat |= RASTER_565;
	} else if (dxtCompression == 0xe) {
		dxtCompression = 3;
		rasterFormat &= ~RASTER_MASK;
		rasterFormat |= RASTER_4444;
	} else if (dxtCompression == 0xf) {
		dxtCompression = 4;
	}
	platform = PLATFORM_D3D8;
}

void NativeTexture::readPs2(istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	uint32 platform = readUInt32(rw);
	// improve error handling
	if (platform != PLATFORM_PS2FOURCC)
		return;

	paletteSize = 0;
	filterFlags = readUInt32(rw);
	
	READ_HEADER(CHUNK_STRING);
	char *buffer = new char[header.length+1];
	rw.read(buffer, header.length);
	name = buffer;
	delete[] buffer;

	READ_HEADER(CHUNK_STRING);
	buffer = new char[header.length+1];
	rw.read(buffer, header.length);
	maskName = buffer;
	delete[] buffer;

	READ_HEADER(CHUNK_STRUCT);

	READ_HEADER(CHUNK_STRUCT);
	width.push_back(readUInt32(rw));
	height.push_back(readUInt32(rw));
	depth = readUInt32(rw);
	rasterFormat = readUInt32(rw);

//	if(rasterFormat == 0x22504 && width[0] == 128 && height[0] == 128 && depth == 4)
//		cout << filename << endl;
//	cout << hex << rasterFormat << endl;

	uint32 unk1 = readUInt32(rw);		// what are these?
	uint32 unk2 = readUInt32(rw);
	uint32 unk3 = readUInt32(rw);
	uint32 unk4 = readUInt32(rw);

	rw.seekg(4*4, ios::cur);		// constant
	uint32 dataSize = readUInt32(rw);	// texels + header
	uint32 paletteDataSize = readUInt32(rw);// palette + header + unknowns
	uint32 unk5 = readUInt32(rw);
	rw.seekg(4, ios::cur);			// constant (sky mipmap val)

	// 0x00000 means the texture is not swizzled and has no headers
	// 0x10000 means the texture is swizzled and has no headers
	// 0x20000 means swizzling information is contained in the header
	// the rest is the same as the generic raster format
	bool hasHeader = (rasterFormat & 0x20000);

/*
	// only these are ever used (so alpha for all textures :/ )
	if ((rasterFormat & RASTER_1555) ||
	    (rasterFormat & RASTER_4444) ||
	    (rasterFormat & RASTER_8888))
		hasAlpha = true;
	else
		hasAlpha = false;
*/
	hasAlpha = false;

//	hasAlpha = !(rasterFormat & 0x4000);
//	cout << " " << maskName;
	if (maskName != "")
		hasAlpha = true;
	if (depth == 16)
		hasAlpha = true;
//	cout << " " << hasAlpha;

	READ_HEADER(CHUNK_STRUCT);

	/* Pixels/Indices */
	uint32 end = rw.tellg();
	end += dataSize;
	uint32 i = 0;
	while (rw.tellg() < end) {
		// half dimensions if we have mipmaps
		if (i > 0) {
			width.push_back(width[i-1]/2);
			height.push_back(height[i-1]/2);
		}

		if (hasHeader) {
			rw.seekg(8*4, ios::cur); // constant
			swizzleWidth.push_back(readUInt32(rw));
			swizzleHeight.push_back(readUInt32(rw));
			rw.seekg(6*4, ios::cur); // constant
			// mipmap size/0x10, (= width*height*depth/0x10)
			dataSize = readUInt32(rw) * 0x10;
			rw.seekg(3*4, ios::cur); // constant
		} else {
			swizzleWidth.push_back(width[i]);
			swizzleHeight.push_back(height[i]);
			if (rasterFormat & 0x10000) {
				swizzleWidth[i] /= 2;
				swizzleHeight[i] /= 2;
			}
			dataSize = height[i]*height[i]*depth/8;
		}

		dataSizes.push_back(dataSize);
		texels.push_back(new uint8[dataSize]);
		rw.read(reinterpret_cast <char *> (&texels[i][0]),
		        dataSize*sizeof(uint8));
		i++;
	}
	mipmapCount = i;

	/* Palette */
	// vc dyn_trash.txd is weird here
	if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4) {
		uint32 unkh2 = 0, unkh3 = 0, unkh4 = 0;
		if (hasHeader) {
			rw.seekg(5*4, ios::cur); // same in every txd
			uint32 unkh1 = readUInt32(rw); // really unknown
			rw.seekg(2*4, ios::cur); // same in every txd
			unkh2 = readUInt32(rw);
			unkh3 = readUInt32(rw);
			rw.seekg(6*4, ios::cur); // same in every txd
			unkh4 = readUInt32(rw);
			rw.seekg(3*4, ios::cur); // same in every txd
		}

		paletteSize = (rasterFormat & RASTER_PAL8) ? 0x100 : 0x10;
		palette = new uint8[paletteSize*4];
		rw.read(reinterpret_cast <char *> (palette),
			paletteSize*4*sizeof(uint8));

		// need to work on 4bit palettes in vc & sa
		if (unkh2 == 8 && unkh3 == 3 && unkh4 == 6)
			rw.seekg(0x20, ios::cur);
		// else 8 2 4
	}
	rasterFormat &= 0xff00;
	if ((rasterFormat & RASTER_8888) && !hasAlpha) {
		rasterFormat &= ~RASTER_8888;
		rasterFormat |= RASTER_888;
	}
//cout << hex << rasterFormat << " " << hasAlpha << endl;
}

void NativeTexture::convertTo32Bit(void)
{
	// depth is always 8 (even if the palette is 4 bit)
	if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4) {
		for (uint32 j = 0; j < mipmapCount; j++) {
			uint32 dataSize = width[j]*height[j]*4;
			uint8 *newtexels = new uint8[dataSize];
			for (uint32 i = 0; i < width[j]*height[j]; i++) {
				// swap r and b
				newtexels[i*4+2] = palette[texels[j][i]*4+0];
				newtexels[i*4+1] = palette[texels[j][i]*4+1];
				newtexels[i*4+0] = palette[texels[j][i]*4+2];
				newtexels[i*4+3] = palette[texels[j][i]*4+3];
			}
			delete[] texels[j];
			texels[j] = newtexels;
			dataSizes[j] = dataSize;
		}
		delete[] palette;
		palette = 0;
		rasterFormat &= ~(RASTER_PAL4 | RASTER_PAL8);
		depth = 0x20;
	} else if ((rasterFormat & RASTER_MASK) ==  RASTER_1555) {
		for (uint32 j = 0; j < mipmapCount; j++) {
			uint32 dataSize = width[j]*height[j]*4;
			uint8 *newtexels = new uint8[dataSize];
			for (uint32 i = 0; i < width[j]*height[j]; i++) {
				uint32 col = *((uint16 *) &texels[j][i*2]);
				newtexels[i*4+0] =((col&0x001F)>>0x0)*0xFF/0x1F;
				newtexels[i*4+1] =((col&0x03E0)>>0x5)*0xFF/0x1F;
				newtexels[i*4+2] =((col&0x7C00)>>0xa)*0xFF/0x1F;
				newtexels[i*4+3] =((col&0x8000)>>0xf)*0xFF;
			}
			delete[] texels[j];
			texels[j] = newtexels;
			dataSizes[j] = dataSize;
		}
		rasterFormat = RASTER_8888;
		depth = 0x20;
	} else if ((rasterFormat & RASTER_MASK) ==  RASTER_565) {
		for (uint32 j = 0; j < mipmapCount; j++) {
			uint32 dataSize = width[j]*height[j]*4;
			uint8 *newtexels = new uint8[dataSize];
			for (uint32 i = 0; i < width[j]*height[j]; i++) {
				uint32 col = *((uint16 *) &texels[j][i*2]);
				newtexels[i*4+0] =((col&0x001F)>>0x0)*0xFF/0x1F;
				newtexels[i*4+1] =((col&0x07E0)>>0x5)*0xFF/0x3F;
				newtexels[i*4+2] =((col&0xF800)>>0xb)*0xFF/0x1F;
				newtexels[i*4+3] = 255;
			}
			delete[] texels[j];
			texels[j] = newtexels;
			dataSizes[j] = dataSize;
		}
		rasterFormat = RASTER_888;
		depth = 0x20;
	} else if ((rasterFormat & RASTER_MASK) ==  RASTER_4444) {
		for (uint32 j = 0; j < mipmapCount; j++) {
			uint32 dataSize = width[j]*height[j]*4;
			uint8 *newtexels = new uint8[dataSize];
			for (uint32 i = 0; i < width[j]*height[j]; i++) {
				uint32 col = *((uint16 *) &texels[j][i*2]);
				// swap r and b
				newtexels[i*4+0] =((col&0x000F)>>0x0)*0xFF/0xF;
				newtexels[i*4+1] =((col&0x00F0)>>0x4)*0xFF/0xF;
				newtexels[i*4+2] =((col&0x0F00)>>0x8)*0xFF/0xF;
				newtexels[i*4+3] =((col&0xF000)>>0xc)*0xFF/0xF;
			}
			delete[] texels[j];
			texels[j] = newtexels;
			dataSizes[j] = dataSize;
		}
		rasterFormat = RASTER_8888;
		depth = 0x20;
	}
	// no support for other raster formats yet
}

void NativeTexture::convertFromPS2(uint32 aref)
{
	if (platform != PLATFORM_PS2)
		return;

	for (uint32 j = 0; j < mipmapCount; j++) {
		// can't understand mipmaps
		if(j > 0){
			delete[] texels[j];
			continue;
		}
		bool swizzled = (swizzleHeight[j] != height[j]);

		// converts to 8bpp, palette stays 4bit
		if (depth == 0x4) {
			uint8 *oldtexels = texels[j];
			dataSizes[j] *= 2;
			texels[j] = new uint8[dataSizes[j]];
			for (uint32 i = 0; i < dataSizes[j]/2; i++) {
				texels[j][i*2+0] = oldtexels[i] & 0x0F;
				texels[j][i*2+1] = oldtexels[i] >> 4;
			}
			delete[] oldtexels;
			depth = 0x8;

			if (swizzled)
				processPs2Swizzle(j);
		} else if (depth == 0x8) {
			if (swizzled)
				processPs2Swizzle(j);
			unclut(texels[j], width[j], height[j]);
		} else if (depth == 0x20) {
			for (uint32 i = 0; i < width[j]*height[j]; i++) {
				// swap R and B
				uint8 tmp = texels[j][i*4+0];
				texels[j][i*4+0] = texels[j][i*4+2];
				texels[j][i*4+2] = tmp;
				// fix alpha
				uint32 newval = texels[j][i*4+3] * 0xff;
				newval /= 0x80;
				texels[j][i*4+3] = newval;
//				if (texels[j][i*4+3] != 0xFF)
//					hasAlpha = true;
			}
		}
	}
	// can't understand ps2 mipmaps
	if(mipmapCount > 1){
		mipmapCount = 1;
		texels.resize(1);
		height.resize(1);
		width.resize(1);
		rasterFormat &= ~(RASTER_AUTOMIPMAP | RASTER_MIPMAP);
	}

	if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4) {
		for (uint32 i = 0; i < paletteSize; i++) {
			if ((alphaDistribution & 0x1) == 0 &&
			    palette[i*4+3] >= aref)
				alphaDistribution |= 0x1;
			else if ((alphaDistribution & 0x2) == 0 &&
			         palette[i*4+3] < aref)
				alphaDistribution |= 0x2;
			uint32 newalpha = palette[i*4+3] * 0xff;
			newalpha /= 0x80;
			palette[i*4+3] = newalpha;
		}
	}

	platform = PLATFORM_D3D8;
	dxtCompression = 0;
}

void NativeTexture::processPs2Swizzle(uint32 i)
{
	dataSizes[i] = swizzleWidth[i] *
	                 swizzleHeight[i] * 4;
	uint8 *newtexels = new uint8[dataSizes[i]];
	unswizzle8(newtexels, texels[i],
	           swizzleWidth[i]*2,
	           swizzleHeight[i]*2);
	delete[] texels[i];
	texels[i] = newtexels;

	uint32 stride = swizzleWidth[i]*2;
	if (stride != width[i]) {
		dataSizes[i] = width[i]*height[i];
		newtexels = new uint8[dataSizes[i]];
		for (uint32 y = 0; y < height[i]; y++)
			for (uint32 x = 0; x < width[i]; x++)
				newtexels[y*width[i]+x] = texels[i][y*stride+x];
		delete[] texels[i];
		texels[i] = newtexels;
	}
}

void NativeTexture::decompressDxt4(void)
{
	for (uint32 i = 0; i < mipmapCount; i++) {
		/* j loops through old texels
		 * x and y loop through new texels */
		uint32 x = 0, y = 0;
		uint32 dataSize = width[i]*height[i]*4;
		uint8 *newtexels = new uint8[dataSize];
		for (uint32 j = 0; j < width[i]*height[i]; j += 16) {
			/* calculate colors */
			uint32 col0 = *((uint16 *) &texels[i][j+8]);
			uint32 col1 = *((uint16 *) &texels[i][j+10]);
			uint32 c[4][4];
			// swap r and b
			c[0][0] = (col0 & 0x1F)*0xFF/0x1F;
			c[0][1] = ((col0 & 0x7E0) >> 5)*0xFF/0x3F;
			c[0][2] = ((col0 & 0xF800) >> 11)*0xFF/0x1F;

			c[1][0] = (col1 & 0x1F)*0xFF/0x1F;
			c[1][1] = ((col1 & 0x7E0) >> 5)*0xFF/0x3F;
			c[1][2] = ((col1 & 0xF800) >> 11)*0xFF/0x1F;

			c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
			c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
			c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

			c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
			c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
			c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

			uint32 a[8];
			a[0] = texels[i][j+0];
			a[1] = texels[i][j+1];
			if (a[0] > a[1]) {
				a[2] = (6*a[0] + 1*a[1])/7;
				a[3] = (5*a[0] + 2*a[1])/7;
				a[4] = (4*a[0] + 3*a[1])/7;
				a[5] = (3*a[0] + 4*a[1])/7;
				a[6] = (2*a[0] + 5*a[1])/7;
				a[7] = (1*a[0] + 6*a[1])/7;
			} else {
				a[2] = (4*a[0] + 1*a[1])/5;
				a[3] = (3*a[0] + 2*a[1])/5;
				a[4] = (2*a[0] + 3*a[1])/5;
				a[5] = (1*a[0] + 4*a[1])/5;
				a[6] = 0;
				a[7] = 0xFF;
			}

			/* make index list */
			uint32 indicesint = *((uint32 *) &texels[i][j+12]);
			uint8 indices[16];
			for (int32 k = 0; k < 16; k++) {
				indices[k] = indicesint & 0x3;
				indicesint >>= 2;
			}
			// actually 6 bytes
			uint64 alphasint = *((uint64 *) &texels[i][j+2]);
			uint8 alphas[16];
			for (int32 k = 0; k < 16; k++) {
				alphas[k] = alphasint & 0x7;
				alphasint >>= 3;
			}

			/* write bytes */
			for (uint32 k = 0; k < 4; k++)
				for (uint32 l = 0; l < 4; l++) {
	// wtf?
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 0] = c[indices[l*4+k]][0];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 1] = c[indices[l*4+k]][1];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 2] = c[indices[l*4+k]][2];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 3] = a[alphas[l*4+k]];
				}
			x += 4;
			if (x >= width[i]) {
				y += 4;
				x = 0;
			}
		}
		delete[] texels[i];
		texels[i] = newtexels;
		dataSizes[i] = dataSize;
	}
	depth = 0x20;
	rasterFormat = RASTER_8888;
	dxtCompression = 0;
}

void NativeTexture::decompressDxt3(void)
{
	for (uint32 i = 0; i < mipmapCount; i++) {
		/* j loops through old texels
		 * x and y loop through new texels */
		uint32 x = 0, y = 0;
		uint32 dataSize = width[i]*height[i]*4;
		uint8 *newtexels = new uint8[dataSize];
		for (uint32 j = 0; j < width[i]*height[i]; j += 16) {
			/* calculate colors */
			uint32 col0 = *((uint16 *) &texels[i][j+8]);
			uint32 col1 = *((uint16 *) &texels[i][j+10]);
			uint32 c[4][4];
			// swap r and b
			c[0][0] = (col0 & 0x1F)*0xFF/0x1F;
			c[0][1] = ((col0 & 0x7E0) >> 5)*0xFF/0x3F;
			c[0][2] = ((col0 & 0xF800) >> 11)*0xFF/0x1F;

			c[1][0] = (col1 & 0x1F)*0xFF/0x1F;
			c[1][1] = ((col1 & 0x7E0) >> 5)*0xFF/0x3F;
			c[1][2] = ((col1 & 0xF800) >> 11)*0xFF/0x1F;

			c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
			c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
			c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

			c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
			c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
			c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

			/* make index list */
			uint32 indicesint = *((uint32 *) &texels[i][j+12]);
			uint8 indices[16];
			for (int32 k = 0; k < 16; k++) {
				indices[k] = indicesint & 0x3;
				indicesint >>= 2;
			}
			uint64 alphasint = *((uint64 *) &texels[i][j+0]);
			uint8 alphas[16];
			for (int32 k = 0; k < 16; k++) {
				alphas[k] = (alphasint & 0xF)*17;
				alphasint >>= 4;
			}

			/* write bytes */
			for (uint32 k = 0; k < 4; k++)
				for (uint32 l = 0; l < 4; l++) {
	// wtf?
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 0] = c[indices[l*4+k]][0];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 1] = c[indices[l*4+k]][1];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 2] = c[indices[l*4+k]][2];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 3] = alphas[l*4+k];
				}
			x += 4;
			if (x >= width[i]) {
				y += 4;
				x = 0;
			}
		}
		delete[] texels[i];
		texels[i] = newtexels;
		dataSizes[i] = dataSize;
	}
	depth = 0x20;
	rasterFormat += 0x0200;
	dxtCompression = 0;
}

void NativeTexture::decompressDxt1(void)
{
	for (uint32 i = 0; i < mipmapCount; i++) {
		/* j loops through old texels
		 * x and y loop through new texels */
		uint32 x = 0, y = 0;
		uint32 dataSize = width[i]*height[i]*4;
		uint8 *newtexels = new uint8[dataSize];
		for (uint32 j = 0; j < width[i]*height[i]/2; j += 8) {
			/* calculate colors */
			uint32 col0 = *((uint16 *) &texels[i][j+0]);
			uint32 col1 = *((uint16 *) &texels[i][j+2]);
			uint32 c[4][4];
			// swap r and b
			c[0][0] = (col0 & 0x1F)*0xFF/0x1F;
			c[0][1] = ((col0 & 0x7E0) >> 5)*0xFF/0x3F;
			c[0][2] = ((col0 & 0xF800) >> 11)*0xFF/0x1F;
			c[0][3] = 0xFF;

			c[1][0] = (col1 & 0x1F)*0xFF/0x1F;
			c[1][1] = ((col1 & 0x7E0) >> 5)*0xFF/0x3F;
			c[1][2] = ((col1 & 0xF800) >> 11)*0xFF/0x1F;
			c[1][3] = 0xFF;
			if (col0 > col1) {
				c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
				c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
				c[2][2] = (2*c[0][2] + 1*c[1][2])/3;
				c[2][3] = 0xFF;

				c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
				c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
				c[3][2] = (1*c[0][2] + 2*c[1][2])/3;
				c[3][3] = 0xFF;
			} else {
				c[2][0] = (c[0][0] + c[1][0])/2;
				c[2][1] = (c[0][1] + c[1][1])/2;
				c[2][2] = (c[0][2] + c[1][2])/2;
				c[2][3] = 0xFF;

				c[3][0] = 0x00;
				c[3][1] = 0x00;
				c[3][2] = 0x00;
				if (rasterFormat & 0x0200)
					c[3][3] = 0xFF;
				else // 0x0100
					c[3][3] = 0x00;
			}

			/* make index list */
			uint32 indicesint = *((uint32 *) &texels[i][j+4]);
			uint8 indices[16];
			for (int32 k = 0; k < 16; k++) {
				indices[k] = indicesint & 0x3;
				indicesint >>= 2;
			}

			/* write bytes */
			for (uint32 k = 0; k < 4; k++)
				for (uint32 l = 0; l < 4; l++) {
	// wtf?
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 0] = c[indices[l*4+k]][0];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 1] = c[indices[l*4+k]][1];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 2] = c[indices[l*4+k]][2];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 3] = c[indices[l*4+k]][3];
				}
			x += 4;
			if (x >= width[i]) {
				y += 4;
				x = 0;
			}
		}
		delete[] texels[i];
		texels[i] = newtexels;
		dataSizes[i] = dataSize;
	}
	depth = 0x20;
	rasterFormat += 0x0400;
	dxtCompression = 0;
}

void NativeTexture::decompressDxt(void)
{
	if (dxtCompression == 0)
		return;

	if (dxtCompression == 1)
		decompressDxt1();
	else if (dxtCompression == 3)
		decompressDxt3();
	else if (dxtCompression == 4) {
		decompressDxt4();
	} else
		cout << "dxt" << dxtCompression << " not supported\n";
}

void NativeTexture::writeTGA(void)
{
	if (depth != 32) {
		cout << "not writing file: " << filename << endl;
		return;
	}
	char filename[36];
	strcpy(filename, name.c_str());
	strcat(filename, ".tga");
	ofstream tga(filename, ios::binary);
	if (tga.fail()) {
		cout << "not writing file: " << filename << endl;
		return;
	}
	writeUInt8(0, tga);
	writeUInt8(0, tga);
	writeUInt8(2, tga);
	writeUInt16(0, tga);
	writeUInt16(0, tga);
	writeUInt8(0, tga);
	writeUInt16(0, tga);
	writeUInt16(0, tga);
	writeUInt16(width[0], tga);
	writeUInt16(height[0], tga);
	writeUInt8(0x20, tga);
	writeUInt8(0x28, tga);
	for (uint32 j = 0; j < width[0]*height[0]; j++){
		writeUInt8(texels[0][j*4+0], tga);
		writeUInt8(texels[0][j*4+1], tga);
		writeUInt8(texels[0][j*4+2], tga);
		writeUInt8(texels[0][j*4+3], tga);
	}
	tga.close();
}

NativeTexture::NativeTexture(void)
: platform(0), name(""), maskName(""), filterFlags(0), rasterFormat(0),
  depth(0), palette(0), paletteSize(0), hasAlpha(false), mipmapCount(0),
  alphaDistribution(0), dxtCompression(0)
{
}

NativeTexture::NativeTexture(const NativeTexture &orig)
: platform(orig.platform),
  name(orig.name),
  maskName(orig.maskName),
  filterFlags(orig.filterFlags),
  rasterFormat(orig.rasterFormat),
  width(orig.width),
  height(orig.height),
  depth(orig.depth),
  dataSizes(orig.dataSizes),
  paletteSize(orig.paletteSize),
  hasAlpha(orig.hasAlpha),
  mipmapCount(orig.mipmapCount),
  swizzleWidth(orig.swizzleWidth),
  swizzleHeight(orig.swizzleHeight),
  alphaDistribution(orig.alphaDistribution),
  dxtCompression(orig.dxtCompression)
{
	if (orig.palette) {
		palette = new uint8[paletteSize*4*sizeof(uint8)];
		memcpy(palette, orig.palette, paletteSize*4*sizeof(uint8));
	} else {
		palette = 0;
	}

	for (uint32 i = 0; i < orig.texels.size(); i++) {
		uint32 dataSize = dataSizes[i];
		uint8 *newtexels = new uint8[dataSize];
		memcpy(newtexels, &orig.texels[i][0], dataSize);
		texels.push_back(newtexels);
	}
}

NativeTexture &NativeTexture::operator=(const NativeTexture &that)
{
	if (this != &that) {
		platform = that.platform;
		name = that.name;
		maskName = that.maskName;
		filterFlags = that.filterFlags;
		rasterFormat = that.rasterFormat;
		width = that.width;
		height = that.height;
		depth = that.depth;
		dataSizes = that.dataSizes;

		paletteSize = that.paletteSize;
		hasAlpha = that.hasAlpha;
		mipmapCount = that.mipmapCount;
		swizzleWidth = that.swizzleWidth;
		swizzleHeight = that.swizzleHeight;
		dxtCompression = that.dxtCompression;


		delete[] palette;
		palette = 0;
		if (that.palette) {
			palette = new uint8[that.paletteSize*4];
			memcpy(&palette[0], &that.palette[0],
			       that.paletteSize*4*sizeof(uint8));
		}

		for (uint32 i = 0; i < texels.size(); i++) {
			delete[] texels[i];
			texels[i] = 0;
			if (that.texels[i]) {
				texels[i] = new uint8[that.dataSizes[i]];
				memcpy(&texels[i][0], &that.texels[i][0],
				       that.dataSizes[i]*sizeof(uint8));
			}
		}
	}
	return *this;
}

NativeTexture::~NativeTexture(void)
{
	delete[] palette;
	palette = 0;
	for (uint32 i = 0; i < texels.size(); i++) {
		delete[] texels[i];
		texels[i] = 0;
	}
}



/* convert from CLUT format used by the ps2 */
static void unclut(uint8 *texels, uint32 width, uint32 height)
{
	uint8 map[4] = { 0, 16, 8, 24 };
	for (uint32 i = 0; i < width*height; i++)
		texels[i] = (texels[i] & ~0x18) | map[(texels[i] & 0x18) >> 3];
}

/* taken from the ps2 linux website */
static void unswizzle8(uint8 *texels, uint8 *rawIndices,
                       uint32 width, uint32 height)
{
	for (uint32 y = 0; y < height; y++)
		for (uint32 x = 0; x < width; x++) {
			int32 block_loc = (y&(~0x0F))*width + (x&(~0x0F))*2;
			uint32 swap_sel = (((y+2)>>2)&0x01)*4;
			int32 ypos = (((y&(~3))>>1) + (y&1))&0x07;
			int32 column_loc = ypos*width*2 + ((x+swap_sel)&0x07)*4;
			int32 byte_sum = ((y>>1)&1) + ((x>>2)&2);
			uint32 swizzled = block_loc + column_loc + byte_sum;
//			cout << swizzled << endl;
			texels[y*width+x] = rawIndices[swizzled];
		}
}

}
