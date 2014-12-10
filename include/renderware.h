#ifndef _RENDERWARE_H_
#define _RENDERWARE_H_
#ifdef _WIN32
  #include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <string>

#ifdef DEBUG
	#define READ_HEADER(x)\
	header.read(rw);\
	if (header.type != (x)) {\
		cerr << filename << " ";\
		ChunkNotFound((x), rw.tellg());\
	}
#else
	#define READ_HEADER(x)\
	header.read(rw);
#endif

namespace rw {

//typedef unsigned int uint;
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef float float32;

extern char *filename;
extern uint32 version;

enum PLATFORM_ID
{
	PLATFORM_OGL = 2,
	PLATFORM_PS2    = 4,
	PLATFORM_XBOX   = 5,
	PLATFORM_D3D8   = 8,
	PLATFORM_D3D9   = 9,
	PLATFORM_PS2FOURCC = 0x00325350 /* "PS2\0" */
};
typedef enum PLATFORM_ID PLATFORM_ID;

enum CHUNK_TYPE {
        CHUNK_NAOBJECT        = 0x0,
        CHUNK_STRUCT          = 0x1,
        CHUNK_STRING          = 0x2,
        CHUNK_EXTENSION       = 0x3,
        CHUNK_CAMERA          = 0x5,
        CHUNK_TEXTURE         = 0x6,
        CHUNK_MATERIAL        = 0x7,
        CHUNK_MATLIST         = 0x8,
        CHUNK_ATOMICSECT      = 0x9,
        CHUNK_PLANESECT       = 0xA,
        CHUNK_WORLD           = 0xB,
        CHUNK_SPLINE          = 0xC,
        CHUNK_MATRIX          = 0xD,
        CHUNK_FRAMELIST       = 0xE,
        CHUNK_GEOMETRY        = 0xF,
        CHUNK_CLUMP           = 0x10,
        CHUNK_LIGHT           = 0x12,
        CHUNK_UNICODESTRING   = 0x13,
        CHUNK_ATOMIC          = 0x14,
        CHUNK_TEXTURENATIVE   = 0x15,
        CHUNK_TEXDICTIONARY   = 0x16,
        CHUNK_ANIMDATABASE    = 0x17,
        CHUNK_IMAGE           = 0x18,
        CHUNK_SKINANIMATION   = 0x19,
        CHUNK_GEOMETRYLIST    = 0x1A,
        CHUNK_ANIMANIMATION   = 0x1B,
        CHUNK_HANIMANIMATION  = 0x1B,
        CHUNK_TEAM            = 0x1C,
        CHUNK_CROWD           = 0x1D,
        CHUNK_RIGHTTORENDER   = 0x1F,
        CHUNK_MTEFFECTNATIVE  = 0x20,
        CHUNK_MTEFFECTDICT    = 0x21,
        CHUNK_TEAMDICTIONARY  = 0x22,
        CHUNK_PITEXDICTIONARY = 0x23,
        CHUNK_TOC             = 0x24,
        CHUNK_PRTSTDGLOBALDATA = 0x25,
        CHUNK_ALTPIPE         = 0x26,
        CHUNK_PIPEDS          = 0x27,
        CHUNK_PATCHMESH       = 0x28,
        CHUNK_CHUNKGROUPSTART = 0x29,
        CHUNK_CHUNKGROUPEND   = 0x2A,
        CHUNK_UVANIMDICT      = 0x2B,
        CHUNK_COLLTREE        = 0x2C,
        CHUNK_ENVIRONMENT     = 0x2D,
        CHUNK_COREPLUGINIDMAX = 0x2E,

	CHUNK_MORPH           = 0x105,
	CHUNK_SKYMIPMAP       = 0x110,
	CHUNK_SKIN            = 0x116,
	CHUNK_PARTICLES       = 0x118,
	CHUNK_HANIM           = 0x11E,
	CHUNK_MATERIALEFFECTS = 0x120,
	CHUNK_PDSPLG          = 0x131,
	CHUNK_ADCPLG          = 0x134,
	CHUNK_UVANIMPLG       = 0x135,
	CHUNK_BINMESH         = 0x50E,
	CHUNK_NATIVEDATA      = 0x510,
	CHUNK_VERTEXFORMAT    = 0x510,

	CHUNK_PIPELINESET      = 0x253F2F3,
	CHUNK_SPECULARMAT      = 0x253F2F6,
	CHUNK_2DFX             = 0x253F2F8,
	CHUNK_NIGHTVERTEXCOLOR = 0x253F2F9,
	CHUNK_COLLISIONMODEL   = 0x253F2FA,
	CHUNK_REFLECTIONMAT    = 0x253F2FC,
	CHUNK_MESHEXTENSION    = 0x253F2FD,
	CHUNK_FRAME            = 0x253F2FE
};
typedef enum CHUNK_TYPE CHUNK_TYPE;

enum {
	RASTER_DEFAULT = 0x0000,
	RASTER_1555 = 0x0100,
	RASTER_565 = 0x0200,
	RASTER_4444 = 0x0300,
	RASTER_LUM8 = 0x0400,
	RASTER_8888 = 0x0500,
	RASTER_888 = 0x0600,
	RASTER_16 = 0x0700,
	RASTER_24 = 0x0800,
	RASTER_32 = 0x0900,
	RASTER_555 = 0x0a00,

	RASTER_AUTOMIPMAP = 0x1000,
	RASTER_PAL8 = 0x2000,
	RASTER_PAL4 = 0x4000,
	RASTER_MIPMAP = 0x8000,

	RASTER_MASK = 0x0F00
};

struct HeaderInfo
{
	uint32 type;
	uint32 length;
	uint32 build;
	uint32 version;
	bool read(std::istream &rw);
	bool peek(std::istream &rw);
	uint32 write(std::ostream &rw);
	bool findChunk(std::istream &rw, uint32 type);
};

void ChunkNotFound(CHUNK_TYPE chunk, uint32 address);
uint32 writeInt8(int8 tmp, std::ostream &rw);
uint32 writeUInt8(uint8 tmp, std::ostream &rw);
uint32 writeInt16(int16 tmp, std::ostream &rw);
uint32 writeUInt16(uint16 tmp, std::ostream &rw);
uint32 writeInt32(int32 tmp, std::ostream &rw);
uint32 writeUInt32(uint32 tmp, std::ostream &rw);
uint32 writeFloat32(float32 tmp, std::ostream &rw);
int8 readInt8(std::istream &rw);
uint8 readUInt8(std::istream &rw);
int16 readInt16(std::istream &rw);
uint16 readUInt16(std::istream &rw);
int32 readInt32(std::istream &rw);
uint32 readUInt32(std::istream &rw);
float32 readFloat32(std::istream &rw);

std::string getChunkName(uint32 i);

/*
 * DFFs
 */

enum { 
        FLAGS_TRISTRIP   = 0x01, 
        FLAGS_POSITIONS  = 0x02, 
        FLAGS_TEXTURED   = 0x04, 
        FLAGS_PRELIT     = 0x08, 
        FLAGS_NORMALS    = 0x10, 
        FLAGS_LIGHT      = 0x20, 
	FLAGS_MODULATEMATERIALCOLOR  = 0x40, 
        FLAGS_TEXTURED2  = 0x80
};

enum {
	MATFX_BUMPMAP = 0x1,
	MATFX_ENVMAP = 0x2,
	MATFX_BUMPENVMAP = 0x3,
	MATFX_DUAL = 0x4,
	MATFX_UVTRANSFORM = 0x5,
	MATFX_DUALUVTRANSFORM = 0x6,
};

enum {
	FACETYPE_STRIP = 0x1,
	FACETYPE_LIST = 0x0
};

/* gta3 ps2: 302, 304, 310 
 * gta3 pc: 304, 310, 401ffff, 800ffff, c02ffff
 * gtavc ps2: c02ffff
 * gtavc pc: c02ffff, 800ffff, 1003ffff
 * gtasa: 1803ffff */

enum {
        GTA3_1 = 0x00000302,
        GTA3_2 = 0x00000304,
        GTA3_3 = 0x00000310,
        GTA3_4 = 0x0800FFFF,
        VCPS2  = 0x0C02FFFF,
        VCPC   = 0x1003FFFF,
        SA     = 0x1803FFFF
};

struct Frame
{
	float32 rotationMatrix[9];
	float32 position[3];
	int32 parent;

	/* Extensions */

	/* node name */
	std::string name;

	/* hanim */
	bool hasHAnim;
	uint32 hAnimUnknown1;
	int32 hAnimBoneId;
	uint32 hAnimBoneCount;
	uint32 hAnimUnknown2;
	uint32 hAnimUnknown3;
	std::vector<int32> hAnimBoneIds;
	std::vector<uint32> hAnimBoneNumbers;
	std::vector<uint32> hAnimBoneTypes;

	/* functions */
	void readStruct(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 writeStruct(std::ostream &dff);
	uint32 writeExtension(std::ostream &dff);

	void dump(uint32 index, std::string ind = "");

	Frame(void);
};


struct Atomic
{
	int32 frameIndex;
	int32 geometryIndex;

	/* Extensions */

	/* right to render */
	bool hasRightToRender;
	uint32 rightToRenderVal1;
	uint32 rightToRenderVal2;

	/* particles */
	bool hasParticles;
	uint32 particlesVal;

	/* pipelineset */
	bool hasPipelineSet;
	uint32 pipelineSetVal;

	/* material fx */
	bool hasMaterialFx;
	uint32 materialFxVal;

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 write(std::ostream &dff);
	void dump(uint32 index, std::string ind = "");

	Atomic(void);
};

struct Texture
{
	uint32 filterFlags;
	std::string name;
	std::string maskName;

	/* Extensions */

	/* sky mipmap */
	bool hasSkyMipmap;

	/* functions */
	void read(std::istream &dff);
	uint32 write(std::ostream &dff);
	void readExtension(std::istream &dff);
	void dump(std::string ind = "");

	Texture(void);
};

struct MatFx
{
	uint32 type;

	float32 bumpCoefficient;
	float32 envCoefficient;
	float32 srcBlend;
	float32 destBlend;

	bool hasTex1;
	Texture tex1;
	bool hasTex2;
	Texture tex2;
	bool hasDualPassMap;
	Texture dualPassMap;

	void dump(std::string ind = "");

	MatFx(void);
};

struct Material
{
	uint32 flags;
	uint8 color[4];
	uint32 unknown;
	bool hasTex;
	float32 surfaceProps[3]; /* ambient, specular, diffuse */

	Texture texture;

	/* Extensions */

	/* right to render */
	bool hasRightToRender;
	uint32 rightToRenderVal1;
	uint32 rightToRenderVal2;

	/* matfx */
	bool hasMatFx;
	MatFx *matFx;

	/* reflection mat */
	bool hasReflectionMat;
	float32 reflectionChannelAmount[4];
	float32 reflectionIntensity;

	/* specular mat */
	bool hasSpecularMat;
	float32 specularLevel;
	std::string specularName;

	/* uv anim */
	bool hasUVAnim;
	uint32 uvVal;
	std::string uvName;

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 write(std::ostream &dff);

	void dump(uint32 index, std::string ind = "");

	Material(void);
	Material(const Material &orig);
	Material &operator=(const Material &that);
	~Material(void);
};

struct MeshExtension
{
	uint32 unknown;

	std::vector<float32> vertices;
	std::vector<float32> texCoords;
	std::vector<uint8> vertexColors;
	std::vector<uint16> faces;
	std::vector<uint16> assignment;

	std::vector<std::string> textureName;
	std::vector<std::string> maskName;
	std::vector<float32> unknowns;
};

struct Split
{
	uint32 matIndex;
	std::vector<uint32> indices;	
};

struct Geometry
{
	uint32 flags;
	uint32 numUVs;
	bool hasNativeGeometry;

	uint32 vertexCount;
	std::vector<uint16> faces;
	std::vector<uint8> vertexColors;
	std::vector<float32> texCoords[8];

	/* morph target (only one) */
	float32 boundingSphere[4];
	uint32 hasPositions;
	uint32 hasNormals;
	std::vector<float32> vertices;
	std::vector<float32> normals;

	std::vector<Material> materialList;

	/* Extensions */

	/* bin mesh */
	uint32 faceType;
	uint32 numIndices;
	std::vector<Split> splits;

	/* skin */
	bool hasSkin;
	uint32 boneCount;
	uint32 specialIndexCount;
	uint32 unknown1;
	uint32 unknown2;
	std::vector<uint8> specialIndices;
	std::vector<uint32> vertexBoneIndices;
	std::vector<float32> vertexBoneWeights;
	std::vector<float32> inverseMatrices;

	/* mesh extension */
	bool hasMeshExtension;
	MeshExtension *meshExtension;

	/* night vertex colors */
	bool hasNightColors;
	uint32 nightColorsUnknown;
	std::vector<uint8> nightColors;

	/* 2dfx */
	bool has2dfx;
	std::vector<uint8> twodfxData;

	/* morph (only flag) */
	bool hasMorph;

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	void readMeshExtension(std::istream &dff);
	uint32 write(std::ostream &dff);
	uint32 writeMeshExtension(std::ostream &dff);

	void cleanUp(void);

	void dump(uint32 index, std::string ind = "", bool detailed = false);

	Geometry(void);
	Geometry(const Geometry &orig);
	Geometry &operator= (const Geometry &other);
	~Geometry(void);
private:
	void readPs2NativeData(std::istream &dff);
	void readXboxNativeData(std::istream &dff);
	void readXboxNativeSkin(std::istream &dff);
	void readOglNativeData(std::istream &dff, int size);
	void readNativeSkinMatrices(std::istream &dff);
	bool isDegenerateFace(uint32 i, uint32 j, uint32 k);
	void generateFaces(void);
	void deleteOverlapping(std::vector<uint32> &typesRead, uint32 split);
	void readData(uint32 vertexCount, uint32 type, // native data block
                      uint32 split, std::istream &dff);

	uint32 addTempVertexIfNew(uint32 index);
};

struct Light
{
	int32 frameIndex;
	float32 radius;
	float32 color[3];
	float32 minusCosAngle;
	uint32 type;
	uint32 flags;

	void read(std::istream &dff);
	uint32 write(std::ostream &dff);
};

struct Clump
{
	std::vector<Atomic> atomicList;
	std::vector<Frame> frameList;
	std::vector<Geometry> geometryList;
	std::vector<Light> lightList;

	/* Extensions */
	/* collision file */
	bool hasCollision;
	std::vector<uint8> colData;

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 write(std::ostream &dff);
	void dump(bool detailed = false);
	void clear(void);
};

/*
 * TXDs
 */

struct NativeTexture
{
	uint32 platform;
	std::string name;
	std::string maskName;
	uint32 filterFlags;
	uint32 rasterFormat;
	std::vector<uint32> width;	// store width & height
	std::vector<uint32> height;	// for each mipmap
	uint32 depth;
	std::vector<uint32> dataSizes;
	std::vector<uint8*> texels;	// holds either indices or color values
					// (also per mipmap)
	uint8 *palette;
	uint32 paletteSize;

	bool hasAlpha;
	uint32 mipmapCount;

	// PS2
	std::vector<uint32> swizzleWidth;
	std::vector<uint32> swizzleHeight;
	// bit 0: alpha values above (or equal to) the threshold
	// bit 1: alpha values below the threshold
	// both 0: no info
	uint32 alphaDistribution;

	// PC
	uint32 dxtCompression;

	/* functions */
	void readD3d(std::istream &txd);
	void readPs2(std::istream &txd);
	void readXbox(std::istream &txd);
	uint32 writeD3d(std::ostream &txd);
	void writeTGA(void);

	void convertFromPS2(uint32 aref);
	void processPs2Swizzle(uint32 mip);
	void convertFromXbox(void);
	void decompressDxt(void);
	void decompressDxt1(void);
	void decompressDxt3(void);
	void decompressDxt4(void);
	void convertTo32Bit(void);

	NativeTexture(void);
	NativeTexture(const NativeTexture &orig);
	NativeTexture &operator=(const NativeTexture &that);
	~NativeTexture(void);
};

struct TextureDictionary
{
	std::vector<NativeTexture> texList;

	/* functions */
	void read(std::istream &txd);
	uint32 write(std::ostream &txd);
	void clear(void);
	~TextureDictionary(void);
};

struct UVAnimation
{
	std::vector<uint8> data;

	void read(std::istream &dff);
	uint32 write(std::ostream &dff);
};

struct UVAnimDict
{
	std::vector<UVAnimation> animList;

	void read(std::istream &dff);
	uint32 write(std::ostream &dff);
	void clear(void);
	~UVAnimDict(void);
};

}

#endif
