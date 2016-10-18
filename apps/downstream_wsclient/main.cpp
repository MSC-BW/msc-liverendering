
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STRICT_ // tells websocketpp that we have a full-featured c++11 compiler...

//windows 2013 has not noexcept and constexpr
#define _WEBSOCKETPP_NOEXCEPT_TOKEN_
#define _WEBSOCKETPP_CONSTEXPR_TOKEN_

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

#include <rsi/rsi.h>





struct WebsocketClient
{
    using client = websocketpp::client<websocketpp::config::asio>;

    WebsocketClient()
    {
        std::string const url = "ws://localhost:9002/";
     
        // disable log output
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        //c.clear_access_channels(websocketpp::log::alevel::all);
     
        c.init_asio(&ios);
     
        std::size_t counter = 0;

        c.set_open_handler(
            [&](websocketpp::connection_hdl) {
                std::cout << "connected" << std::endl;
                con->send(std::string("hello"), websocketpp::frame::opcode::text);
            });

        c.set_message_handler(
            [&](websocketpp::connection_hdl, client::message_ptr const& msg)
            {
                std::cout << "message from upstream: msg=" << msg->get_payload() <<std::endl;

                if(msg->get_payload() == "stop listening")
                    con->close(websocketpp::close::status::normal, "");
                /*
                switch (counter++) {
                case 0:
                    con->send(std::string("stop listening"), websocketpp::frame::opcode::text);
                    return;
                case 1:
                    con->close(websocketpp::close::status::normal, "");
                }
                */
            });

        c.set_close_handler(
            [](websocketpp::connection_hdl) {
                std::cout << "closed" << std::endl;
            });

        websocketpp::lib::error_code ec;
        con = c.get_connection(url, ec);
        std::cout << "get_connection result: " << ec.message() << std::endl;
     
        c.connect(con);
     
    }

    void run()
    {
        ios.run();          
    }

    client c;
    client::connection_ptr con;
    asio::io_service ios;

};

WebsocketClient* g_wsclient;
void client_loop()
{
     g_wsclient->run();
}


// this is a websocket client interface to our renderer =================================


int main()
{
    g_wsclient = new WebsocketClient();

    std::thread* client_thread = new std::thread(client_loop);
    
    char msg[20];

    while(1)
    {
        // send text messages to server ---
        printf("Enter message:");
        gets(msg);

        // send message test
        /*
        {
            Command cmd = message( std::string(msg) );
            // send to wsserver ---
            g_wsclient->con->send(cmd.data_alt, websocketpp::frame::opcode::binary);
        }
        */

        ///*
        // setAttr test
        {
            Attribute attr("test_attribute", Attribute::EType::EP3f, 1);

            // set attribute value
            attr.ptr<float>()[0] = 1.0f;
            attr.ptr<float>()[1] = 2.0f;
            attr.ptr<float>()[2] = 3.0f;

            Command cmd = setAttr( std::string(msg), &attr, 1 );

            // send to wsserver ---
            g_wsclient->con->send(cmd.data_alt, websocketpp::frame::opcode::binary);
        }
        //*/
    }
}






