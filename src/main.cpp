// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //


/* This is a small example tutorial how to use OSPRay in an application.
 *
 * On Linux build it in the build_directory with
 *   g++ ../apps/ospTutorial.cpp -I ../ospray/include -I .. -I ../ospray/embree/common ./libospray.so -Wl,-rpath,. -o ospTutorial
 * On Windows build it in the build_directory\$Configuration with
 *   cl ..\..\apps\ospTutorial.cpp /EHsc -I ..\..\ospray\include -I ..\.. -I ..\..\ospray\embree\common ospray.lib
 */

#include <zmq.h>
#include <czmq.h>
#include <iostream>
#include <map>
#include <memory>

#include <rsi/rsi.h>

#include <util/empty_delete.h>
#include <util/jpeg.h>


#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#  include <malloc.h>
#else
#  include <alloca.h>
#endif

#include <common/miniSG/miniSG.h>
#include <ospray/ospray.h>
#include <common/commandline/Utility.h>
#include <ospray_cpp/Camera.h>
#include <ospray_cpp/Model.h>
#include <ospray_cpp/Renderer.h>
#include <ospray_cpp/Data.h>


#include <common/commandline/CameraParser.h>
#include <common/commandline/LightsParser.h>
#include <common/commandline/SceneParser/MultiSceneParser.h>
#include <common/commandline/RendererParser.h>

#include <tuple>
#include <type_traits>

#undef PING

#include <util/timer.h>
#include <util/string.h>

#include <gzstream/gzstream.h>





using namespace ospray;



inline void parseForLoadingModules2(int ac, const char**& av)
{
  for (int i = 1; i < ac; i++) {
    const std::string arg = av[i];
    if (arg == "--module" || arg == "-m") {
      ospLoadModule(av[++i]);
    }
  }
}


template <typename RendererParser_T,
          typename CameraParser_T,
          typename SceneParser_T,
          typename LightsParser_T>
inline ParsedOSPObjects parseCommandLine2(int ac, const char **&av)
{
  static_assert(std::is_base_of<RendererParser, RendererParser_T>::value,
                "RendererParser_T is not a subclass of RendererParser.");
  static_assert(std::is_base_of<CameraParser, CameraParser_T>::value,
                "CameraParser_T is not a subclass of CameraParser.");
  static_assert(std::is_base_of<SceneParser, SceneParser_T>::value,
                "SceneParser_T is not a subclass of SceneParser.");
  static_assert(std::is_base_of<LightsParser, LightsParser_T>::value,
                "LightsParser_T is not a subclass of LightsParser.");

  parseForLoadingModules2(ac, av);

  //CameraParser_T cameraParser;
  //cameraParser.parse(ac, av);
  //auto camera = cameraParser.camera();
  ospray::cpp::Camera camera;

  RendererParser_T rendererParser;
  rendererParser.parse(ac, av);
  auto renderer = rendererParser.renderer();

  SceneParser_T sceneParser{rendererParser.renderer()};
  sceneParser.parse(ac, av);
  auto model = sceneParser.model();
  auto bbox  = sceneParser.bbox();

  //ospcommon::box3f    bbox;
  //ospray::cpp::Model model;
  //model.commit();


  //LightsParser_T lightsParser(renderer);
  //lightsParser.parse(ac, av);

  return std::make_tuple(bbox, model, renderer, camera);
}

inline ParsedOSPObjects parseWithDefaultParsers2(int ac, const char**& av)
{
  return parseCommandLine2<DefaultRendererParser, DefaultCameraParser,
                          MultiSceneParser, DefaultLightsParser>(ac, av);
}


// Three-index vertex, indexing start at 0, -1 means invalid vertex
struct FaceVertex
{
	int v, vt, vn;
	FaceVertex() {}
	FaceVertex(int v) : v(v), vt(v), vn(v) {}
	FaceVertex(int v, int vt, int vn) : v(v), vt(vt), vn(vn) {}
};

static inline bool operator < ( const FaceVertex& a, const FaceVertex& b )
{
	if (a.v  != b.v)  return a.v  < b.v;
	if (a.vn != b.vn) return a.vn < b.vn;
	if (a.vt != b.vt) return a.vt < b.vt;
	return false;
}


struct BOBJLoader
{
	ospray::miniSG::Model &model;
	std::map<std::string,ospray::miniSG::Material *> material;


	BOBJLoader(ospray::miniSG::Model &model, const ospcommon::FileName& fileName):
		model(model),
		path(fileName.path()),
		curMaterial(nullptr)
	{
		igzstream in( fileName.c_str(), std::ios::in );

		int numVertices = 0;
		in.read( (char*)&numVertices, sizeof(int) );
		for( int i=0;i<numVertices;++i )
		{
			float x, y, z;
			in.read( (char*)&x, sizeof(float) );
			in.read( (char*)&y, sizeof(float) );
			in.read( (char*)&z, sizeof(float) );
			//v.push_back(ospcommon::vec3f(x, y, z));
			v.push_back(ospcommon::vec3f(x, z, y));
		}

		int numNormals = 0;
		in.read( (char*)&numNormals, sizeof(int) );
		for( int i=0;i<numNormals;++i )
		{
			float x, y, z;
			in.read( (char*)&x, sizeof(float) );
			in.read( (char*)&y, sizeof(float) );
			in.read( (char*)&z, sizeof(float) );
			//vn.push_back(ospcommon::vec3f(x, y, z));
		}

		int numTriangles = 0;
		in.read( (char*)&numTriangles, sizeof(int) );
		for( int i=0;i<numTriangles;++i )
		{
			int i0, i1, i2;
			in.read( (char*)&i0, sizeof(int) );
			in.read( (char*)&i1, sizeof(int) );
			in.read( (char*)&i2, sizeof(int) );

			std::vector<FaceVertex> face;
			face.push_back(FaceVertex(i0, -1, -1));
			face.push_back(FaceVertex(i1, -1, -1));
			face.push_back(FaceVertex(i2, -1, -1));
			curGroup.push_back(face);
		}
		flushFaceGroup();



		std::cout << "numVertices=" << numVertices << std::endl;
		std::cout << "numNormals=" << numNormals << std::endl;
		std::cout << "numTriangles=" << numTriangles << std::endl;

	}
	~BOBJLoader()
	{

	}
private:

	void flushFaceGroup()
	{
		if (curGroup.empty()) return;

		std::map<FaceVertex, uint32_t> vertexMap;
		ospray::miniSG::Mesh *mesh = new ospray::miniSG::Mesh;
		model.mesh.push_back(mesh);
		model.instance.push_back(ospray::miniSG::Instance(model.mesh.size()-1));
		mesh->material = curMaterial;

		// merge three indices into one
		for (size_t j=0; j < curGroup.size(); ++j)
		{
			// iterate over all faces
			const std::vector<FaceVertex>& face = curGroup[j];
			FaceVertex i0 = face[0], i1 = FaceVertex(-1), i2 = face[1];

			// triangulate the face with a triangle fan
			for (size_t k=2; k < face.size(); k++)
			{
				i1 = i2; i2 = face[k];
				int32_t v0 = getVertex(vertexMap, mesh, i0);
				int32_t v1 = getVertex(vertexMap, mesh, i1);
				int32_t v2 = getVertex(vertexMap, mesh, i2);
				if (v0 < 0 || v1 < 0 || v2 < 0)
					continue;
				ospray::miniSG::Triangle tri;
				tri.v0 = v0;
				tri.v1 = v1;
				tri.v2 = v2;
				mesh->triangle.push_back(tri);
			}
		}

		curGroup.clear();
	}


	uint32_t getVertex(std::map<FaceVertex,uint32_t>& vertexMap, ospray::miniSG::Mesh *mesh, const FaceVertex& i)
	{
		const std::map<FaceVertex, uint32_t>::iterator& entry = vertexMap.find(i);
		if (entry != vertexMap.end())
			return(entry->second);

		if (std::isnan(v[i.v].x) || std::isnan(v[i.v].y) || std::isnan(v[i.v].z))
			return -1;

		if (i.vn >= 0 && (std::isnan(vn[i.vn].x) ||
			std::isnan(vn[i.vn].y) ||
			std::isnan(vn[i.vn].z)))
			return -1;

		if (i.vt >= 0 && (std::isnan(vt[i.vt].x) ||
			std::isnan(vt[i.vt].y)))
			return -1;

		mesh->position.push_back(v[i.v]);
		if (i.vn >= 0)
			mesh->normal.push_back(vn[i.vn]);
		if (i.vt >= 0)
			mesh->texcoord.push_back(vt[i.vt]);

		return(vertexMap[i] = int(mesh->position.size()) - 1);
	}

	ospcommon::FileName path;

	// Geometry buffer ---
	std::vector<ospcommon::vec3f> v;
	std::vector<ospcommon::vec3f> vn;
	std::vector<ospcommon::vec2f> vt;
	std::vector<std::vector<FaceVertex> > curGroup;

	// Material handling ---
	ospray::miniSG::Material *curMaterial;
	ospray::miniSG::Material *defaultMaterial;

	// Internal methods ---




};


// SERIALIZATION HELPERS =====================================================

// here we give very simple binary buffers for serialization and deserialization
// these are used to pack scene manipulation commands into binary messages which
// can go over the wire

struct OutputFile
{
    OutputFile( const std::string& filename ):file(filename.c_str(), std::ios::out | std::ios::binary)
    {
    }

    void write_int( const int& value )
    {
        file.write( reinterpret_cast<const char*>(&value), sizeof(int) );
    }

    void write_float( const float& value )
    {
        file.write( reinterpret_cast<const char*>(&value), sizeof(float) );
    }

    void write_string( const std::string& str )
    {
        int size = str.size();
        write_int( size );
        file.write( reinterpret_cast<const char*>(&str[0]), size );
    }

    void write_data( const char* ptr, int size )
    {
        write_int( size );
        file.write( ptr, size );
    }


 private:
    std::ofstream file;
};


struct InputFile
{
    InputFile( const std::string& filename ):
    	file(filename.c_str(), std::ios::in | std::ios::binary)
    {
    }

    int read_int()
    {
    	int value = -1;
    	file.read( (char*)&value, sizeof(int));
    	return value;
    }

    float read_float()
    {
    	float value = -1.0f;
    	file.read( (char*)&value, sizeof(float));
    	return value;
    }

    std::string read_string()
    {
    	int size = read_int();
    	std::string str(size, ' ');

    	file.read( (char*)&str[0], size);

    	return str;
    }

    std::shared_ptr<void> read_data()
    {
    	int size = read_int();
    	std::shared_ptr<void> data( malloc(size), free );
    	file.read( (char*)data.get(), size );
    	return data;
    }

    void read_data( char *data, int max_size )
    {
    	int size = read_int();
    	if( max_size != size )
    	{
    		std::cout << "InputFile::read_data error: sizes dont match!\n";
    		throw std::runtime_error("InputFile::read_data error: sizes dont match!");
    		return;
    	}
    	file.read( data, size );
    }

private:
	std::ifstream file;
};


struct MiniSGExporter
{
	std::map<ospray::miniSG::Material*, int> materialref_to_index;
	std::map<ospray::miniSG::Texture2D*, int> textureref_to_index;

	std::string paramTypeToString( ospray::miniSG::Material::Param::DataType type )
	{
		switch( type )
		{
			case ospray::miniSG::Material::Param::INT:return "INT";break;
			case ospray::miniSG::Material::Param::INT_2:return "INT_2";break;
			case ospray::miniSG::Material::Param::INT_3:return "INT_3";break;
			case ospray::miniSG::Material::Param::INT_4:return "INT_4";break;
			case ospray::miniSG::Material::Param::UINT:return "UINT";break;
			case ospray::miniSG::Material::Param::UINT_2:return "UINT_2";break;
			case ospray::miniSG::Material::Param::UINT_3:return "UINT_3";break;
			case ospray::miniSG::Material::Param::UINT_4:return "UINT_4";break;
			case ospray::miniSG::Material::Param::FLOAT:return "FLOAT";break;
			case ospray::miniSG::Material::Param::FLOAT_2:return "FLOAT_2";break;
			case ospray::miniSG::Material::Param::FLOAT_3:return "FLOAT_3";break;
			case ospray::miniSG::Material::Param::FLOAT_4:return "FLOAT_4";break;
			case ospray::miniSG::Material::Param::TEXTURE:return "TEXTURE";break;
			case ospray::miniSG::Material::Param::STRING:return "STRING";break;
			default:
			case ospray::miniSG::Material::Param::UNKNOWN:return "UNKNOWN";break;
		};
		return "unknown";
	}
	OutputFile out;
	MiniSGExporter( const std::string& filename, ospray::miniSG::Model &model ):
		out(filename)
	{
		std::cout << "exporting miniSG model " << filename << std::endl;
		std::cout << "numMeshes=" << model.numMeshes() << std::endl;
		std::cout << "numInstances=" << model.instance.size() << std::endl;
		


		// we collect all materials and textures...
		std::vector<ospray::miniSG::Material*> materials;
		std::vector<ospray::miniSG::Texture2D*> textures;
		
		// here find all materials from meshes
		for( int i=0, numMeshes=model.numMeshes();i<numMeshes;++i )
		{
			if( model.mesh[i]->material )
			{
				ospray::miniSG::Material* mat = model.mesh[i]->material.ptr;
				auto it = materialref_to_index.find( mat );
				if( it == materialref_to_index.end() )
				{
					materials.push_back( mat );
					materialref_to_index[mat] = materials.size()-1;
				}
			}
		}

		// here we find all textures from materials
		for( int i=0, numMaterials = materials.size();i<numMaterials;++i )
		{
			ospray::miniSG::Material* mat = materials[i];
			// now iterate all parameters and look for textures
			for( auto it :mat->params )
			{
				ospcommon::Ref<ospray::miniSG::Material::Param> param = it.second;
				if(param->type == ospray::miniSG::Material::Param::TEXTURE)
				{
					if( param->ptr != nullptr )
					{
						ospray::miniSG::Texture2D* tex = (ospray::miniSG::Texture2D*)param->ptr;
						auto it = textureref_to_index.find( tex );
						if( it == textureref_to_index.end() )
						{
							textures.push_back( tex );
							textureref_to_index[tex] = textures.size()-1;
						}						
					}
				}
			}
		}


		std::cout << "numMaterials=" << materials.size() << std::endl;
		std::cout << "numTextures=" << textures.size() << std::endl;


		// export all textures ---
		out.write_int(textures.size());
		for( int i=0, numTextures = textures.size();i<numTextures;++i )
			exportTexture( textures[i] );

		// export all materials ---
		out.write_int(materials.size());
		for( int i=0, numMaterials = materials.size();i<numMaterials;++i )
			exportMaterial( materials[i] );


		//export meshes --------------

		out.write_int(model.numMeshes());
		for( int i=0, numMeshes=model.numMeshes();i<numMeshes;++i )
			exportMesh(*model.mesh[i]);
		out.write_int( model.instance.size() );
		for( int i=0, numInstances=model.instance.size();i<numInstances;++i )
		{
			out.write_int(model.instance[i].meshID);
		}

	}

	void exportTexture( ospray::miniSG::Texture2D* tex )
	{
		out.write_int(tex->width);
		out.write_int(tex->height);
		out.write_int(tex->channels);
		out.write_int(tex->depth);
		out.write_int(tex->prefereLinear);
		int size = tex->width*tex->height*tex->channels*tex->depth;
		out.write_data((char*)tex->data, size);
	}

	void exportMaterial( ospray::miniSG::Material* mat )
	{
		out.write_string(mat->name);
		out.write_string(mat->type);
		out.write_int( mat->params.size() );
		for( auto it :mat->params )
		{
			std::string name = it.first;
			ospcommon::Ref<ospray::miniSG::Material::Param> param = it.second;

			out.write_string(name);
			out.write_int((int)param->type);

			switch( param->type )
			{
				case ospray::miniSG::Material::Param::INT:
				{
					out.write_int(param->i[0]);
				}break;
				case ospray::miniSG::Material::Param::INT_2:
				{
					out.write_int(param->i[0]);
					out.write_int(param->i[1]);
				}break;
				case ospray::miniSG::Material::Param::INT_3:
				{
					out.write_int(param->i[0]);
					out.write_int(param->i[1]);
					out.write_int(param->i[2]);
				}break;
				case ospray::miniSG::Material::Param::INT_4:
				{
					out.write_int(param->i[0]);
					out.write_int(param->i[1]);
					out.write_int(param->i[2]);
					out.write_int(param->i[3]);
				}break;
				case ospray::miniSG::Material::Param::UINT:
				{
					out.write_int(param->ui[0]);
				}break;
				case ospray::miniSG::Material::Param::UINT_2:
				{
					out.write_int(param->ui[0]);
					out.write_int(param->ui[1]);
				}break;
				case ospray::miniSG::Material::Param::UINT_3:
				{
					out.write_int(param->ui[0]);
					out.write_int(param->ui[1]);
					out.write_int(param->ui[2]);
				}break;
				case ospray::miniSG::Material::Param::UINT_4:
				{
					out.write_int(param->ui[0]);
					out.write_int(param->ui[1]);
					out.write_int(param->ui[2]);
					out.write_int(param->ui[3]);
				}break;
				case ospray::miniSG::Material::Param::FLOAT:
				{
					out.write_float(param->f[0]);
				}break;
				case ospray::miniSG::Material::Param::FLOAT_2:
				{
					out.write_float(param->f[0]);
					out.write_float(param->f[1]);
				}break;
				case ospray::miniSG::Material::Param::FLOAT_3:
				{
					out.write_float(param->f[0]);
					out.write_float(param->f[1]);
					out.write_float(param->f[2]);
				}break;
				case ospray::miniSG::Material::Param::FLOAT_4:
				{
					out.write_float(param->f[0]);
					out.write_float(param->f[1]);
					out.write_float(param->f[2]);
					out.write_float(param->f[3]);
				}break;
				case ospray::miniSG::Material::Param::TEXTURE:
				{
					ospray::miniSG::Texture2D* tex = (ospray::miniSG::Texture2D*)param->ptr;
					int tex_index = -1;
					auto it = textureref_to_index.find(tex);
					if( it != textureref_to_index.end() )
						tex_index = it->second;
					out.write_int(tex_index);
				}break;
				case ospray::miniSG::Material::Param::STRING:
				{
					std::string s = param->s;
					out.write_string(s);
				}break;
				default:
				case ospray::miniSG::Material::Param::UNKNOWN:
				{
				}break;
			}; // switch(param->type)
		} // for param
	}


	void exportMesh( ospray::miniSG::Mesh& mesh )
	{
		/*
		std::cout << "MiniSGExporter::exportMesh...\n";
		std::cout << "name=" << mesh.name << std::endl;;
		std::cout << "numPoints=" << mesh.position.size() << std::endl;
		std::cout << "numNormals=" << mesh.normal.size() << std::endl;
		std::cout << "numColors=" << mesh.color.size() << std::endl;
		std::cout << "numTexcoords=" << mesh.texcoord.size() << std::endl;
		std::cout << "numTriangles=" << mesh.triangle.size() << std::endl;
		std::cout << "numTriangleMaterialId=" << mesh.triangleMaterialId.size() << std::endl;
		std::cout << "numMaterials=" << mesh.materialList.size()<< std::endl;
		*/

		/*
		if(mesh.materialList.size()>0)
			std::cout << "got mesh with materiallist\n";
		if(mesh.triangleMaterialId.size()>0)
			std::cout << "got mesh with triangleMaterialId\n";
		if(mesh.material)
			std::cout << "got mesh with material\n";
		*/


		// name
		out.write_string( mesh.name );

		// position
		out.write_int( mesh.position.size() );
		if( !mesh.position.empty() )
			out.write_data( (char*)&mesh.position[0], mesh.position.size()*sizeof(ospcommon::vec3fa) );

		// normal
		out.write_int( mesh.normal.size() );
		if( !mesh.normal.empty() )
			out.write_data( (char*)&mesh.normal[0], mesh.normal.size()*sizeof(ospcommon::vec3fa) );

		// color
		out.write_int( mesh.color.size() );
		if( !mesh.color.empty() )
			out.write_data( (char*)&mesh.color[0], mesh.color.size()*sizeof(ospcommon::vec3fa) );

		// texcoord
		out.write_int( mesh.texcoord.size() );
		if( !mesh.texcoord.empty() )
			out.write_data( (char*)&mesh.texcoord[0], mesh.texcoord.size()*sizeof(ospcommon::vec2f) );

		// triangle
		out.write_int( mesh.triangle.size() );
		if( !mesh.triangle.empty() )
			out.write_data( (char*)&mesh.triangle[0], mesh.triangle.size()*sizeof(ospray::miniSG::Triangle) );

		// triangleMaterialId
		out.write_int( mesh.triangleMaterialId.size() );
		if( !mesh.triangleMaterialId.empty() )
			out.write_data( (char*)&mesh.triangleMaterialId[0], mesh.triangleMaterialId.size()*sizeof(uint32_t) );


		int material_index = -1;
		if(mesh.material)
			material_index = materialref_to_index[mesh.material.ptr];
		out.write_int(material_index);
	}

};

struct MiniSGImporter
{
	InputFile in;
	std::vector<ospcommon::Ref<ospray::miniSG::Material>> m_materials;
	std::vector<ospray::miniSG::Texture2D*> m_textures;


	MiniSGImporter( const std::string& filename, ospray::miniSG::Model &model ):
		in(filename)
	{
		std::cout << "importing miniSG model " << filename << std::endl;
		
		// import textures ---
		int numTextures = in.read_int();
		for( int i=0;i<numTextures;++i )
			m_textures.push_back(importTexture());

		// import materials ---
		int numMaterials = in.read_int();
		for( int i=0;i<numMaterials;++i )
			m_materials.push_back(importMaterial());


		//import meshes --------------
		int numMeshes = in.read_int();
		for( int i=0;i<numMeshes;++i )
			importMesh(model);
		int numInstances = in.read_int();
		for( int i=0;i<numInstances;++i )
			model.instance.push_back(ospray::miniSG::Instance( in.read_int() ));


		std::cout << "numMeshes=" << numMeshes << std::endl;
		std::cout << "numInstances=" << numInstances << std::endl;
		std::cout << "numMaterials=" << m_materials.size() << std::endl;
		std::cout << "numTextures=" << m_textures.size() << std::endl;
	}

	ospray::miniSG::Texture2D* importTexture()
	{
		ospray::miniSG::Texture2D* tex = new ospray::miniSG::Texture2D();

		tex->width = in.read_int();
		tex->height = in.read_int();
		tex->channels = in.read_int();
		tex->depth = in.read_int();
		tex->prefereLinear = in.read_int();
		int size = tex->width*tex->height*tex->channels*tex->depth;
		tex->data = malloc(size);
		in.read_data((char*)tex->data, size);

		return tex;
	}


	ospcommon::Ref<ospray::miniSG::Material> importMaterial()
	{
		ospcommon::Ref<ospray::miniSG::Material> mat = new ospray::miniSG::Material();

		mat->name = in.read_string();
		mat->type = in.read_string();
		int numParams = in.read_int();
		for( int i=0;i<numParams;++i )
		{
			//ospcommon::Ref<ospray::miniSG::Material::Param> param = new ospray::miniSG::Material::Param();
			ospray::miniSG::Material::Param* param = new ospray::miniSG::Material::Param();

			std::string name = in.read_string();
			param->type = (ospray::miniSG::Material::Param::DataType)in.read_int();

			switch( param->type )
			{
				case ospray::miniSG::Material::Param::INT:
				{
					param->i[0] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::INT_2:
				{
					param->i[0] = in.read_int();
					param->i[1] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::INT_3:
				{
					param->i[0] = in.read_int();
					param->i[1] = in.read_int();
					param->i[2] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::INT_4:
				{
					param->i[0] = in.read_int();
					param->i[1] = in.read_int();
					param->i[2] = in.read_int();
					param->i[3] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::UINT:
				{
					param->ui[0] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::UINT_2:
				{
					param->ui[0] = in.read_int();
					param->ui[1] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::UINT_3:
				{
					param->ui[0] = in.read_int();
					param->ui[1] = in.read_int();
					param->ui[2] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::UINT_4:
				{
					param->ui[0] = in.read_int();
					param->ui[1] = in.read_int();
					param->ui[2] = in.read_int();
					param->ui[3] = in.read_int();
				}break;
				case ospray::miniSG::Material::Param::FLOAT:
				{
					param->f[0] = in.read_float();
				}break;
				case ospray::miniSG::Material::Param::FLOAT_2:
				{
					param->f[0] = in.read_float();
					param->f[1] = in.read_float();
				}break;
				case ospray::miniSG::Material::Param::FLOAT_3:
				{
					param->f[0] = in.read_float();
					param->f[1] = in.read_float();
					param->f[2] = in.read_float();
				}break;
				case ospray::miniSG::Material::Param::FLOAT_4:
				{
					param->f[0] = in.read_float();
					param->f[1] = in.read_float();
					param->f[2] = in.read_float();
					param->f[3] = in.read_float();
				}break;
				case ospray::miniSG::Material::Param::TEXTURE:
				{
					int texture_index = in.read_int();
					if( texture_index >= 0 )
						param->ptr = m_textures[texture_index];
					else
						param->ptr = 0;
				}break;
				case ospray::miniSG::Material::Param::STRING:
				{
					std::string s = in.read_string();
					param->set(s.c_str());
				}break;
				default:
				case ospray::miniSG::Material::Param::UNKNOWN:
				{
				}break;
			}; // switch(param->type)

			//if(param->type == ospray::miniSG::Material::Param::TEXTURE)
			//	continue;

			mat->params[name] = param;
		} // for param



		return mat;
	}

	void importMesh(ospray::miniSG::Model &model)
	{
		ospray::miniSG::Mesh *mesh = new ospray::miniSG::Mesh();
		model.mesh.push_back(mesh);
		mesh->material = nullptr;

		// name
		mesh->name = in.read_string();
		
		// positions
		mesh->position.resize( in.read_int() );
		if( !mesh->position.empty() )
			in.read_data( (char*)&mesh->position[0], mesh->position.size()*sizeof(ospcommon::vec3fa) );

		// normal
		mesh->normal.resize( in.read_int() );
		if( !mesh->normal.empty() )
			in.read_data( (char*)&mesh->normal[0], mesh->normal.size()*sizeof(ospcommon::vec3fa) );

		// color
		mesh->color.resize( in.read_int() );
		if( !mesh->color.empty() )
			in.read_data( (char*)&mesh->color[0], mesh->color.size()*sizeof(ospcommon::vec3fa) );

		// texcoord
		mesh->texcoord.resize( in.read_int() );
		if( !mesh->texcoord.empty() )
			in.read_data( (char*)&mesh->texcoord[0], mesh->texcoord.size()*sizeof(ospcommon::vec2f) );

		// triangle
		mesh->triangle.resize( in.read_int() );
		if( !mesh->triangle.empty() )
			in.read_data( (char*)&mesh->triangle[0], mesh->triangle.size()*sizeof(ospray::miniSG::Triangle) );

		// triangleMaterialId
		mesh->triangleMaterialId.resize( in.read_int() );
		if( !mesh->triangleMaterialId.empty() )
			in.read_data( (char*)&mesh->triangleMaterialId[0], mesh->triangleMaterialId.size()*sizeof(uint32_t) );

		// material
		int material_index = in.read_int();
		if(material_index >= 0)
			mesh->material = m_materials[material_index];
		/*
		std::cout << "MiniSGExporter::importMesh...\n";
		std::cout << "mesh.name=" << mesh->name << std::endl;
		std::cout << "numPoints=" << mesh->position.size() << std::endl;
		std::cout << "numNormals=" << mesh->normal.size() << std::endl;
		std::cout << "numColors=" << mesh->color.size() << std::endl;
		std::cout << "numTexcoords=" << mesh->texcoord.size() << std::endl;
		std::cout << "numTriangles=" << mesh->triangle.size() << std::endl;
		std::cout << "numTriangleMaterialId=" << mesh->triangleMaterialId.size() << std::endl;
		std::cout << "numMaterials=" << mesh->materialList.size() << std::endl;
		*/
	}

};



struct OSPRRenderer : public IScene
{
	std::map<std::string, ospray::cpp::ManagedObject*> m_objects;
	
	std::vector<std::pair<std::string, ospray::cpp::Camera>> m_cameras;
	std::vector<std::pair<std::string, ospray::cpp::Light>> m_lights;
	

	ospray::cpp::Renderer renderer;
	ospray::cpp::FrameBuffer fb;
	osp::vec2i imgSize;

	OSPRRenderer(int argc, const char **argv)
	{
		counter = 0;
		Timer timer;


		timer.reset();
		timer.start();
		auto ospObjs = parseWithDefaultParsers2(argc, argv);
		timer.stop();
		std::cout << "parsing done. took " << timer.elapsedSeconds() << "s" << std::endl;std::flush(std::cout);


		ospray::cpp::Camera camera;
		ospcommon::box3f      bbox;
		ospray::cpp::Model   model;
		std::tie(bbox, model, renderer, camera) = ospObjs;


		// create default camera ---
		camera = ospray::cpp::Camera("perspective");
		camera.commit();


		renderer.set("world",  model);
		renderer.set("model",  model);
		renderer.set("camera", camera);

		int spp = 1;
		renderer.set("spp", spp);

		renderer.commit();

		// image size
		//imgSize.x = 4096; // width
		//imgSize.y = 4096; // height
		imgSize.x = 512; // width
		imgSize.y = 512; // height


		fb = ospray::cpp::FrameBuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM);
		fb.clear(OSP_FB_ACCUM);


		std::cout << "renderFrame about to start\n";std::flush(std::cout);
		std::cout << "resolution=" << imgSize.x << " " << imgSize.y << std::endl;;std::flush(std::cout);
		std::cout << "spp=" << spp << std::endl;;std::flush(std::cout);


		m_cameras.push_back(std::make_pair("camera", camera));
		updateObjectMap();
	}

	// this function is required to be called after any of the camera/light/etc. list changes
	// this is because m_objects holds pointers to the handles in the type lists and these can change
	// during reallocation...
	// TODO: move to our own managed objects...
	void updateObjectMap()
	{
		m_objects.clear();
		for( auto& p : m_cameras )
		{
			std::string name = p.first;
			ospray::cpp::Camera& cam = p.second;
			m_objects[ name ] = &cam;
		}
		for( auto& p : m_lights )
		{
			std::string name = p.first;
			ospray::cpp::Light& light = p.second;
			m_objects[ name ] = &light;
		}
	}

	void updateLightList()
	{
		std::vector<OSPLight> lights;
		for( auto& p : m_lights )
		{
			lights.push_back( p.second.handle() );
		}
		auto lightArray = ospray::cpp::Data(lights.size(), OSP_OBJECT, lights.data());
		renderer.set("lights", lightArray);
		renderer.commit();
	}

	ospray::cpp::ManagedObject* getObject( const std::string& id )
	{
		auto it = m_objects.find(id);
		if( it != m_objects.end() )
			return it->second;
		return 0;
	}

	ospray::cpp::Material createDefaultMaterial()
	{
		static auto ospMat = ospray::cpp::Material(nullptr);

		if (ospMat.handle())
			return ospMat;

		ospMat = renderer.newMaterial("OBJMaterial");

		//ospMat.set("Kd", .6f, 0.6f, 0.6f);
		ospMat.commit();
		return ospMat;
	}


	ospray::cpp::Material createMaterial(ospray::miniSG::Material *mat)
	{
		//return createDefaultMaterial();
		if (mat == nullptr) return createDefaultMaterial();


		static std::map<miniSG::Material *, cpp::Material> alreadyCreatedMaterials;

		//if (alreadyCreatedMaterials.find(mat) != alreadyCreatedMaterials.end())
		//{
		//	return alreadyCreatedMaterials[mat];
		//}

		const char *type = mat->getParam("type", "OBJMaterial");
		assert(type);

		cpp::Material ospMat;
		try
		{
			ospMat = alreadyCreatedMaterials[mat] = renderer.newMaterial(type);
		} catch (const std::runtime_error &)
		{
			//warnMaterial(type);
			return createDefaultMaterial();
		}

		const bool isOBJMaterial = !strcmp(type, "OBJMaterial");

		for (auto it =  mat->params.begin(); it !=  mat->params.end(); ++it)
		{
			const char *name = it->first.c_str();
			const miniSG::Material::Param *p = it->second.ptr;

			//std::cout << "\tmaterial parameter " << name << std::endl;
			

			switch(p->type)
			{
				case ospray::miniSG::Material::Param::INT:
				ospMat.set(name, p->i[0]);
				break;
				case ospray::miniSG::Material::Param::FLOAT:
				{
					float f = p->f[0];
					// many mtl materials of obj models wrongly store the phong exponent
					//'Ns' in range [0..1], whereas OSPRay's material implementations
					//correctly interpret it to be in [0..inf), thus we map ranges here
					if (isOBJMaterial &&
						(!strcmp(name, "Ns") || !strcmp(name, "ns")) &&
						f < 1.f)
					{
						f = 1.f/(1.f - f) - 1.f;
					}
					ospMat.set(name, f);
				} break;
				case ospray::miniSG::Material::Param::FLOAT_3:
					ospMat.set(name, p->f[0], p->f[1], p->f[2]);
					break;
				case ospray::miniSG::Material::Param::STRING:
					ospMat.set(name, p->s);
					break;
				case ospray::miniSG::Material::Param::TEXTURE:
				{
					ospray::miniSG::Texture2D *tex = (ospray::miniSG::Texture2D*)p->ptr;
					if (tex)
					{
						OSPTexture2D ospTex = ospray::miniSG::createTexture2D(tex);
						assert(ospTex);
						ospCommit(ospTex);
						ospMat.set(name, ospTex);
					}
				}break;
				default:
					throw std::runtime_error("unknown material parameter type");
			};
		}

		ospMat.commit();
		return ospMat;
	}

	// IScene implementation ------------------------
	virtual void message( const std::string& msg )override
	{
		if(msg == "loadSponza")
		{
			//loadObj( "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/crytek-sponza/sponza.obj" );
			loadMSG( "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/crytek-sponza/sponza.msg" );
		}else
		if( msg == "loadFluidsurface" )
		{
			//loadBObj( "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.bobj.gz" );
			loadMSG("/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.msg");
		}else
		if( msg == "loadSanMiguel" )
		{
			//loadObj( "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/san-miguel/sanMiguel.obj" );
			loadMSG( "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/san-miguel/sanMiguel.msg" );
		}
	}

	void loadBObj(const std::string filename)
	{
		std::cout << "loading bobj.gz " << filename << std::endl;
		ospcommon::Ref<ospray::miniSG::Model> msgModel(new ospray::miniSG::Model);
		BOBJLoader( *msgModel,  ospcommon::FileName(filename) );
		ospcommon::box3f bbox = msgModel.ptr->getBBox();
		std::cout << "bbox.lower=" <<bbox.lower.x << " " << bbox.lower.y << " " << bbox.lower.z << std::endl;
		std::cout << "bbox.upper=" <<bbox.upper.x << " " << bbox.upper.y << " " << bbox.upper.z << std::endl;
		setModel(msgModel);
	}

	void loadObj(const std::string filename)
	{
		std::cout << "loading obj " << filename << std::endl;
		ospcommon::Ref<ospray::miniSG::Model> msgModel(new ospray::miniSG::Model);
		ospray::miniSG::importOBJ(*msgModel, ospcommon::FileName(filename));
		setModel(msgModel);
	}

	void loadMSG(const std::string filename)
	{
		std::cout << "loading msg " << filename << std::endl;
		ospcommon::Ref<ospray::miniSG::Model> msgModel(new ospray::miniSG::Model);
		MiniSGImporter(filename, *msgModel);
		setModel(msgModel);
	}

	void setModel( ospcommon::Ref<ospray::miniSG::Model> msgModel )
	{
		std::cout << "setModel\n";
		std::string geometryType = "triangles";
		

		ospray::cpp::Model model = ospray::cpp::Model();
		for (size_t i=0;i<msgModel->mesh.size();i++)
		{
			ospcommon::Ref<ospray::miniSG::Mesh> msgMesh = msgModel->mesh[i];
			auto ospMesh = cpp::Geometry(geometryType);

			// add position array to mesh
			OSPData position = ospNewData(msgMesh->position.size(),
			                              OSP_FLOAT3A,
			                              &msgMesh->position[0]);
			ospMesh.set("position", position);

			// add triangle index array to mesh
			if (!msgMesh->triangleMaterialId.empty())
			{
			  OSPData primMatID = ospNewData(msgMesh->triangleMaterialId.size(),
			                                 OSP_INT,
			                                 &msgMesh->triangleMaterialId[0]);
			  ospMesh.set("prim.materialID", primMatID);
			}

			// add triangle index array to mesh
			OSPData index = ospNewData(msgMesh->triangle.size(),
			                           OSP_INT3,
			                           &msgMesh->triangle[0]);
			assert(msgMesh->triangle.size() > 0);
			ospMesh.set("index", index);

			// add normal array to mesh
			if (!msgMesh->normal.empty())
			{
			  OSPData normal = ospNewData(msgMesh->normal.size(),
			                              OSP_FLOAT3A,
			                              &msgMesh->normal[0]);
			  assert(msgMesh->normal.size() > 0);
			  ospMesh.set("vertex.normal", normal);
			}

			// add color array to mesh
			if (!msgMesh->color.empty())
			{
			  OSPData color = ospNewData(msgMesh->color.size(),
			                             OSP_FLOAT3A,
			                             &msgMesh->color[0]);
			  assert(msgMesh->color.size() > 0);
			  ospMesh.set("vertex.color", color);
			}

			// add texcoord array to mesh
			if (!msgMesh->texcoord.empty())
			{
			  OSPData texcoord = ospNewData(msgMesh->texcoord.size(),
			                                OSP_FLOAT2,
			                                &msgMesh->texcoord[0]);
			  assert(msgMesh->texcoord.size() > 0);
			  ospMesh.set("vertex.texcoord", texcoord);
			}

			ospMesh.set("alpha_type", 0);
			ospMesh.set("alpha_component", 4);

			// handle materials -----------------
			// add triangle material id array to mesh
			if (msgMesh->materialList.empty())
			//if(1)
			{
				//std::cout << "single material\n";
				// we have a single material for this mesh...
				auto singleMaterial = createMaterial(msgMesh->material.ptr);
				//auto singleMaterial = createDefaultMaterial(renderer);
				ospMesh.setMaterial(singleMaterial);
			} else
			{
				//std::cout << "material list\n";
				// we have an entire material list, assign that list
				std::vector<OSPMaterial> materialList;
				std::vector<OSPTexture2D> alphaMaps;
				std::vector<float> alphas;
				for (size_t i = 0; i < msgMesh->materialList.size(); i++)
				{
					auto m = createMaterial(msgMesh->materialList[i].ptr);
					auto handle = m.handle();
					materialList.push_back(handle);

					for (ospray::miniSG::Material::ParamMap::const_iterator it = msgMesh->materialList[i]->params.begin();
						 it != msgMesh->materialList[i]->params.end();
						 it++)
					{
						const char *name = it->first.c_str();
						const ospray::miniSG::Material::Param *p = it->second.ptr;
						if(p->type == miniSG::Material::Param::TEXTURE)
						{
							if(!strcmp(name, "map_kd") || !strcmp(name, "map_Kd"))
							{
								ospray::miniSG::Texture2D *tex = (ospray::miniSG::Texture2D*)p->ptr;
								OSPTexture2D ospTex = createTexture2D(tex);
								ospCommit(ospTex);
								alphaMaps.push_back(ospTex);
							}
						} else
						if(p->type == ospray::miniSG::Material::Param::FLOAT)
						{
							if(!strcmp(name, "d"))
								alphas.push_back(p->f[0]);
						}
					}

					while(materialList.size() > alphaMaps.size())
					{
						alphaMaps.push_back(nullptr);
					}
					while(materialList.size() > alphas.size())
					{
						alphas.push_back(0.f);
					}
				}
				auto ospMaterialList = cpp::Data(materialList.size(), OSP_OBJECT, &materialList[0]);
				ospMesh.set("materialList", ospMaterialList);

				/*
				// only set these if alpha aware mode enabled
				// this currently doesn't work on the MICs!
				if(alpha)
				{
					auto ospAlphaMapList = cpp::Data(alphaMaps.size(), OSP_OBJECT, &alphaMaps[0]);
					ospMesh.set("alpha_maps", ospAlphaMapList);

					auto ospAlphaList = cpp::Data(alphas.size(), OSP_OBJECT, &alphas[0]);
					ospMesh.set("alphas", ospAlphaList);
				}
				*/
			}

			ospMesh.commit();

			model.addGeometry(ospMesh);
		}
		std::cout << "setModel done\n";
		model.commit();

		renderer.set("world",  model);
		renderer.set("model",  model);
		renderer.commit();

		fb.clear(OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
		



/*
		std::string name = "hdri light";

		ospray::cpp::ManagedObject* object = getObject( name );
		if( object )
		{
			std::cout << "got object!!!!!!!\n";
			std::string HDRI_file_name = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/lightProbes/rnl_probe.pfm";
			ospcommon::FileName imageFile(HDRI_file_name.c_str());
			ospray::miniSG::Texture2D *lightMap = ospray::miniSG::loadTexture(imageFile.path(), imageFile.base());
		    if (lightMap == NULL){
		      std::cout << "Failed to load hdri-light texture '" << imageFile << "'" << std::endl;
		    } else {
		      std::cout << "Successfully loaded hdri-light texture '" << imageFile << "'" << std::endl;
		    }
		    OSPTexture2D ospLightMap = ospray::miniSG::createTexture2D(lightMap);
			object->set( "map", ospLightMap);
			object->commit();
		}

		fb.clear(OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
*/
	}
	virtual void setAttr( const std::string& object_handle, const Attribute* attr_list, int nattrs )override
	{
		for( int i=0;i<nattrs;++i )
		{
			const Attribute& attr = attr_list[i];
			std::cout << "OSPRRenderer::setAttr: object=" << object_handle << " attr=" << attr.m_name;

			switch(attr.m_type)
			{
				case Attribute::EType::EString:
					std::cout << " value=" << getString(attr, 0) << std::endl;
					break;
				case Attribute::EType::EFloat:
					std::cout << " value=" << attr.ptr<float>()[0] << std::endl;
					break;
				case Attribute::EType::EP3f:
				case Attribute::EType::EV3f:
				case Attribute::EType::EC3f:
				case Attribute::EType::EN3f:
					std::cout << " value=" << attr.ptr<float>()[0] << " " << attr.ptr<float>()[1] << " " << attr.ptr<float>()[2] << std::endl;
					break;
				case Attribute::EType::EM44f:
					std::cout << " value=" << attr.ptr<float>()[0] << " " << attr.ptr<float>()[1] << " " << attr.ptr<float>()[2] << " " << attr.ptr<float>()[3];
					std::cout << " " << attr.ptr<float>()[4] << " " << attr.ptr<float>()[5] << " " << attr.ptr<float>()[6] << " " << attr.ptr<float>()[7];
					std::cout << " " << attr.ptr<float>()[8] << " " << attr.ptr<float>()[9] << " " << attr.ptr<float>()[10] << " " << attr.ptr<float>()[11];
					std::cout << " " << attr.ptr<float>()[12] << " " << attr.ptr<float>()[13] << " " << attr.ptr<float>()[14] << " " << attr.ptr<float>()[15] << std::endl;
					break;
			}

			ospray::cpp::ManagedObject* object = getObject( object_handle );
			if( object )
			{
				if( object_handle == "camera" && attr.m_name == "xform" )
				{
					std::cout << "setting transform matrix....\n";
					// ospray camera expects position, viewing direction and up vector which we extract from the transformation matrix
					object->set("pos", ospcommon::vec3f(attr.ptr<float>()[12], attr.ptr<float>()[13], attr.ptr<float>()[14]));
					object->set("dir", ospcommon::vec3f(attr.ptr<float>()[8], attr.ptr<float>()[9], attr.ptr<float>()[10]) );
					object->set("up", ospcommon::vec3f(attr.ptr<float>()[4], attr.ptr<float>()[5], attr.ptr<float>()[6]) );
				}else
				{
					switch(attr.m_type)
					{
						case Attribute::EType::EString:
						{
							// ospray objects can have special parameter types such as textures.
							// Our attributes only support basic types such as strings, floats etc.
							// the solution is, to use string attributes with some markup to encode these special parameters
							// for example:
							// "texture2d:/path/to/texture" will cause a texture load and setting the texture object
							std::string value = getString(attr, 0);
							std::vector<std::string> strings;
							splitString( value, strings, ":" );

							if( strings.size() == 1 )
								// ordinary string
								object->set( attr.name(), value );
							else
							{
								std::string& type = strings[0];
								std::string& value = strings[1];

								if(type == "texture2d")
								{
									ospcommon::FileName imageFile(value.c_str());
									ospray::miniSG::Texture2D *texture2d = ospray::miniSG::loadTexture(imageFile.path(), imageFile.base());
									if (texture2d == NULL)
									{
										std::cout << "Failed to load hdri-light texture '" << imageFile << "'" << std::endl;
									}else
									{
										std::cout << "Successfully loaded hdri-light texture '" << imageFile << "'" << std::endl;
									}
		    						OSPTexture2D ospTexture2d = ospray::miniSG::createTexture2D(texture2d);
		    						object->set( attr.name(), ospTexture2d);
								}
							}
						}break;
						case Attribute::EType::EFloat:
							object->set( attr.name(), attr.ptr<float>()[0] );
							break;
						case Attribute::EType::EP3f:
						case Attribute::EType::EV3f:
						case Attribute::EType::EC3f:
						case Attribute::EType::EN3f:
							object->set( attr.name(), attr.ptr<float>()[0], attr.ptr<float>()[1], attr.ptr<float>()[2] );
							break;
						case Attribute::EType::EM44f:
							break;
					}
				}
				object->commit();
			}else
			{
				std::cout << "OSPRRenderer::setAttr: error: object " << object_handle << " not found!\n";
			}

		}

		fb.clear(OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
	}

	virtual void create( const std::string& type, const std::string& object_handle )override
	{
		std::cout << "OSPRRenderer::create: type="  << type << " handle=" << object_handle << std::endl;
		bool somethingCreated = true;
		if( type == "SphereLight" )
		{
			auto ospLight = renderer.newLight("sphere");
			ospLight.set("name", object_handle);
			m_lights.push_back(std::make_pair(object_handle, ospLight));
			updateLightList();
		}else
		if( type == "DirectionalLight" )
		{
			auto ospLight = renderer.newLight("directional");
			ospLight.set("name", object_handle);
			m_lights.push_back(std::make_pair(object_handle, ospLight));
			updateLightList();
		}else
		if( type == "HDRILight" )
		{
			auto ospLight = renderer.newLight("hdri");
			//ospLight.set("name", object_handle);
			m_lights.push_back(std::make_pair(object_handle, ospLight));
			updateLightList();
		}else
		{
			somethingCreated = false;
		}

		if(somethingCreated)
		{
			fb.clear(OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
			updateObjectMap();
		}
	}

	virtual void remove( const std::string& object_handle )override
	{
		std::cout << "OSPRRenderer::delete: handle=" << object_handle << std::endl;
		ospray::cpp::ManagedObject* object = getObject( object_handle );

		if(object)
		{
			// find object in light list
			//for( int i=0,numLights=m_lights.size();i<numLights;++i )
			for( auto it = m_lights.begin();it!=m_lights.end();++it )
			{
				ospray::cpp::Light* light = &it->second;
				if( light == object )
				{
					m_lights.erase(it);
					std::cout << "object removed\n";
					break;
				}
			}

			updateLightList();
			updateObjectMap();
			fb.clear(OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
		}else
		{
			std::cout << "OSPRRenderer::delete: error object " << object_handle << " not found\n";
		}

	}



	void advance()
	{
		Timer timer;
		timer.reset();
		timer.start();
		renderer.renderFrame(fb, OSP_FB_COLOR | OSP_FB_ACCUM);
		timer.stop();
		//std::cout << "render frame took " << timer.elapsedSeconds() << "s\n";
		++counter;
	}

	int counter;
private:

};


OSPRRenderer* g_osprRenderer;











// helper function to write the rendered image as PPM file
void writePPM(const char *fileName,
			  const osp::vec2i &size,
			  const uint32_t *pixel)
{
  FILE *file = fopen(fileName, "wb");
  fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
  unsigned char *out = (unsigned char *)alloca(3*size.x);
  for (int y = 0; y < size.y; y++) {
	const unsigned char *in = (const unsigned char *)&pixel[(size.y-1-y)*size.x];
	for (int x = 0; x < size.x; x++)
	{
		out[3*x + 0] = in[4*x + 0];
		out[3*x + 1] = in[4*x + 1];
		out[3*x + 2] = in[4*x +2 ];
	}
	fwrite(out, 3*size.x, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}


void error(const std::string &msg)
{
  std::cout << "#ospModelViewer fatal error : " << msg << std::endl;
  std::cout << std::endl;
  std::cout << "Proper usage: " << std::endl;
  std::cout << "  ./ospModelViewer [-bench <warmpup>x<numFrames>] [-model] <inFileName>" << std::endl;
  std::cout << std::endl;
  exit(1);
}



static void *client_task (void *args)
{
    zctx_t *ctx = zctx_new ();
    void *client = zsocket_new (ctx, ZMQ_DEALER);

    //  Set random identity to make tracing easier
    //char identity [10];
    //sprintf (identity, "%04X-%04X", randof (0x10000), randof (0x10000));
    //zsocket_set_identity (client, identity);
    std::string identity = "head";
    zsocket_set_identity (client, identity.c_str());
    //zsocket_connect (client, "tcp://localhost:5570");
    int result = zsocket_connect (client, "tcp://193.196.155.55:5570");
    if(!result)
        std::cout << "connected to server...\n";

    zmq_pollitem_t items [] = { { client, 0, ZMQ_POLLIN, 0 } };
    int request_nbr = 0;
    int counter = 0;
    while (true)
    {
        //  Tick once per second, pulling in arriving messages
        int centitick;
        //int count = 100;
        int count = 50;
        for (centitick = 0; centitick < count; centitick++)
        {
            // ??
            //zmq_poll (items, 1, 10 * ZMQ_POLL_MSEC);
            zmq_poll (items, 1, 0);
            if (items [0].revents & ZMQ_POLLIN)
            {
                // receive message from server
                zmsg_t *msg = zmsg_recv (client);

                // we expect messages of the following form:
                //<int: opcode> <binary data chunk>
                // this will be routed directly to the execute function of our scene interface
                // note that there is no identity frame here

                
                zframe_t* content = zmsg_last (msg);

                // debug prints
                //std::cout << "received message !!!!!!!!!!!!!!!!!!!!!\n";
                //std::string test_str = std::string( (char*)zframe_data(content),  zframe_size (content));
                //std::cout << "message from downstream: id=" << identity << " msg=" << test_str << std::endl;


                // execute the binary chunk with our render server---
                Command command;
                command.data = std::shared_ptr<void>((char*)zframe_data(content), empty_delete<void>());
                command.size = zframe_size (content);
                //execute(&g_simpleScene, command);
                execute(g_osprRenderer, command);

                zmsg_destroy (&msg);
            }
        }

        // send a request to the server (this would be the rendered image)
        //std::cout << "client:sending request\n";
        std::string image = ">                    <";
        image[counter++%(image.size()-2) + 1] = 'o';
        //zstr_sendf (client, image.c_str());


        // render image 
        g_osprRenderer->advance();

        // access rendered image, convert to ldr and compress to jpeg
        {
//            int xres, yres;
//            float* data_hdr = g_simpleRenderer.getRGBData(xres, yres);
//            int numPixels = xres*yres;
//            std::vector<unsigned char> data_ldr( numPixels*3 );

//            std::transform( data_hdr, data_hdr + numPixels*3,
//                            data_ldr.begin(),
//                [](float value_linear)
//                {
//                    float value_srgb = 0;
//                    if (value_linear <= 0.0031308f)
//                        value_srgb = 12.92f * value_linear;
//                    else
//                        value_srgb = (1.0f + 0.055f)*std::pow(value_linear, 1.0f/2.4f) -  0.055f;

//                    return (unsigned char)(std::max( 0.0, std::min(255.0, value_srgb*255.0) ));
//                });


            // --------------------------------------
            //TODO: convert srgba to srgb and write to memory
            uint32_t *p = (uint32_t*)g_osprRenderer->fb.map(OSP_FB_COLOR);
            int xres = g_osprRenderer->imgSize.x;
            int yres = g_osprRenderer->imgSize.y;
            int numPixels = xres*yres;
            std::vector<unsigned char> data_ldr( numPixels*3 );

            unsigned char* ptr_dst = (unsigned char*)&data_ldr[0];
			unsigned char* ptr_src = (unsigned char*)p;

			for (int y = 0; y < yres; ++y)
			{
				const unsigned char *in = &ptr_src[(yres-1-y)*xres*4];
				unsigned char* out = &ptr_dst[y*xres*3];

				for (int x = 0; x < xres; ++x)
				{
					out[3*x + 0] = in[4*x + 0];
					out[3*x + 1] = in[4*x + 1];
					out[3*x + 2] = in[4*x + 2];
				}
			}

			g_osprRenderer->fb.unmap(p);

			//std::string filename = "images/test_" + zeroPadNumber(g_osprRenderer->counter) + ".jpg";
			//write_jpeg_to_file( filename.c_str(), xres, yres, &data_ldr[0] );
            write_jpeg_to_memory( xres, yres, &data_ldr[0], image );


            zmsg_t* msg = zmsg_new();
            zframe_t* frame = zframe_new(&image[0], image.size());
            zmsg_append( msg, &frame );
            zmsg_send( &msg, client );
        }

    }
    zctx_destroy (&ctx);
    return NULL;
}


int main(int argc, const char **argv)
{

	// initialize OSPRay; OSPRay parses (and removes) its commandline parameters, e.g. "--osp:debug"
	ospInit(&argc, argv);
	std::cout << "ospInit done\n";std::flush(std::cout);


	g_osprRenderer = new OSPRRenderer(argc, argv);

	/*
	std::string exportFile;
	{
		ospcommon::Ref<ospray::miniSG::Model> msgModel(new ospray::miniSG::Model);



		// fluidsurface ---
		//std::string importFile = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.bobj.gz";
		//exportFile = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.msg";

		// sponza ---
		//std::string importFile = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/crytek-sponza/sponza.obj";
		//exportFile = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/crytek-sponza/sponza.msg";

		// san-miguel ---
		std::string importFile = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/san-miguel/sanMiguel.obj";
		exportFile = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/san-miguel/sanMiguel.msg";


		Timer timer;
		timer.reset();
		timer.start();

		//BOBJLoader( *msgModel,  ospcommon::FileName(importFile) );
		ospray::miniSG::importOBJ(*msgModel, ospcommon::FileName(importFile));

		timer.stop();
		std::cout << "importOBJ took " << timer.elapsedSeconds() << "s\n";
		MiniSGExporter exportMiniSG(exportFile, *msgModel);

	}
	std::cout << "done export\n";
	{
		ospcommon::Ref<ospray::miniSG::Model> msgModel(new ospray::miniSG::Model);

		// fluidsurface ---
		//MiniSGImporter importMiniSG("/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.msg", *msgModel);

		Timer timer;
		timer.reset();
		timer.start();
		MiniSGImporter importMiniSG(exportFile, *msgModel);
		timer.stop();
		std::cout << "importMSG took " << timer.elapsedSeconds() << "s\n";
		g_osprRenderer->setModel( msgModel );
	}

	return 0;
	*/


/*
	// save screenshot
	std::string basename = "firstFrame";
	const uint32_t *p = (uint32_t*)fb.map(OSP_FB_COLOR);
	writePPM(std::string(basename+".ppm").c_str(), imgSize, p);
	std::cout << "#ospGlutViewer: saved current frame to '" << basename << ".ppm" << std::endl;
*/


	// run thread
    zthread_new (client_task, NULL);
    //int secondsToRun = 50;
    int secondsToRun = 5000;
    zclock_sleep( secondsToRun * 1000);    //  Run for 50 seconds then quit

	delete g_osprRenderer;

    return 0;
}
