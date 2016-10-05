//  Hello World server
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <iostream>


/*
int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    std::cout << "running server...\n";
    while (1) {
        char buffer [10];
        zmq_recv (responder, buffer, 10, 0);
        printf ("Received Hello\n");
        sleep (1);          //  Do some 'work'
        zmq_send (responder, "World", 5, 0);
    }
    return 0;
}
*/





#include <czmq.h>
#include <iostream>

#include "si.h"

//  This is our server task.
//  It uses the multithreaded server model to deal requests out to a pool
//  of workers and route replies back to clients. One worker can handle
//  one request at a time but one client can talk to multiple workers at
//  once.

static void server_worker (void *args, zctx_t *ctx, void *pipe);

void *server_task (void *args)
{
    //  Frontend socket talks to clients over TCP
    zctx_t *ctx = zctx_new ();
    void *frontend = zsocket_new (ctx, ZMQ_ROUTER);
    zsocket_bind (frontend, "tcp://*:5570");

    //  Backend socket talks to workers over inproc
    void *backend = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_bind (backend, "inproc://backend");

    //  Launch pool of worker threads, precise number is not critical
    int thread_nbr;
    for (thread_nbr = 0; thread_nbr < 5; thread_nbr++)
        zthread_fork (ctx, server_worker, NULL);

    //  Connect backend to frontend via a proxy
    zmq_proxy (frontend, backend, NULL);

    zctx_destroy (&ctx);
    return NULL;
}

//  Each worker task works on one request at a time and sends a random number
//  of replies back, with random delays between replies:

static void
server_worker (void *args, zctx_t *ctx, void *pipe)
{
    void *worker = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_connect (worker, "inproc://backend");

    while (true) {
        // receive a message from the client ---
        zmsg_t *msg = zmsg_recv (worker);
        //  The DEALER socket gives us the reply envelope and message
        zframe_t *identity = zmsg_pop (msg);
        zframe_t *content = zmsg_pop (msg);
        assert (content);
        zmsg_destroy (&msg);

        // handle message from render client (backend) ---
        {
        	int size = zframe_size(content);

        	std::string image((char*)zframe_data(content), size);
        	std::cout << "image:" << image << std::endl;
        }

        // produce message from user client (frontend)
        {
        	int value = 123456;

        	Parameter param;
        	param.m_name = "pos";
        	param.m_type = Parameter::EInteger;
        	param.m_size = 1;
        	param.m_data = &value;


            //  Sleep for some fraction of a second
            zclock_sleep (randof (1000) + 1);

        	//zframe_send (&identity, worker, ZFRAME_REUSE + ZFRAME_MORE);
            //zframe_send (&content, worker, ZFRAME_REUSE);

            //zmsg_t *m = zmsg_new ();
            //zmsg_append( m, &identity);
            //zmsg_append( m, &content);
            //zmsg_send(&m, worker);

            ///*
            zmsg_t *m = zmsg_from_param(&param, &content);
            zmsg_prepend(m, &identity);
            zmsg_send(&m, worker);
            //*/

        }
/*
        //  Send 0..4 replies back (this would be the scene updates)
        int reply, replies = randof (5);
        for (reply = 0; reply < replies; reply++)
        {
            //  Sleep for some fraction of a second
            zclock_sleep (randof (1000) + 1);
            //std::cout << "worker sending reply\n";
            zframe_send (&identity, worker, ZFRAME_REUSE + ZFRAME_MORE);
            zframe_send (&content, worker, ZFRAME_REUSE);
        }
*/
        //zframe_destroy (&identity);
        //zframe_destroy (&content);
    }
}

//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.

int main (void)
{
    zthread_new (server_task, NULL);
    zclock_sleep (50 * 1000);    //  Run for 50 seconds then quit
    return 0;
}