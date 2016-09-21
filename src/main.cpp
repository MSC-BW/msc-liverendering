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

#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#  include <malloc.h>
#else
#  include <alloca.h>
#endif


#include "miniSG/miniSG.h"
#include <ospray/ospray.h>
#include <common/commandline/Utility.h>
#undef PING

#include <util/timer.h>





#include <rserver/RenderServer.h>
#include <jsmn/jsmn_util.h>



struct OSPRayRenderer : public ProgressiveRendererInterface
{
	int m_resx, m_resy;


	OSPRayRenderer() : ProgressiveRendererInterface(),
						m_doReset(false)
	{

	}

	//Scene::Ptr m_scene;
	ProgressCallback m_progress_cb;


	void start()
	{

	}


	virtual void setProgressCallback(ProgressCallback callback)override
	{
		m_progress_cb = callback;
	}


	virtual void getImageResolution(int& width, int& height)override
	{
		width = m_resx;
		height = m_resy;
	}

	/*
	virtual void copyRGBData( float *rgb_data )override
	{
	std::lock_guard<std::mutex> lock(m_renderer_access_lock);

	// convert gray channel to rgb color
	// TODO: render rgb
	int width = RenderThreadInfo::g.xres;
	int height = RenderThreadInfo::g.yres;
	float* ptr = rgb_data;
	for( int i=0;i<width*height;++i )
	{
	  *ptr++ = RenderThreadInfo::g.result_f[i];
	  *ptr++ = RenderThreadInfo::g.result_f[i];
	  *ptr++ = RenderThreadInfo::g.result_f[i];
	}
	}
	*/

	//virtual void copyRGBData( unsigned char* rgb_data )
	virtual void copyRGBData( unsigned char* rgb_data, int& resx, int& resy )
	{
		std::lock_guard<std::mutex> lock(m_renderer_access_lock);

		/*
		// convert gray channel to rgb color and uchar
		// TODO: render rgb
		int width = m_xres;
		int height = m_yres;
		unsigned char* ptr = rgb_data;
		for( int i=0;i<width*height;++i )
		{
		  Color3f val_rgb = Color3f(RenderThreadInfo::g.result_f[i]).toSRGB();
		  *ptr++ = std::max(0, std::min( 255, int(val_rgb.r() * 255.0)));
		  *ptr++ = std::max(0, std::min( 255, int(val_rgb.g() * 255.0)));
		  *ptr++ = std::max(0, std::min( 255, int(val_rgb.b() * 255.0)));
		}
		*/
	}


	virtual void receiveMessage( const std::string& id, const std::string& message )override
	{
		std::lock_guard<std::mutex> lock(m_renderer_access_lock);
		//std::cout << "DummyRenderer::receiveMessage  " << message << std::endl;std::flush(std::cout);

		// parse json ---
		jsmn_parser parser;
		jsmn_init(&parser);
		jsmntok_t tokens[256];
		int r = jsmn_parse(&parser, &message[0], message.size(), tokens, 256);


		std::map<std::string, jsmntok_t*> map;
		jsmn_to_map( message, &tokens[0], map );

		std::string command = jsmn_to_string(message, map["command"]);
		//std::cout << "command=" << command << std::endl;


		if(command == "set")
		{
		  std::map<std::string, jsmntok_t*> props;
		  jsmn_to_map( message, map["properties"], props );

		  //for( auto& it:props )
		  //{
			//	std::string prop = it.first;
			//if( it.second->type == JSMN_PRIMITIVE )
			//  std::cout << "setting property:" << prop << "=" << jsmn_to_number<double>(message, it.second) << std::endl;
			//else
			//if( it.second->type == JSMN_STRING )
			//  std::cout << "setting property:" << prop << "=" << jsmn_to_string(message, it.second) << std::endl;
		  //}


		  /*
		  {
			// parse gl-matrix.js matrix string
			std::string camToWorldStr = jsmn_to_string(message, props["camToWorld"]);
			camToWorldStr = replace( camToWorldStr, "mat4(", "" );
			camToWorldStr = replace( camToWorldStr, ")", "" );
			std::vector<std::string> components;
			splitString( camToWorldStr, components, "," );

			M44d m;
			m << fromString<double>(components[0]), fromString<double>(components[1]), fromString<double>(components[2]), fromString<double>(components[3]),
			   fromString<double>(components[4]), fromString<double>(components[5]), fromString<double>(components[6]), fromString<double>(components[7]),
			   fromString<double>(components[8]), fromString<double>(components[9]), fromString<double>(components[10]), fromString<double>(components[11]),
			   fromString<double>(components[12]), fromString<double>(components[13]), fromString<double>(components[14]), fromString<double>(components[15]);

			M44d m2 = m.transpose();
			for( int i=0;i<4;++i )
			  for( int j=0;j<4;++j )
				std::cout << m2(i, j) << " ";
				//std::cout << m_scene->m_camera->m_cameraToWorld.getMatrix()(i, j) << " ";
			std::cout << std::endl;


			//m_scene->m_camera->setCameraToWorld( Transformd(m2) );

			//math::M44f m;
			//if( components.size() == 16 )
			//{
			//  for( int i=0;i<16;++i )
			//	m.ma[i] = core::fromString<float>(components[i]);
			//}
			//camera->setViewToWorld(m);


			//m_glViewer->update();
			m_doReset = true;
		  }
		  */

		  //cx = jsmn_to_number<double>(message, props["cx"]);
		  //cy = jsmn_to_number<double>(message, props["cy"]);
		}
	}

	//GLWidget* m_glViewer;
	private:
	bool m_doReset;

	std::mutex m_renderer_access_lock;

};




using namespace ospray;

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
	for (int x = 0; x < size.x; x++) {
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


int main(int ac, const char **av)
{
  Timer timer;

  // initialize OSPRay; OSPRay parses (and removes) its commandline parameters, e.g. "--osp:debug"
  ospInit(&ac, av);
  std::cout << "ospInit done\n";std::flush(std::cout);

  timer.reset();
  timer.start();
  auto ospObjs = parseWithDefaultParsers(ac, av);
  timer.stop();
  std::cout << "parsing done. took " << timer.elapsedSeconds() << "s" << std::endl;std::flush(std::cout);

  ospcommon::box3f      bbox;
  ospray::cpp::Model    model;
  ospray::cpp::Renderer renderer;
  ospray::cpp::Camera   camera;

  std::tie(bbox, model, renderer, camera) = ospObjs;


  /*
  float pos_x = 252.85;
  float pos_y = 72.075;
  float pos_z = 32.1726;

  float lookat_x = 153.497;
  float lookat_y = 15.94;
  float lookat_z = 146.397;
  */
  float pos_x = 292.579;
  float pos_y = 127.829;
  float pos_z = 346.911;

  float lookat_x = 162.636;
  float lookat_y = 0;
  float lookat_z = 152.048;

  camera.set("pos", ospcommon::vec3f(pos_x, pos_y, pos_z));
  camera.set("dir", ospcommon::vec3f(lookat_x-pos_x,lookat_y-pos_y,lookat_z-pos_z) );
  camera.set("up", ospcommon::vec3f(0.0f, 1.0f, 0.0f) );
  camera.commit();


  renderer.set("world",  model);
  renderer.set("model",  model);
  renderer.set("camera", camera);

  int spp = 1;
  renderer.set("spp", spp);

  renderer.commit();

  // image size
  osp::vec2i imgSize;
  imgSize.x = 4096; // width
  imgSize.y = 4096; // height

  //imgSize.x = 512; // width
  //imgSize.y = 512; // height


  ospray::cpp::FrameBuffer fb = ospray::cpp::FrameBuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM);
  fb.clear(OSP_FB_ACCUM);


  std::cout << "renderFrame about to start\n";std::flush(std::cout);
  std::cout << "resolution=" << imgSize.x << " " << imgSize.y << std::endl;;std::flush(std::cout);
  std::cout << "spp=" << spp << std::endl;;std::flush(std::cout);


  timer.reset();
  timer.start();
  renderer.renderFrame(fb, OSP_FB_COLOR | OSP_FB_ACCUM);
  timer.stop();
  std::cout << "render frame took " << timer.elapsedSeconds() << "s\n";

  // save screenshot
  std::string basename = "firstFrame";
  const uint32_t *p = (uint32_t*)fb.map(OSP_FB_COLOR);
  writePPM(std::string(basename+".ppm").c_str(), imgSize, p);
  std::cout << "#ospGlutViewer: saved current frame to '" << basename << ".ppm" << std::endl;

  // websocket server test -----------
  {
 	OSPRayRenderer renderer;
 	RenderServer rserver(&renderer);

 	// this will start the render server which listens to messages from clients and progress updates from the renderer
	rserver.start();
	//// this will start the actual rendering process
	//renderer.start();
}


/*
  // ospTutorial -----------------------------------
  // image size
  osp::vec2i imgSize;
  imgSize.x = 1024; // width
  imgSize.y = 768; // height

  // camera
  float cam_pos[] = {0.f, 0.f, 0.f};
  float cam_up [] = {0.f, 1.f, 0.f};
  float cam_view [] = {0.1f, 0.f, 1.f};

  // triangle mesh data
  float vertex[] = { -1.0f, -1.0f, 3.0f, 0.f,
					 -1.0f,  1.0f, 3.0f, 0.f,
					  1.0f, -1.0f, 3.0f, 0.f,
					  0.1f,  0.1f, 0.3f, 0.f };
  float color[] =  { 0.9f, 0.5f, 0.5f, 1.0f,
					 0.8f, 0.8f, 0.8f, 1.0f,
					 0.8f, 0.8f, 0.8f, 1.0f,
					 0.5f, 0.9f, 0.5f, 1.0f };
  int32_t index[] = { 0, 1, 2,
					  1, 2, 3 };


  // create and setup camera
  OSPCamera camera = ospNewCamera("perspective");
  ospSetf(camera, "aspect", imgSize.x/(float)imgSize.y);
  ospSet3fv(camera, "pos", cam_pos);
  ospSet3fv(camera, "dir", cam_view);
  ospSet3fv(camera, "up",  cam_up);
  ospCommit(camera); // commit each object to indicate modifications are done


  // create and setup model and mesh
  OSPGeometry mesh = ospNewGeometry("triangles");
  OSPData data = ospNewData(4, OSP_FLOAT3A, vertex); // OSP_FLOAT3 format is also supported for vertex positions (currently not on MIC)
  ospCommit(data);
  ospSetData(mesh, "vertex", data);

  data = ospNewData(4, OSP_FLOAT4, color);
  ospCommit(data);
  ospSetData(mesh, "vertex.color", data);

  data = ospNewData(2, OSP_INT3, index); // OSP_INT4 format is also supported for triangle indices
  ospCommit(data);
  ospSetData(mesh, "index", data);

  ospCommit(mesh);


  OSPModel world = ospNewModel();
  ospAddGeometry(world, mesh);
  ospCommit(world);


  // create and setup renderer
  OSPRenderer renderer = ospNewRenderer("scivis"); // choose Scientific Visualization renderer
  ospSet1f(renderer, "aoWeight", 1.0f);            // with full Ambient Occlusion
  ospSet1i(renderer, "aoSamples", 1);
  ospSetObject(renderer, "model",  world);
  ospSetObject(renderer, "camera", camera);
  ospCommit(renderer);


  // create and setup framebuffer
  OSPFrameBuffer framebuffer = ospNewFrameBuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  ospFrameBufferClear(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

  // render one frame
  ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

  // access framebuffer and write its content as PPM file
  const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
  writePPM("firstFrame.ppm", imgSize, fb);
  ospUnmapFrameBuffer(fb, framebuffer);


  // render 10 more frames, which are accumulated to result in a better converged image
  for (int frames = 0; frames < 10; frames++)
	ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

  fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
  writePPM("accumulatedFrame.ppm", imgSize, fb);
  ospUnmapFrameBuffer(fb, framebuffer);
  */

  return 0;
}
