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


#include "miniSG/miniSG.h"
#include <ospray/ospray.h>
#include <common/commandline/Utility.h>
#undef PING

#include <util/timer.h>
#include <util/string.h>




using namespace ospray;







struct OSPRRenderer : public IScene
{
	ospcommon::box3f      bbox;
	ospray::cpp::Model    model;
	ospray::cpp::Renderer renderer;
	ospray::cpp::Camera   camera;
	ospray::cpp::FrameBuffer fb;
	osp::vec2i imgSize;

	OSPRRenderer(int argc, const char **argv)
	{
		counter = 0;
		Timer timer;


		timer.reset();
		timer.start();
		auto ospObjs = parseWithDefaultParsers(argc, argv);
		timer.stop();
		std::cout << "parsing done. took " << timer.elapsedSeconds() << "s" << std::endl;std::flush(std::cout);


		std::tie(bbox, model, renderer, camera) = ospObjs;


/*
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
*/

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


		timer.reset();
		timer.start();
		renderer.renderFrame(fb, OSP_FB_COLOR | OSP_FB_ACCUM);
		timer.stop();
		std::cout << "render frame took " << timer.elapsedSeconds() << "s\n";

	}

	// IScene implementation ------------------------
	virtual void message( const std::string& msg )override
	{
		std::cout << "OSPRRenderer::message: msg="  << msg << std::endl;
	}
	virtual void setAttr( const std::string& object_handle, const Attribute* attr_list, int nattrs )override
	{
		for( int i=0;i<nattrs;++i )
		{
			const Attribute& attr = attr_list[i];
			std::cout << "OSPRRenderer::setAttr: object=" << object_handle << " attr=" << attr.m_name;

			switch(attr.m_type)
			{
				case Attribute::EType::EFloat:
					std::cout << " value=" << attr.ptr<float>()[0] << std::endl;
					break;
				case Attribute::EType::EP3f:
					std::cout << " value=" << attr.ptr<float>()[0] << " " << attr.ptr<float>()[1] << " " << attr.ptr<float>()[2] << std::endl;
					break;
				case Attribute::EType::EM44f:
					std::cout << " value=" << attr.ptr<float>()[0] << " " << attr.ptr<float>()[1] << " " << attr.ptr<float>()[2] << " " << attr.ptr<float>()[3];
					std::cout << " " << attr.ptr<float>()[4] << " " << attr.ptr<float>()[5] << " " << attr.ptr<float>()[6] << " " << attr.ptr<float>()[7];
					std::cout << " " << attr.ptr<float>()[8] << " " << attr.ptr<float>()[9] << " " << attr.ptr<float>()[10] << " " << attr.ptr<float>()[11];
					std::cout << " " << attr.ptr<float>()[12] << " " << attr.ptr<float>()[13] << " " << attr.ptr<float>()[14] << " " << attr.ptr<float>()[15] << std::endl;
					break;
			}

			if( attr.m_name == "position" )
			{
				//m_cx = attr.ptr<float>()[0];
				//m_cy = attr.ptr<float>()[1];            
			}else
			if( attr.m_name == "radius" )
			{
				//m_radius = attr.ptr<float>()[0];            
			}else
			if( attr.m_name == "xform" )
			{
				std::cout << "setting transform matrix....\n";
				camera.set("pos", ospcommon::vec3f(attr.ptr<float>()[12], attr.ptr<float>()[13], attr.ptr<float>()[14]));
				//camera.set("dir", ospcommon::vec3f(lookat_x-pos_x,lookat_y-pos_y,lookat_z-pos_z) );
				//camera.set("up", ospcommon::vec3f(0.0f, 1.0f, 0.0f) );
				camera.set("dir", ospcommon::vec3f(attr.ptr<float>()[8], attr.ptr<float>()[9], attr.ptr<float>()[10]) );
				camera.set("up", ospcommon::vec3f(attr.ptr<float>()[4], attr.ptr<float>()[5], attr.ptr<float>()[6]) );
				camera.commit();

				// ospray expects position, viewing direction and up vector
				// we extract this information from the matrix

				//m_radius = attr.ptr<float>()[0];            
			}

		}

		fb.clear(OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
	}



	void advance()
	{
		Timer timer;
		timer.reset();
		timer.start();
		renderer.renderFrame(fb, OSP_FB_COLOR | OSP_FB_ACCUM);
		timer.stop();
		std::cout << "render frame took " << timer.elapsedSeconds() << "s\n";
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
    int result = zsocket_connect (client, "tcp://193.196.155.57:5570");
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
	// save screenshot
	std::string basename = "firstFrame";
	const uint32_t *p = (uint32_t*)fb.map(OSP_FB_COLOR);
	writePPM(std::string(basename+".ppm").c_str(), imgSize, p);
	std::cout << "#ospGlutViewer: saved current frame to '" << basename << ".ppm" << std::endl;
*/


	// run thread
    zthread_new (client_task, NULL);
    zclock_sleep (50 * 1000);    //  Run for 50 seconds then quit

	delete g_osprRenderer;

    return 0;
}
