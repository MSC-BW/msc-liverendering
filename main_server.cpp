//  Hello World server
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <mutex>


// required for websocket server test
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STRICT_ // tells websocketpp that we have a full-featured c++11 compiler...
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>


#include <czmq.h>
#include "si.h"


struct Message
{
	typedef std::shared_ptr<Message> Ptr;

	Message():m_data(0), m_size(0)
	{
	}

	Message(const std::string& data)
	{
		m_size = data.size();
		m_data = malloc(m_size);
		memcpy( m_data, &data[0], m_size );
	}

	~Message()
	{
		if(m_data)
			free(m_data);
	}

	zmsg_t* make_zmsg()
	{
		zmsg_t *m = zmsg_new ();

		zframe_t* data = zframe_new (m_data, m_size);
		std::cout << "make_zmsg:test=" << std::string((const char*)zframe_data(data), zframe_size(data)) << std::endl;
		zmsg_append( m, &data );

		return m;
	}

	static Ptr from_string( const std::string& data )
	{
		return std::make_shared<Message>(data);
	}


	void *m_data;
	int m_size;
};


// https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
// this only works for single consumer, single producer scenarios
template<typename Data>
class concurrent_queue
{
private:
    std::queue<Data> the_queue;
    mutable std::mutex the_mutex;
public:
    void push(const Data& data)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.push(data);
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }

    Data& front()
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.front();
    }
    
    Data const& front() const
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.front();
    }

    void pop()
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.pop();
    }
};
concurrent_queue<Message::Ptr> g_msg; // messages from downstream client









struct WebsocketServer
{
	typedef websocketpp::server<websocketpp::config::asio> server;
	//using websocketpp::lib::placeholders::_1;
	//using websocketpp::lib::placeholders::_2;
	//using websocketpp::lib::bind;

	// pull out the type of messages sent by our config
	typedef server::message_ptr message_ptr;

	WebsocketServer()
	{
	    try {
	        // Set logging settings
	        //echo_server.set_access_channels(websocketpp::log::alevel::all);
	        //echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
	        echo_server.clear_access_channels(websocketpp::log::alevel::all);

	        // Initialize Asio
	        echo_server.init_asio();

	        // Register our message handler
	        echo_server.set_message_handler(std::bind(&WebsocketServer::on_message, this, std::placeholders::_1,std::placeholders::_2));

	        // Listen on port 9002
	        echo_server.listen(9002);

	        // Start the server accept loop
	        echo_server.start_accept();

	        //// Start the ASIO io_service run loop
	        //server.echo_server.run();
	    } catch (websocketpp::exception const & e) {
	        std::cout << e.what() << std::endl;
	    } catch (...) {
	        std::cout << "other exception" << std::endl;
	    }

	}

	void run()
	{
		echo_server.run();
	}


	// Define a callback to handle incoming messages
	void on_message(websocketpp::connection_hdl hdl, WebsocketServer::message_ptr msg)
	{
		temp_hdl = hdl;

	    std::cout << "on_message called with hdl: " << hdl.lock().get()
	              << " and message: " << msg->get_payload()
	              << std::endl;

	    // check for a special command to instruct the server to stop listening so
	    // it can be cleanly exited.
	    if (msg->get_payload() == "stop-listening") {
	    	std::cout << "WebsocketServer::stop_listening";
	        echo_server.stop_listening();
	        return;
	    }

	    // put message onto the queue
	    g_msg.push(Message::from_string(msg->get_payload()));

	    try {
	    	// send echo
	        echo_server.send(hdl, msg->get_payload(), msg->get_opcode());
	    } catch (const websocketpp::lib::error_code& e) {
	        std::cout << "Echo failed because: " << e
	                  << "(" << e.message() << ")" << std::endl;
	    }
	}


	// Create a server endpoint
    server echo_server;
    websocketpp::connection_hdl temp_hdl;
};
WebsocketServer* g_wsserver;




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

    /*
    //  Launch pool of worker threads, precise number is not critical
    int thread_nbr;
    for (thread_nbr = 0; thread_nbr < 1; thread_nbr++)
        zthread_fork (ctx, server_worker, NULL);

    //  Connect backend to frontend via a proxy
    zmq_proxy (frontend, backend, NULL);
    */





    ///*
    // test
    {
    	bool is_connected_to_head = false;
    	std::string head_id;


    	int rc = 0;

	    int more;
	    size_t moresz;
	    zmq_pollitem_t itemsin [] =
	    {
	        { frontend, 0, ZMQ_POLLIN|ZMQ_POLLOUT, 0 },
	    };
	    int num_poll_items = 1;
	    zmq_pollitem_t itemsout [] =
	    {
	        { frontend, 0, ZMQ_POLLOUT, 0 },
	    };

	    //  Proxy can be in these three states
	    enum {
	        active,
	        paused,
	        terminated
	    } state = active;

	    while(1)
	    {
	    	// wait for messages from frontend or backend
	    	//TODO: make this non-blocking
		    int rc = zmq_poll (&itemsin[0], num_poll_items, -1);
		    if (rc < 0)
		        return NULL;

	        //  Get the pollout separately because when combining this with pollin it maxes the CPU
	        //  because pollout shall most of the time return directly.
	        //  POLLOUT is only checked when frontend and backend sockets are not the same.
            //rc = zmq_poll (&itemsout [0], num_poll_items, 0);
            //if (rc < 0)
            //    return NULL;

	        //  process messages from frontend client (compute node) ---
	        if (state == active &&  itemsin[0].revents & ZMQ_POLLIN)
	        {
	        	// now consume message (and forward it to websocket server)
				zmsg_t* msg = zmsg_recv (frontend);

				if(msg != NULL)
				{
					std::cout << "received message from upstream client\n";

					//  The DEALER socket gives us the reply envelope and message
					zframe_t *identity = zmsg_pop (msg);

					// if this is the first time we see the upstream client...
					if(!is_connected_to_head)
					{
						// ...register upstream client
						is_connected_to_head = true;
						head_id = string_from_zframe(identity);
					}

					zframe_t *content = zmsg_pop (msg);
					assert (content);
					zmsg_destroy (&msg);

					// TODO:deal with message and forward it to websocket server
					//std::string text = "image!!!!";
					//g_wsserver->echo_server.send(g_wsserver->temp_hdl, text, websocketpp::frame::opcode::text);
					std::string data( (const char*)zframe_data(content), zframe_size(content) );
					//g_wsserver->echo_server.send(g_wsserver->temp_hdl, data, websocketpp::frame::opcode::binary);
					

					zframe_destroy(&identity);
					zframe_destroy(&content);
				}
	        }

	        // process messages from websocket server (user client) ---
	        //if there is something in the websocket incomming queue
	        //if(!(itemsin[0].revents & ZMQ_POLLOUT))
	        //	std::cout << "NO pollout!\n";
	        //else
	        //	std::cout << "GOT pollout!\n";
	        //if(itemsin[0].revents & ZMQ_POLLOUT)
	        //	std::cout << "GOT pollout!\n";


	        //if(!g_msg.empty() && itemsin[0].revents & ZMQ_POLLOUT)
	        if((itemsin[0].revents & ZMQ_POLLOUT) &&
	           is_connected_to_head &&
	           !g_msg.empty())
	        {
	        	Message::Ptr msg = g_msg.front();
	        	g_msg.pop();

	        	// send message to upstream client via zmq
	        	std::cout << "sending message to upstream client\n";

	        	/*
	        	int value = 123456;

	        	Parameter param;
	        	param.m_name = "pos";
	        	param.m_type = Parameter::EInteger;
	        	param.m_size = 1;
	        	param.m_data = &value;
	        	*/

	        	zframe_t* identity = zframe_from_string(head_id);
	            //zmsg_t *m = zmsg_from_param(&param);
	            zmsg_t* m = msg->make_zmsg();
	            zmsg_prepend(m, &identity);
	            zmsg_send(&m, frontend);
	        }
	    };

    }
    //*/





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

    while (true)
    {
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



        // receive a message from the user client ---



        // alternatively produce message from user client
        {
        	int value = 123456;

        	Parameter param;
        	param.m_name = "pos";
        	param.m_type = Parameter::EInteger;
        	param.m_size = 1;
        	param.m_data = &value;


            //  Sleep for some fraction of a second
            zclock_sleep (randof (1000) + 1);

        	zframe_send (&identity, worker, ZFRAME_REUSE + ZFRAME_MORE);
            zframe_send (&content, worker, ZFRAME_REUSE);

            //zmsg_t *m = zmsg_new ();
            //zmsg_append( m, &identity);
            //zmsg_append( m, &content);
            //zmsg_send(&m, worker);

            ///*
            //zmsg_t *m = zmsg_from_param(&param);
            //zmsg_prepend(m, &identity);
            //zmsg_send(&m, worker);
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

/*
int main (void)
{
    zthread_new (server_task, NULL);
    zclock_sleep (50 * 1000);    //  Run for 50 seconds then quit
    return 0;
}
*/



///*






int main()
{
	g_wsserver = new WebsocketServer();
	std::thread* wsserver_thread = new std::thread(&WebsocketServer::run, g_wsserver);

	zthread_new (server_task, NULL);
    zclock_sleep (50 * 1000);    //  Run for 50 seconds then quit
    return 0;

/*
	wsserver_thread->join();
*/	
	//while(1)
	//{
	//}

}
//*/