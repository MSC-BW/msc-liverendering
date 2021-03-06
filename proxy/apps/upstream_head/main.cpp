
#include <zmq.h>
#include <czmq.h>
#include <iostream>

#include <rsi/rsi.h>

#include "empty_delete.h"
#include "jpeg.h"

// required by renderer
#include <vector>
#include <cmath>
#include <algorithm>


struct SimpleRenderer : public IScene
{
    SimpleRenderer()
    {
        m_width = 512;
        m_height = 512;
        m_rgb_data = std::vector<float>( m_width*m_height*3, 0 );

        m_t = 0.0;
        //m_cx = 0.5 + 0.3*std::cos(m_t);
        //m_cy = 0.5 - 0.3*std::sin(m_t);
        m_cx = 256;
        m_cy = 256;
        m_radius = 20.0;

    }

    // IScene implementation ------------------------
    virtual void message( const std::string& msg )override
    {
        std::cout << "SimpleScene::message: msg="  << msg << std::endl;
    }
    virtual void setAttr( const std::string& object_handle, const Attribute* attr_list, int nattrs )override
    {
        for( int i=0;i<nattrs;++i )
        {
            const Attribute& attr = attr_list[i];
            std::cout << "SimpleScene::setAttr: object=" << object_handle << " attr=" << attr.m_name;

            switch(attr.m_type)
            {
                case Attribute::EType::EFloat:
                    std::cout << " value=" << attr.ptr<float>()[0] << std::endl;
                    break;
                case Attribute::EType::EP3f:
                    std::cout << " value=" << attr.ptr<float>()[0] << " " << attr.ptr<float>()[1] << " " << attr.ptr<float>()[2] << std::endl;
                    break;
            }


            if( attr.m_name == "position" )
            {
                m_cx = attr.ptr<float>()[0];
                m_cy = attr.ptr<float>()[1];            
            }else
            if( attr.m_name == "radius" )
            {
                m_radius = attr.ptr<float>()[0];            
            }

        }
    }




    int m_width;
    int m_height;
    std::vector<float> m_rgb_data;


    double m_t;
    double m_cx, m_cy;
    double m_radius;


    void advance()
    {
        std::fill(m_rgb_data.begin(), m_rgb_data.end(), 0);

        m_t += 0.1;

        // do some shit
        for( int y=0;y<m_height;++y )
            for( int x=0;x<m_width;++x )
            {
                double x2 = x-m_cx;
                double y2 = y-m_cy;
                double d = std::sqrt( x2*x2+y2*y2 );
                if( d < m_radius )
                {
                    m_rgb_data[ y*m_width*3+x*3 + 0 ] = 1.0;
                    m_rgb_data[ y*m_width*3+x*3 + 1 ] = 1.0;
                    m_rgb_data[ y*m_width*3+x*3 + 2 ] = 1.0;
                }
            }
    }

    float *getRGBData( int& width, int& height )
    {
        width = this->m_width;
        height = this->m_height;
        return &m_rgb_data[0];
    }

private:

};


SimpleRenderer g_simpleRenderer;

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
    int result = zsocket_connect (client, "tcp://193.196.155.54:5570");
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
                execute(&g_simpleRenderer, command);

                zmsg_destroy (&msg);
            }
        }

        // send a request to the server (this would be the rendered image)
        //std::cout << "client:sending request\n";
        std::string image = ">                    <";
        image[counter++%(image.size()-2) + 1] = 'o';
        //zstr_sendf (client, image.c_str());


        // render image 
        g_simpleRenderer.advance();

        // access rendered image, convert to ldr and compress to jpeg
        {
            int xres, yres;
            float* data_hdr = g_simpleRenderer.getRGBData(xres, yres);
            int numPixels = xres*yres;
            std::vector<unsigned char> data_ldr( numPixels*3 );

            std::transform( data_hdr, data_hdr + numPixels*3,
                            data_ldr.begin(),
                [](float value_linear)
                {
                    float value_srgb = 0;
                    if (value_linear <= 0.0031308f)
                        value_srgb = 12.92f * value_linear;
                    else
                        value_srgb = (1.0f + 0.055f)*std::pow(value_linear, 1.0f/2.4f) -  0.055f;

                    return (unsigned char)(std::max( 0.0, std::min(255.0, value_srgb*255.0) ));
                });

            //write_jpeg_to_file( "test.jpg", xres, yres, &data_ldr[0] );
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



int main (void)
{
    // render image
    g_simpleRenderer.advance();

    zthread_new (client_task, NULL);
    zclock_sleep (50 * 1000);    //  Run for 50 seconds then quit
    return 0;
}