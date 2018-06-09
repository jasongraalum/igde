//
//  server.cpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#include "igde_server.hpp"

//

// Create a new server listening to port 'portno'
//
IGDE_Server::IGDE_Server(int portno)
{
    this->server_portno = portno;
    this->server_socket_fd = 0;
    
    if(this->start())
        info_msg("Server exitting..");
    
}

int IGDE_Server::start()
{
    int server_queue;
    struct kevent server_events;
    
    //
    // Open the socket
    //
    if(this->open_server_socket()) {
        error("Unable to open port");
        exit(EXIT_FAILURE);
    }
    
    if ((server_queue = kqueue()) == -1) {
        error("kqueue threw an error");
        exit(EXIT_FAILURE);
    }

    EV_SET(&server_events, this->server_socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(server_queue, &server_events, 1, NULL, 0, NULL) == -1)
        error("kevent threw an error");
    
    return(this->listen_loop(server_queue));
}


// Main loop of the server
//
int IGDE_Server::listen_loop(int server_queue)
{
    struct kevent server_ev_set;
    struct kevent client_ev_list[MAX_SERVER_CONNECTIONS];
    
    int nev, i;
    
    struct sockaddr_storage client_addr;
    socklen_t socklen = sizeof(client_addr);
    int fd;
    
    while(1)
    {
        info_msg("Waiting for new server socket event");

        nev = kevent(server_queue, NULL, 0, client_ev_list, MAX_SERVER_CONNECTIONS, NULL);
        info_msg("New server socket event");
        if (nev < 1)
        {
            error("kevent error in number of events loop");
            std::exit(-1);
        }
        
        //
        // For each kevent - cycle through the number of kevent messages(nev) in client_ev_list[]
        //
        for (i=0; i<nev; i++) {
            std::ostringstream msg;
            msg << i << " : " << client_ev_list[i].ident << " : " << client_ev_list[i].filter  << " : " << client_ev_list[i].flags;
            info_msg(msg.str());
            
            //
            // Process a client disconnecting
            //
            if (client_ev_list[i].flags & EV_EOF) {
                fd = (int)client_ev_list[i].ident;
                EV_SET(&server_ev_set, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                if (kevent(server_queue, &server_ev_set, 1, NULL, 0, NULL) == -1)
                {
                    error("kevent error in client disconnect");
                }
                this->remove_client(fd);
            }
            //
            // Process a new connection from a client
            //
            else if (client_ev_list[i].ident == this->server_socket_fd) {
                std::ostringstream msg;
                msg << "New Client with ident: " << client_ev_list[i].ident;
                info_msg(msg.str());
                
                fd = accept((int)client_ev_list[i].ident, (struct sockaddr *)&client_addr,&socklen);
                if (fd == -1)
                {
                    error("Server connection accept error");
                }
                else{
                    std::string info = "New socket fd : " + std::to_string(fd);
                    info_msg(info);
                }
                
                if (this->add_client(fd, (struct sockaddr_in *)&client_addr) == 0) {
                    EV_SET(&server_ev_set, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    if (kevent(server_queue, &server_ev_set, 1, NULL, 0, NULL) == -1)
                    {
                        error("kevent error in add_client");
                    }
                } else {
                    warning_msg("Server client connection refused");
                    close(fd);
                }
            }
            
            //
            // Process an incoming message from a previously connected client
            //
            else
            {
                for(int client_index = 0; client_index < MAX_CLIENTS; client_index++)
                {
                    if((int)client_ev_list[i].ident == this->clients[client_index].fd)
                    {
                        std::cout << "New editor message received " << std::endl;
                        IGDE_Message * msg = new IGDE_Message;
                        msg->get_msg(this->clients[client_index].fd);
                        this->process_message(client_index, msg);
                        std::cout << "Done processing message" << std::endl;
                    }
                }

            }
        }
    }
    
}

int IGDE_Server::add_client(int fd, struct sockaddr_in * client_addr)
{
    std::cout << "Adding new client:" << fd << std::endl;
    int client_index = 0;
    while(this->clients[client_index].fd != 0 && client_index < MAX_CLIENTS)
        client_index++;
    
    if(client_index == MAX_CLIENTS)
        return -1;
    
    this->clients[client_index].fd = fd;
    this->clients[client_index].type = UNKNOWN_TYPE;
    this->clients[client_index].state = S_NEW_CONN;
    memcpy(&(this->clients[client_index].addr), &client_addr, sizeof(sockaddr_storage));

    // Get Client Type Message
    char connect_msg[] = "Who are you?";
    
    IGDE_Message * hello_msg = new IGDE_Message(SERVER_TYPE, S_SEND_TYPE, 0, 0, connect_msg);
    if(hello_msg->send_msg(this->clients[client_index].fd) > 0)
        return 0;
    else
        return -1;

}

int IGDE_Server::remove_client(int client_index)
{
    int client_fd = this->clients[client_index].fd;
    this->clients[client_index].fd = 0;
    
    return close(client_fd);
}

int IGDE_Server::process_message(int client_index, IGDE_Message * msg)
{
    int client_type = this->clients[client_index].type;
    
    IGDE_Message * reply = NULL;
    char type_set[] = "Type Set";
    int cmd = msg->get_cmd();
    switch(client_type) {
        case UNKNOWN_TYPE:
            if(cmd == OS_SET_OWNER_TYPE)
            {
                this->clients[client_index].type = OWNER_TYPE;
                this->clients[client_index].state = O_FREE;
                reply = new IGDE_Message(SERVER_TYPE, SO_OWNER_TYPE_SET, (TBD_T)std::strlen(type_set), 0, type_set);
                reply->send_msg(this->clients[client_index].fd);
            }
            if(cmd == ES_SET_EDITOR_TYPE)
            {
                this->clients[client_index].type = EDITOR_TYPE;
                this->clients[client_index].state = E_NEW_EDITOR;
                reply = new IGDE_Message(SERVER_TYPE, SE_EDITOR_TYPE_SET, (TBD_T)std::strlen(type_set), 0, type_set);
                reply->send_msg(this->clients[client_index].fd);
            }
            break;
        case OWNER_TYPE:
            this->process_owner_message(client_index, msg);
            break;
        case EDITOR_TYPE:
            this->process_editor_message(client_index, msg);
            break;
        default:
            std::cout << "Unknown client type" << std::endl;
            break;
    }
    
    
    return 0;
}
int IGDE_Server::process_owner_message(int client_index, IGDE_Message * msg)
{
    int client_fd = this->clients[client_index].fd;
    int client_state = this->clients[client_index].state;
    int cmd = msg->get_cmd();
    std::string data(msg->get_data());

    IGDE_Message * reply;

    int result;
    
    std::cout << "Processing owner message(state = " << client_state << " : cmd = " << cmd << std::endl;
    switch(client_state) {
        case O_FREE:
            if(cmd == OS_REG_SESSION)
            {
                if(this->add_session(client_fd, msg))
                {
                    char msg_text[] = "Session Registered";
                    reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_REGD, 0, 0, msg_text);
                    this->clients[client_index].state = O_ACCEPTING;
                }
                else
                {
                    char msg_text[] = "Session NOT Registered";
                    reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_NOT_REGD, 0, 0, msg_text);
                }
                result = reply->send_msg(client_fd);

            }
            if(cmd == OS_BYE)
            {
                char msg_text[] = "Bye";
                this->clients[client_index].state=O_FULL;
                reply = new IGDE_Message(SERVER_TYPE, SO_BYE, 0, 0, msg_text);
                result = reply->send_msg(client_fd);
            }
            break;
        case O_ACCEPTING:
            if(cmd == OS_END_SESSION)
            {
                if(this->remove_session(client_fd,msg))
                {
                    char msg_text[] = "Session Removed";
                    reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_ENDED, 0, 0, msg_text);
                    this->clients[client_index].state = O_FREE;
                }
                else
                {
                    char msg_text[] = "Session NOT Removed";
                    reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_NOT_ENDED, 0, 0, msg_text);
                }
                result = reply->send_msg(client_fd);
            }
            if(cmd == OS_CLOSE_SESSION)
            {
                this->clients[client_index].state = O_FULL;
                char msg_text[] = "Session Closed";
                reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_CLOSED, 0, 0, msg_text);
                result = reply->send_msg(client_fd);
            }
            break;
        case O_FULL:
            if(cmd == OS_END_SESSION)
            {
                if(this->remove_session(client_fd,msg))
                {
                    char msg_text[] = "Session Removed";
                    reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_ENDED, 0, 0, msg_text);
                    this->clients[client_index].state = O_FREE;

                }
                else
                {
                    char msg_text[] = "Session NOT Removed";
                    reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_NOT_ENDED, 0, 0, msg_text);
                }
                result = reply->send_msg(client_fd);
            }
            if(cmd == OS_OPEN_SESSION)
            {
                this->clients[client_index].state = O_ACCEPTING;
                char msg_text[] = "Session Opened";
                reply = new IGDE_Message(SERVER_TYPE, SO_SESSION_OPENED, 0, 0, msg_text);
                result = reply->send_msg(client_fd);
            }
            break;
        default:
            break;
    }
    return 0;
}

int IGDE_Server::process_editor_message(int client_index, IGDE_Message * msg)
{
    //int client_fd = this->clients[client_index].fd;
    int client_state = this->clients[client_index].state;
    
    switch(client_state) {
        case E_NEW_EDITOR:
            break;
        case E_EDITOR_WAITING:
            break;
        case E_EDITOR_CLOSED:
            break;
        default:
            break;
    }
    return 0;
}


int IGDE_Server::open_server_socket()
{
    // Build hints structure
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string so_portno_str = std::to_string(this->server_portno);
    char const *so_portno_char = so_portno_str.c_str();
    int addr_error = getaddrinfo("127.0.0.1", so_portno_char, &hints, &(this->server_addr));

    if(addr_error)
        perror("getaddrinfo");
    
    //create a master socket
    if((this->server_socket_fd = socket(this->server_addr->ai_family, this->server_addr->ai_socktype, this->server_addr->ai_protocol)) == 0)
    {
        perror("socket failed");
        return(EXIT_FAILURE);
    }

    if (bind(this->server_socket_fd, this->server_addr->ai_addr, this->server_addr->ai_addrlen)<0)
    {
        perror("bind failed");
        return(EXIT_FAILURE);
    }
    
    //try to specify maximum of pending connections for the master socket
    if (listen(this->server_socket_fd, MAX_SERVER_CONNECTIONS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Listener on port %d \n", this->server_portno);

    return(EXIT_SUCCESS);
}




int IGDE_Server::add_session(int client_id, IGDE_Message * msg)
{
    session * new_session = new session;
    std::string data(msg->get_data());
    
    int epos = (int)data.find(":");
    new_session->session_name = data.substr(0,epos);
    data = data.substr(epos+1,data.length());
    
    epos = (int)data.find(":");
    new_session->portno = std::stoi(data.substr(0,epos));
    data = data.substr(epos+1,data.length());
    
    epos = (int)data.find(":");
    new_session->filename = data.substr(0,epos);
    data = data.substr(epos+1,data.length());
    
    epos = (int)data.find(":");
    new_session->max_editors = std::stoi(data.substr(0,epos));
    
    size_t org_size = this->session_list.size();
    this->session_list.push_back(new_session);
    
    return (int)(this->session_list.size() - org_size);
}

int IGDE_Server::remove_session(int client_id, IGDE_Message * msg)
{

    std::string session_name(msg->get_data());
    size_t org_size = this->session_list.size();

    for(std::list<session *>::iterator it = this->session_list.begin(); it != this->session_list.begin(); it++)
    {
        if((*it)->session_name == session_name)
            it = this->session_list.erase(it);
    }

    return 1;
    //return (int)(org_size - this->session_list.size());

}
