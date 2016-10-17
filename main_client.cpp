
#include <zmq.h>
#include <czmq.h>
#include <iostream>

#include "si.h"
#include <util/empty_delete.h>


struct SimpleScene : public IScene
{
    virtual void message( const std::string& msg )
    {
        std::cout << "SimpleScene::message: msg="  << msg << std::endl;
    }
    virtual void setAttr( const std::string& object_handle, const Attribute* attr_list, int nattrs )
    {
        std::cout << "SimpleScene::setAttr: object=" << object_handle << " #attr=" << nattrs << std::endl;
        // test:
        std::cout << "value=" << attr_list[0].ptr<float>()[0] << " " << attr_list[0].ptr<float>()[1] << " " << attr_list[0].ptr<float>()[2] << std::endl;

    }
};


SimpleScene g_simpleScene;

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
    zsocket_connect (client, "tcp://193.196.155.52:5570");

    zmq_pollitem_t items [] = { { client, 0, ZMQ_POLLIN, 0 } };
    int request_nbr = 0;
    int counter = 0;
    while (true)
    {
        //  Tick once per second, pulling in arriving messages
        int centitick;
        for (centitick = 0; centitick < 100; centitick++)
        {
            // ??
            zmq_poll (items, 1, 10 * ZMQ_POLL_MSEC);
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
                execute(&g_simpleScene, command);

                zmsg_destroy (&msg);
            }
        }

        // send a request to the server (this would be the rendered image)
        //std::cout << "client:sending request\n";
        std::string image = ">                    <";
        image[counter++%(image.size()-2) + 1] = 'o';
        //zstr_sendf (client, "request #%d", ++request_nbr);
        zstr_sendf (client, image.c_str());
    }
    zctx_destroy (&ctx);
    return NULL;
}



int main (void)
{
    zthread_new (client_task, NULL);
    zclock_sleep (50 * 1000);    //  Run for 50 seconds then quit
    return 0;
}