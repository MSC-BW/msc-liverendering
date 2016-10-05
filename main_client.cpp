
//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
int main (void)
{
    printf ("Connecting to hello world server...\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    //zmq_connect (requester, "tcp://localhost:5555");
    zmq_connect (requester, "tcp://193.196.155.56:5555");
    


    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        char buffer [10];
        printf ("Sending Hello %d...\n", request_nbr);
        zmq_send (requester, "Hello", 5, 0);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received World %d\n", request_nbr);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}
*/





#include <czmq.h>
#include <iostream>

//  This is our client task
//  It connects to the server, and then sends a request once per second
//  It collects responses as they arrive, and it prints them out. We will
//  run several client tasks in parallel, each with a different random ID.

static void *
client_task (void *args)
{
    zctx_t *ctx = zctx_new ();
    void *client = zsocket_new (ctx, ZMQ_DEALER);

    //  Set random identity to make tracing easier
    char identity [10];
    sprintf (identity, "%04X-%04X", randof (0x10000), randof (0x10000));
    zsocket_set_identity (client, identity);
    //zsocket_connect (client, "tcp://localhost:5570");
    zsocket_connect (client, "tcp://193.196.155.54:5570");

    zmq_pollitem_t items [] = { { client, 0, ZMQ_POLLIN, 0 } };
    int request_nbr = 0;
    while (true) {
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
                // print message from server
                std::cout << "client:received message from server\n";
                zframe_print (zmsg_last (msg), identity);
                zmsg_destroy (&msg);
            }
        }

        // send a request to the server (this would be the rendered image)
        std::cout << "client:sending request\n";
        zstr_sendf (client, "request #%d", ++request_nbr);
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