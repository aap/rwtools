#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <renderware.h>
#include "args.h"

using namespace std;
using namespace rw;

enum {
	DEFAULT = 0,
	WORLD,
	VEHICLE,
	PED
};

char *argv0;

int
fixmat(Material *mat)
{
	if(mat->hasTex)
		mat->texture.hasSkyMipmap = false;
	if(mat->hasMatFx){
		if(mat->matFx->hasTex1)
			mat->matFx->tex1.hasSkyMipmap = false;
		if(mat->matFx->hasTex2)
			mat->matFx->tex2.hasSkyMipmap = false;
	}

	if(!mat->hasRightToRender || mat->rightToRenderVal1 != CHUNK_PDSPLG)
		return 0;
	mat->hasRightToRender = false;
	switch(mat->rightToRenderVal2){
	// maybe the other way around?
	case 0x53f20085:
		mat->hasReflectionMat = false;
	case 0x53f20087:
		mat->hasSpecularMat = false;
	case 0x53f2008b:
		mat->hasRightToRender = false;
		return 0x53f2009a;	// vehicle pipeline
		break;
	}
	return 0;
}

void
fixatm(Atomic *a, int type, int pipeline)
{
	if(a->hasRightToRender && a->rightToRenderVal1 == CHUNK_PDSPLG){
/*		// whaaa
		if(pipeline  == 0 && 
		   (a->rightToRenderVal2 == 0x1100d || a->rightToRenderVal2 == 0x1100e))
			pipeline = 0x53f2009a;
*/
		a->hasRightToRender = false;
	}
/*	// what's this? when do we have one?
	if(a->hasMaterialFx && !a->hasRightToRender){
		a->hasRightToRender = true;
		a->rightToRenderVal1 = CHUNK_MATERIALEFFECTS;
		a->rightToRenderVal2 = 0;
	}
*/
	if(pipeline == 0)
		return;
	if(type == DEFAULT){
		a->hasPipelineSet = true;
		a->pipelineSetVal = pipeline;
	}else if(type == WORLD){
	}else if(type == VEHICLE){
		if(pipeline != 0x53f2009a){
			a->hasPipelineSet = true;
			a->pipelineSetVal = pipeline;
		}
	}else if(type == PED){
	}
}

void
fixpipeline(Clump *c, int type)
{
	int ret, pipeline;
	for(uint32 i = 0; i < c->atomicList.size(); i++){
		Geometry *g = &c->geometryList[c->atomicList[i].geometryIndex];
		pipeline = 0;
		for(uint32 j = 0; j < g->materialList.size(); j++){
			ret = fixmat(&g->materialList[j]);
			if(ret)
				pipeline = ret;
		}
		fixatm(&c->atomicList[i], type, pipeline);
	}
}

void
usage(void)
{
	cerr << "usage: " << argv0 <<
	        " [-d] [-dd]" <<
	        " [-c] [-v version_string] [-V version] " <<
	        " in_dff out_dff\n";
	cerr << "-c: Clean up geometries; advised for PS2 dffs.\n";
	cerr << "-m: Fix environment and specular material of PS2 dffs " <<
	        "according to pipeline used.\n";
	cerr << "-v: Known versions: GTA3, GTAVC_1, GTAVC_2, GTASA\n";
	cerr << "-V: Set any version you like in hexadecimal.\n";
	cerr << "-d: Dump dff data.\n";
	cerr << "-dd: Dump dff data detailed.\n";
	exit(1);
}

void
sanityCheck(Geometry *g)
{
	int nverts = g->vertices.size()/3;
	int nnorms = g->normals.size()/3;
	int ncolors = g->vertexColors.size()/4;
	int nncolors = g->nightColors.size()/4;
	int nuv[8];
	nuv[0] = g->texCoords[0].size()/2;
	nuv[1] = g->texCoords[1].size()/2;
	int nfaces = g->faces.size()/4;
//	if(nverts != g->vertexCount)
//		cout << filename << " vertices: " << nverts << " " << g->vertexCount << endl;
	if((g->flags & FLAGS_NORMALS) && nverts != nnorms)
		cout << filename << " normals: " << nnorms << " " << nverts << endl;
	if((g->flags & FLAGS_PRELIT) && nverts != ncolors)
		cout << filename << " colors: " << ncolors << " " << nverts << endl;
	if(((g->flags & FLAGS_TEXTURED) || (g->flags & FLAGS_TEXTURED2)))
		for(int i = 0; i < g->numUVs; i++)
			if(nverts != nuv[i])
				cout << filename << " uv " << i << ": " << nuv[i] << " " << nverts << endl;
	if(g->hasNightColors && nncolors != nverts)
		cout << filename << " ncolors: " << nncolors << " " << nverts << endl;
}

int
main(int argc, char *argv[])
{
	HeaderInfo header;
	if(sizeof(uint32) != 4 || sizeof(int32) != 4 ||
	   sizeof(uint16) != 2 || sizeof(int16) != 2 ||
	   sizeof(uint8)  != 1 || sizeof(int8)  != 1 ||
	   sizeof(float32) != 4){
		cerr << "type size not correct\n";
		return 1;
	}

	string verstring;
	string typestr = "default";
	version = VCPC;
	int cleanflag = 0;
	int dumpflag = 0;
	int fixmatflag = 0;
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
		else
			cout << "unknown version\n";
		break;
	case 'V':
		sscanf(EARGF(usage()), "%x", &version);
		break;
	case 'c':
		cleanflag++;
		break;
	case 'd':
		dumpflag++;
		break;
	case 'm':
		fixmatflag++;
		break;
	case 't':
		typestr = EARGF(usage());
		break;
	default:
		usage();
	}ARGEND;

	int type;
	if(typestr == "default")
		type = DEFAULT;
	else if(typestr == "world")
		type = WORLD;
	else if(typestr == "vehicle")
		type = VEHICLE;
	else if(typestr == "ped")
		type = PED;
	else{
		cerr << "unknown type " << typestr << endl;
		return 1;
	}

	if(argc < 2)
		usage();

	filename = argv[0];
	ifstream in(argv[0], ios::binary);
	if(in.fail()){
		cerr << "cannot open " << argv[0] << endl;
		return 1;
	}

	ofstream out(argv[1], ios::binary);
	if(out.fail()){
		cerr << "cannot open " << argv[1] << endl;
		return 1;
	}

	while(header.read(in) && header.type != CHUNK_NAOBJECT){
		if(header.type == CHUNK_CLUMP){
			in.seekg(-12, ios::cur);
			Clump *clump = new Clump;
			clump->read(in);

			for(uint32 i = 0; i < clump->geometryList.size(); i++)
				sanityCheck(&clump->geometryList[i]);

			if(cleanflag)
				for(uint32 i = 0; i < clump->geometryList.size(); i++)
					clump->geometryList[i].cleanUp();

			if(fixmatflag)
				fixpipeline(clump, type);

			if(dumpflag)
				clump->dump(dumpflag > 1);
		
			clump->write(out);
			delete clump;
		}else if(header.type == CHUNK_UVANIMDICT){
			in.seekg(-12, ios::cur);
			UVAnimDict *uvd = new UVAnimDict;
			uvd->read(in);
			uvd->write(out);
			delete uvd;
		}else
			in.seekg(header.length);
	}
	out.close();
	in.close();

	return 0;
}
