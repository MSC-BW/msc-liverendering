/*
This file implements a simple proxy to run within the HPC network.
The purpose of this proxy is to allow cray compute nodes to talk bidirectional with the outside world.
Special about this proxy is that it exposes a websocket server to the downstream client side
and an ordinary tcp/ip connection (using zeromq) to the upstream client.
*/
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <mutex>
#include <vector>


// required for running the websocket server which talks to downstream clients
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STRICT_ // tells websocketpp that we have a full-featured c++11 compiler...
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>


// required for talking to upstream client from compute node
#include <czmq.h>



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

concurrent_queue<zmsg_t*> g_msg; // messages from downstream client






// required to test connection handles for validity
// see http://stackoverflow.com/questions/26913743/can-an-expired-weak-ptr-be-distinguished-from-an-uninitialized-one
template <typename T>
bool PointsToValidOrExpiredObject(const std::weak_ptr<T>& w) {
    return w.owner_before(std::weak_ptr<T>{}) || std::weak_ptr<T>{}.owner_before(w);
}


struct WebsocketServer
{
	typedef websocketpp::server<websocketpp::config::asio> server;

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
	        echo_server.set_validate_handler(std::bind(&WebsocketServer::on_validate, this, std::placeholders::_1));
	        echo_server.set_open_handler(bind(&WebsocketServer::on_open,this,std::placeholders::_1));

	        // Listen on port 9002
	        echo_server.set_reuse_addr(true);
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
		std::cout << "running websocket server...\n";
		echo_server.run();
	}

    void on_open(websocketpp::connection_hdl hdl)
    {
		std::cout << "downstream client connected..." << std::endl;std::flush(std::cout);
		connected_client = hdl;
    }
 	

	bool on_validate(websocketpp::connection_hdl hdl)
	{
    	std::cout << "downstream client connection attempt..." << std::endl;std::flush(std::cout);

    	// we currently only allow one single client to be connected
    	if(PointsToValidOrExpiredObject(connected_client))
    		return false;
    	
		/*
		//std::cout << "on_validate\n";std::flush(std::cout);
		websocketpp::server<websocketpp::config::asio>::connection_ptr con = server.get_con_from_hdl(hdl);
		websocketpp::uri_ptr uri = con->get_uri();
		std::string query = uri->get_query(); // returns empty string if no query string set.
		//std::cout << "WebsocketServer::on_validate querystring=" << query << std::endl;
		std::string id = "";
		if (!query.empty())
		{
			// Split the query parameter string here, if desired.
			// We assume we extracted a string called 'id' here.
			id = "test";
			std::cout << "warning: id is hardcoded:" << id << std::endl;
		}
		else
		{
			// Reject if no query parameter provided, for example.
			return false;
		}

		websocketsLock.lock();
		g_id_to_socket.insert(std::pair<std::string, websocketpp::connection_hdl>(id, hdl));
		g_socket_to_id.insert(std::pair<websocketpp::connection_hdl, std::string>(hdl, id));
		websocketsLock.unlock();
		*/

		return true;
	}

	// Define a callback to handle incoming messages
	void on_message(websocketpp::connection_hdl hdl, WebsocketServer::message_ptr msg)
	{
	    //std::cout << "on_message called with hdl: " << hdl.lock().get() << " and message: " << msg->get_payload() << std::endl;
	    //std::cout << "on_message called" << std::endl;

	    // check for a special command to instruct the server to stop listening so
	    // it can be cleanly exited.
	    if (msg->get_payload() == "stop-listening") {
	    	std::cout << "WebsocketServer::stop_listening";
	        echo_server.stop_listening();
	        return;
	    }

	    // create zmq message and push onto the queue of messages to be sent to upstream client
	    {
	    	const std::string& data = msg->get_payload();

			zmsg_t *m = zmsg_new ();
			zframe_t* data_frame = zframe_new ((const char*)&data[0], data.size());
			zmsg_append( m, &data_frame );
			g_msg.push(m);
		}



	    try {
	    	// send echo
	        echo_server.send(hdl, msg->get_payload(), msg->get_opcode());
	    } catch (const websocketpp::lib::error_code& e) {
	        std::cout << "Echo failed because: " << e << "(" << e.message() << ")" << std::endl;
	    }
	}

	void broadcast( const std::string& data, websocketpp::frame::opcode::value opcode )
	{
		if(PointsToValidOrExpiredObject(connected_client))
			echo_server.send(connected_client, data, opcode);
	}

	// Create a server endpoint
    server echo_server;
    websocketpp::connection_hdl connected_client; // will be extended as soon as we allow more than one client
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

    // uncomment the following codesnipped if the downstream client talks to the proxy
    // using zeromq (instead of websockets)
    /*
    //  Launch pool of worker threads, precise number is not critical
    int thread_nbr;
    for (thread_nbr = 0; thread_nbr < 1; thread_nbr++)
        zthread_fork (ctx, server_worker, NULL);

    //  Connect backend to frontend via a proxy
    zmq_proxy (frontend, backend, NULL);
    */


    // the following code part runs an infinite while loop which waits for messages from downstream
    // or upstream and redirects them to the other side. 
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

		std::cout << "listening on frontend socket...\n";
	    while(1)
	    {
	    	// wait for messages from frontend or backend
	    	//TODO: make this non-blocking
		    int rc = zmq_poll (&itemsin[0], num_poll_items, -1);
		    if (rc < 0)
		        return NULL;

	        //  process messages from frontend client (compute node) ---
	        if (state == active &&  itemsin[0].revents & ZMQ_POLLIN)
	        {
	        	// now consume message (and forward it to websocket server)
				zmsg_t* msg = zmsg_recv (frontend);

				if(msg != NULL)
				{
					//std::cout << "received message from upstream client\n";

					//  The DEALER socket gives us the reply envelope and message
					zframe_t *identity = zmsg_pop (msg);

					// if this is the first time we see the upstream client...
					if(!is_connected_to_head)
					{
						// ...register upstream client
						is_connected_to_head = true;
						// string_From_zframe
						head_id = std::string( (char*)zframe_data(identity),  zframe_size (identity));
						// log
						std::cout << "upstream client connected (id=" << head_id << ")..." << std::endl;
					}

					zframe_t *content = zmsg_pop (msg);
					assert (content);
					zmsg_destroy (&msg);

					// TODO:deal with message and forward it to websocket server
					//std::string text = "image!!!!";
					//g_wsserver->echo_server.send(g_wsserver->temp_hdl, text, websocketpp::frame::opcode::text);
					std::string data( (const char*)zframe_data(content), zframe_size(content) );
					g_wsserver->broadcast(data, websocketpp::frame::opcode::binary);
					
					

					zframe_destroy(&identity);
					zframe_destroy(&content);
				}
	        }

	        // process messages from websocket server (downstream client) ---
	        if((itemsin[0].revents & ZMQ_POLLOUT) &&
	           is_connected_to_head &&
	           !g_msg.empty())
	        {
	        	//Message::Ptr msg = g_msg.front();
	        	//g_msg.pop();
	        	zmsg_t* m = g_msg.front();
	        	g_msg.pop();

	        	// send message to upstream client via zmq
	        	//std::cout << "sending message to upstream client\n";

	        	// zframe_from_string
	        	zframe_t* identity = zframe_new (&head_id[0], head_id.size());
	            zmsg_prepend(m, &identity);
	            zmsg_send(&m, frontend);
	        }
	    };

    }


    zctx_destroy (&ctx);
    return NULL;
}


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
        	//std::cout << "image:" << image << std::endl;
        }
    }
}





int main()
{
	g_wsserver = new WebsocketServer();
	std::thread* wsserver_thread = new std::thread(&WebsocketServer::run, g_wsserver);

	zthread_new (server_task, NULL);
	//int secondsToRun = 50;
	int secondsToRun = 5000;
    zclock_sleep (secondsToRun * 1000);    //  Run for 50 seconds then quit
    return 0;
}
