 #include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <stdexcept>
#include <signal.h>
#include <algorithm>

#include "Utils.hpp"
#include "server/TcpConnection.hpp"
#include "server/Server.hpp"
#include "server/Request.hpp"
#include "server/Response.hpp"
#include "controller/Controller.hpp"
#include "controller/SendFileController.hpp"
#include "controller/TextController.hpp"
#include "controller/ExecScriptController.hpp"
#include "http/HttpStatus.hpp"
#include "error/RequestError.hpp"
#include "error/ResponseError.hpp"
#include "error/ControllerError.hpp"
#include "error/SocketError.hpp"
#include "error/ConnectionError.hpp"
#include "error/TodoError.hpp"

Server::Server(Config const& config) : m_config(config)
{
	//soscket creation:
	//m_master = socket(AF_INET, SOCK_STREAM, 0);
	//if (m_master < 0) {
	//	exit(1);
	//}
	printf("i created a socket");	
	struct sockaddr_in server_addr;	
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons((u_short)config.port);
	
	int optval = 1;
	setsockopt(m_master, SOL_SOCKET, SO_REUSEADDR,(char *) &optval, sizeof( int ) );

	//socket creation
	m_master = socket(AF_INET, SOCK_STREAM, 0);
	if (m_master < 0) {
		exit(1);
	}

	//bind: 
	if ((bind(m_master, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) {
		exit(1);
	}
	
	//listen:
	listen(m_master, config.queue_length);
	//throw TodoError("2", "Server constructor/connecting to a socket");

}

void Server::run_linear() const
{
    while (true)
    {
        std::string response;
        TcpConnection* conn = new TcpConnection(m_config, m_master);

	handle(conn);

        delete conn;
    }
}

void Server::run_fork() const
{
    while(true) {
	
	std::string response;
	TcpConnection* conn = new TcpConnection(m_config, m_master);

	pid_t pid = fork();
	if (pid == 0) {
	  	handle(conn);
	/*	//deal with zombie
		if (signal(SIGCHLD, SIG_DFL)==SIG_ERR){
			delete conn;
			perror("zombie");
			_exit(1);
		}*/
		delete conn;
		_exit(0);
	}

	if (pid > 0) {
		//deal with zombie
	/*	if (signal(SIGCHLD, SIG_IGN)==SIG_ERR){
			delete conn;
			perror("zombie");
			exit(1);
		}*/
		//delete conn;
	}
	if (conn != NULL) delete conn;
    }
    //exit(0);
    return;
//    throw TodoError("3", "You need to implement thread-per-request mode");
}

void Server::run_thread_request() const
{
    while(true) {
	//std::thread t = std::thread(&Server::run_linear, this);
	
	std::string response;
	TcpConnection* conn = new TcpConnection(m_config, m_master);
	std::thread t = std::thread(&Server::forThread,this,conn);
	
	t.detach();
    }
    //exit(0);
    return;
//    throw TodoError("3", "You need to implement process-per-request mode");
}

void Server::forThread(TcpConnection* conn)const {
	Server::handle(conn);
	delete conn;
}

void Server::run_thread_pool() const
{
    //std::vector<std::thread> t;
/*
    for (int i = 0; i < m_config.threads; i++){
	t.push_back(std::thread([&]()
	{
	   Server::run_linear();
    	}));
    }

    std::for_each(t.begin(), t.end(), [](std::thread &tt)
    {
	tt.join();
    });
*/
    std::thread t[m_config.threads];
    for (int i =0; i < m_config.threads;i++){
	t[i] = std::thread(&Server::run_linear,this);	
    }
    for (int i = 0; i < m_config.threads; i++){
	t[i].join();
    }
    //exit(0);
    return;
//    throw TodoError("3", "You need to implement thread-pool mode");

}

void Server::handle(TcpConnection* conn) const
{

    Controller const* controller = nullptr;

    try
    {
        // creating req will parse the incoming request
        Request req(m_config, *conn);

        // creating res as an empty response
        Response res(m_config, *conn);

        // Printing the request will be helpful to tell what our server is seeing
        req.print();

        std::string path = req.get_path();

        // This will route a request to the right controller
        // You only need to change this if you rename your controllers or add more routes
        if (path == "/hello-world")
        {
            controller = new TextController(m_config, "Hello world!\n");
        }
        else if (path.find("/script") == 0)
        {
            controller = new ExecScriptController(m_config, "/script");
        }
        else
        {
            controller = new SendFileController(m_config);
        }

        // Whatever controller we picked needs to be run with the given request and response
        controller->run(req, res);
    }
    catch (RequestError const& e)
    {
        d_warnf("Error parsing request: %s", e.what());
        
        Controller::send_error_response(m_config, conn, e.status, "Error while parsing request\n");
    }
    catch (ControllerError const& e)
    {
        d_warnf("Error while handling request: %s", e.what());

        Controller::send_error_response(m_config, conn, HttpStatus::InternalServerError, "Error while handling request\n");
    }
    catch (ResponseError const& e)
    {
        d_warnf("Error while creating response: %s", e.what());

        Controller::send_error_response(m_config, conn, HttpStatus::InternalServerError, "Error while handling response\n");
    }
    catch (ConnectionError const& e)
    {
        // Do not try to write a response when we catch a ConnectionError, because that will likely just throw
        d_errorf("Connection error: %s", e.what());
    }
    catch (TodoError const& e)
    {
        d_errorf("You tried to use unimplemented functionality: %s", e.what());
    }

    // Dont forget about freeing memory!
    delete controller;
}

Server::~Server() noexcept
{
    //release resources acquired in the constructor.

    if (close(m_master) == -1)
    {
        d_error("Could not close master socket");
    }
}
