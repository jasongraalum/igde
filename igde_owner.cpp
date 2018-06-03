//
//  owner.cpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#include "igde_owner.hpp"

IGDE_Owner::IGDE_Owner()
{
    IGDE_Owner(3001);
}

IGDE_Owner::IGDE_Owner(int control_portno)
{
    this->control_portno = control_portno;
    this->control_socket_fd = 0;

    this->session_portno = 0;
    this->session_socket_fd = 0;
    this->state = O_DISCONNECTED;
}

void IGDE_Owner::start()
{
    if(this->open_control_socket())
    {
        error("Unable to open menu port");
        exit(EXIT_FAILURE);
    }
    
    if((this->owner_queue = kqueue()) == -1)
    {
        error("kqueue error");
        exit(EXIT_FAILURE);
    }
    
    EV_SET(&(this->owner_event_set), this->control_socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if(kevent(this->owner_queue, &(this->owner_event_set), 1, NULL, 0, NULL) == -1)
    {
        error("kevent error");
        exit(EXIT_FAILURE);
    }
    
    // Pass menu thread to TCP loop
    this->message_loop(this->owner_queue);
}

//
// Open port for control process to connect in
//
int IGDE_Owner::open_control_socket()
{
    // Build hints structure
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string control_portno_str = std::to_string(this->control_portno);
    char const *control_portno_char = control_portno_str.c_str();
    
    int addr_error = getaddrinfo("127.0.0.1", control_portno_char, &hints, &(this->owner_addr));
    
    if(addr_error)
        perror("getaddrinfo");
    
    //create a menu socket
    if((this->control_socket_fd = socket(this->owner_addr->ai_family, this->owner_addr->ai_socktype, this->owner_addr->ai_protocol)) == 0)
    {
        perror("socket failed");
        return(EXIT_FAILURE);
    }
    
    if (bind(this->control_socket_fd, this->owner_addr->ai_addr, this->owner_addr->ai_addrlen)<0)
    {
        perror("bind failed");
        return(EXIT_FAILURE);
    }
    
    //try to specify maximum of pending connections for the master socket
    if (listen(this->control_socket_fd, MAX_SERVER_CONNECTIONS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Listener on port %d \n", this->control_portno);
    
    return(EXIT_SUCCESS);
}

//
// Main loop of the owner
//
void IGDE_Owner::message_loop(int owner_queue)
{
    struct kevent client_ev_list[MAX_SERVER_CONNECTIONS];
    
    int nev, i;
    
    struct sockaddr_storage client_addr;
    socklen_t socklen = sizeof(client_addr);
    int fd;
    
    while(1)
    {
        info_msg("Waiting for new owner socket event");
        
        nev = kevent(owner_queue, NULL, 0, client_ev_list, MAX_SERVER_CONNECTIONS, NULL);
        info_msg("New owner socket event");
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
                EV_SET(&(this->owner_event_set), fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                if (kevent(owner_queue, &(this->owner_event_set), 1, NULL, 0, NULL) == -1)
                {
                    error("kevent error in client disconnect");
                }
                if(fd == this->control_socket_fd)
                    this->disconnect_control();
                else
                    this->disconnect_editor(fd);
            }
            //
            // Process a new connection from a controller
            //
            else if (client_ev_list[i].ident == this->control_socket_fd) {
                std::ostringstream msg;
                msg << "New Controller with ident: " << client_ev_list[i].ident;
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
                
                // Connect to the Controller
                if (this->connect_control(fd, (struct sockaddr_in *)&client_addr) == 0) {
                    EV_SET(&(this->owner_event_set), fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    if (kevent(owner_queue, &(this->owner_event_set), 1, NULL, 0, NULL) == -1)
                    {
                        error("kevent error in add_client");
                    }
                } else {
                    warning_msg("Server client connection refused");
                    close(fd);
                }
            }
            //
            // Process a new connection from an editor
            //
            else if (client_ev_list[i].ident == this->session_socket_fd) {
                std::ostringstream msg;
                msg << "New Editor with ident: " << client_ev_list[i].ident;
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
                
                //
                // Connect to an editor
                //
                if (this->connect_editor(fd, (struct sockaddr_in *)&client_addr) == 0) {
                    EV_SET(&(this->owner_event_set), fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    if (kevent(owner_queue, &(this->owner_event_set), 1, NULL, 0, NULL) == -1)
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
                // If incoming message from the server
                if ((int)client_ev_list[i].ident == this->server_socket_fd)
                {
                    std::cout << "New server message received " << std::endl;
                    IGDE_Message * msg = new IGDE_Message;
                    msg->get_msg(this->server_socket_fd);
                    this->process_srv_msg(msg);
                }
                else
                    // If incoming message from the controller
                    if ((int)client_ev_list[i].ident == this->controller.fd)
                    {
                        std::cout << "New contoller message received " << std::endl;
                        IGDE_Message * msg = new IGDE_Message;
                        msg->get_msg(this->controller.fd);
                        this->process_control_msg(msg);
                    }
                    else
                    {
                        // Check if connection from editor
                        for(std::list<editor *>::iterator e_it = this->editors.begin();
                            e_it != this->editors.end();
                            e_it++)
                        {
                            if((int)client_ev_list[i].ident == (*e_it)->fd)
                            {
                                std::cout << "New editor message received " << std::endl;
                                IGDE_Message * msg = new IGDE_Message;
                                msg->get_msg((*e_it)->fd);
                                this->process_editor_msg(e_it,msg);
                            }
                        }
                    }
            }
        }
    }
    
}

//
// Process incoming connect request from Controller type
//
int IGDE_Owner::connect_control(int fd, struct sockaddr_in * controller_addr)
{
    this->controller.name = "unknown";
    this->controller.fd = fd;
    this->controller.addr = new struct sockaddr_storage;
    memcpy(controller.addr, &controller_addr, sizeof(sockaddr_storage));
    
    // Get Client Type Message
    char connect_msg[] = "Who are you?";
    IGDE_Message * hello_msg = new IGDE_Message(OWNER_TYPE, S_SEND_TYPE, controller.fd, 0, connect_msg);
    
    if(hello_msg->send_msg(controller.fd) > 0)
    {
        IGDE_Message * reply = new IGDE_Message;
        reply->get_msg(controller.fd);
        if(reply->get_cmd() != CO_SET_CONTROL_TYPE)
        {
            std::cout << "Incorrect Contoller Reply. Disconnecting" << std::endl;
            close(this->controller.fd);
            this->controller.fd = 0;
            this->controller.state = O_DISCONNECTED;
            return -1;
        }
        else
        {
            this->controller.state = O_CONNECTED;
            std::cout << "Correct Controller Reply. Connected" << std::endl;
            return 0;
        }
    }
    else
    {
        std::cout << "Unable to send message" << std::endl;
        close(this->controller.fd);
        this->controller.fd = 0;
        this->controller.state = O_DISCONNECTED;
        return -1;
    }
    
}
int IGDE_Owner::disconnect_control()
{
    close(this->controller.fd);
    this->controller.fd = 0;
    this->controller.state = O_DISCONNECTED;
    return 0;
}


// Handshake to connect to the server
int IGDE_Owner::connect_to_server(std::string hostname, int portno)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int addr_error;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string portno_str = std::to_string(portno);
    char const *portno_char = portno_str.c_str();
    if ((addr_error = getaddrinfo(hostname.c_str(), portno_char, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr_error));
        return 1;
    }
    
    // loop through all the results and connect to the first we can
    sockfd = NULL;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    std::cout << "Attempting to connect to server: " << hostname << ":" << portno << std::endl;

    this->server_socket_fd = sockfd;
    this->server_hostname = hostname;
    this->server_portno = portno;
    
    memcpy(&(this->server_addr), &(p->ai_addr), sizeof(struct sockaddr));
    freeaddrinfo(servinfo); // all done with this structure
    
    EV_SET(&(this->owner_event_set), this->server_socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    
    if (kevent(this->owner_queue, &(this->owner_event_set), 1, NULL, 0, NULL) == -1)
        error("kevent threw an error");
    
    std::cout << "Connected to: " << this->server_hostname << ":" << this->server_portno << std::endl;
    
    this->state = O_NEW_OWNER;
    return sockfd;
}

int IGDE_Owner::disconnect_from_server()
{
    // Remove event from queue
    char disconnect_msg[] = "See ya!";
    IGDE_Message * bye_msg = new IGDE_Message(OWNER_TYPE, OS_BYE, 0, 0, disconnect_msg);
    
    EV_SET(&(this->owner_event_set), this->server_socket_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    bye_msg->send_msg(this->server_socket_fd);
    
    close(this->server_socket_fd);
    this->server_socket_fd = 0;
    return 0;
}



std::string IGDE_Owner::list_editors()
{
    std::string list = "Editors:";
    for(std::list<editor *>::iterator editor_it = editors.begin(); editor_it!=this->editors.end(); editor_it++)
    {
        list = list + ":" + (*editor_it)->name;
    }
    return list;
}

void IGDE_Owner::stop_owner()
{
    this->end_session();
    close(this->session_socket_fd);
    close(this->control_socket_fd);
    exit(0);
}





int IGDE_Owner::process_srv_msg(IGDE_Message * msg)
{
    std::cout << "Processing message from server" << std::endl;

    int msg_cmd = msg->get_cmd();
    std::cout << "Owner state: " << this->state << std::endl;
    std::cout << "Server command: " << msg_cmd << std::endl;
    
    char huh_msg[] = "Huh?";
    char owner_msg[] = "I'm an onwer";
    char connect_ok[] = "I connected!";

    int sent_bytes = 0;
    IGDE_Message * srv_reply = NULL;
    switch(this->state) {
        case O_DISCONNECTED :
            // Shouldn't get a message from the server in this state
        case O_NEW_OWNER :
            // Being asked for type
            if(msg_cmd == S_SEND_TYPE)
            {
                srv_reply = new IGDE_Message(OWNER_TYPE, OS_SET_OWNER_TYPE, 0, 0, owner_msg);
            }
            else
            {
                // Reply to Controller and set state to O_CONNECTED
                this->state = O_CONNECTED;
                IGDE_Message * controller_reply = NULL;
                controller_reply = new IGDE_Message(OWNER_TYPE, OC_SRV_CONNECTED, 0, 0, connect_ok);
                controller_reply->send_msg(this->controller.fd);
            }
            break;
        case O_CONNECTED :
            srv_reply = new IGDE_Message(OWNER_TYPE, OS_OK, 0, 0, huh_msg);
            break;
        case O_ACCEPTING :
            srv_reply = new IGDE_Message(OWNER_TYPE, OS_OK, 0, 0, huh_msg);
            break;
        case O_FULL :
            srv_reply = new IGDE_Message(OWNER_TYPE, OS_OK, 0, 0, huh_msg);

            break;
        default:
            srv_reply = new IGDE_Message(OWNER_TYPE, OS_OK, 0, 0, huh_msg);
            break;
    }
    if(srv_reply)
        sent_bytes = srv_reply->send_msg(this->server_socket_fd);
    return(sent_bytes);
}

int IGDE_Owner::process_editor_msg(std::list<editor *>::iterator editor_it, IGDE_Message * msg)
{
    std::cout << "Processing message from editor" << std::endl;

    return 0;
}
int IGDE_Owner::connect_editor(int fd, struct sockaddr_in * editor_addr)
{
    editor * new_editor = new editor;
    new_editor->name = "unknown";
    new_editor->fd = fd;
    new_editor->addr = new struct sockaddr_storage;
    memcpy(new_editor->addr, &editor_addr, sizeof(sockaddr_storage));
    
    this->editors.push_back(new_editor);
    
    // Get Client Type Message
    char connect_msg[] = "Who are you?";
    IGDE_Message * hello_msg = new IGDE_Message(OWNER_TYPE, S_SEND_TYPE, new_editor->fd, 0, connect_msg);
    
    if(hello_msg->send_msg(new_editor->fd) > 0)
        return 0;
    else
        return -1;
    
}


int IGDE_Owner::disconnect_editor(int fd)
{
    
    return 0;
}
int IGDE_Owner::process_control_msg(IGDE_Message * msg)
{
    std::cout << "Processing message from controller" << std::endl;
    int result = 0;
    int msg_cmd = msg->get_cmd();
    std::string data(msg->get_data());
    
    IGDE_Message * reply;

    switch(msg_cmd) {
            //
            // Command to connect to server
            //
        case CO_CONNECT_SRV :
            if(this->state == O_DISCONNECTED)
            {
                std::string server_hostname = data.substr(0,data.find(":"));
                int server_portno = std::stoi(data.substr(data.find(":")+1,data.length()));

                if(this->connect_to_server(server_hostname, server_portno) == 0)
                {
                    reply = new IGDE_Message(OWNER_TYPE, OC_SRV_NOT_CONNECTED, 0, 0, server_hostname);
                    result = reply->send_msg(this->controller.fd);
                }
            }
            else
            {
                std::string msg_text;
                msg_text = this->server_hostname + ":" + std::to_string(this->server_portno);
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, msg_text);
                result = reply->send_msg(this->controller.fd);
            }
            break;
            //
            // Command to disconnect from server
            //
        case CO_DISCONNECT_SRV :
            if(this->state == O_CONNECTED)
            {
                if(this->disconnect_from_server()==0)
                {
                    reply = new IGDE_Message(OWNER_TYPE, OC_SRV_DISCONNECTED, 0, 0, NULL);
                    this->state = O_DISCONNECTED;
                }
                else
                {
                    reply = new IGDE_Message(OWNER_TYPE, OC_SRV_NOT_DISCONNECTED, 0, 0, NULL);
                }
            }
            else
                reply = new IGDE_Message(OWNER_TYPE, OS_OK, 0, 0, NULL);
            
            result = reply->send_msg(this->controller.fd);
            break;
        case CO_START_SESSION :
            if(this->state == O_CONNECTED)
            {
                if(this->start_session(data))
                {
                    // Notify server
                    reply = new IGDE_Message(OWNER_TYPE, OC_UPLOAD_FILE, 0, 0, NULL);
                    this->state = O_UPLOADING;
                }
                else
                {
                    reply = new IGDE_Message(OWNER_TYPE, OC_SESSION_NOT_STARTED, 0, 0, NULL);
                }
            }
            else
            {
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            }
            result = reply->send_msg(this->controller.fd);
            break;
        case CO_UPLOAD_STARTING:
            if(this->state == O_UPLOADING)
            {
                reply = new IGDE_Message(OWNER_TYPE, OC_UPLOAD_GO, 0, 0, NULL);
            }
            else
            {
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            }
            result = reply->send_msg(this->controller.fd);

            break;
        case CO_UPLOAD_CHUNK:
            if(this->state == O_UPLOADING)
            {
                if(this->upload_chunk(data))
                    reply = new IGDE_Message(OWNER_TYPE, OC_UPLOAD_CHUNK_OK, 0, 0, NULL);
                else
                    reply = new IGDE_Message(OWNER_TYPE, OC_UPLOAD_CHUNK_ERR, 0, 0, NULL);
            }
            else
            {
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            }
            result = reply->send_msg(this->controller.fd);

        case CO_UPLOAD_LAST_CHUNK:
            if(this->state == O_UPLOADING)
            {
                if(this->finalize_upload(data))
                {
                    reply = new IGDE_Message(OWNER_TYPE, OC_UPLOAD_DONE, 0, 0, NULL);
                }
                else
                    reply = new IGDE_Message(OWNER_TYPE, OC_UPLOAD_CHUNK_ERR, 0, 0, NULL);
            }
            else
            {
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            }
            result = reply->send_msg(this->controller.fd);
            break;
        case CO_OPEN_SESSION:
            if(this->state == O_SESSION_READY)
            {
                if(this->open_session_socket() == 0)
                {
                    std::string reg_session_data = this->session_name + ":" +
                    std::to_string(this->session_portno) + ":" +
                    this->session_filename + ":" +
                    std::to_string(this->session_max_editors);
                    
                    IGDE_Message * srv_msg = new IGDE_Message(OWNER_TYPE, OS_REG_SESSION, 0, 0, reg_session_data);
                    srv_msg->send_msg(this->server_socket_fd);
                    reply = new IGDE_Message(OWNER_TYPE, OC_SESSION_STARTED, 0, 0, NULL);
                }
                else
                    reply = new IGDE_Message(OWNER_TYPE, OC_SESSION_NOT_STARTED, 0, 0, NULL);

            }
            else
            {
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            }
            result = reply->send_msg(this->controller.fd);
            break;
        case CO_END_SESSION :
            if(this->state == O_ACCEPTING || this->state == O_FULL)
            {
                if(this->end_session())
                    reply = new IGDE_Message(OWNER_TYPE, OC_SESSION_ENDED, 0, 0, NULL);
                else
                    reply = new IGDE_Message(OWNER_TYPE, OC_SESSION_NOT_ENDED, 0, 0, NULL);
            }
            else
            {
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            }
            result = reply->send_msg(this->controller.fd);
            break;
        case CO_LIST_EDITORS :
            if(this->state == O_ACCEPTING)
            {
                std::string data = this->list_editors();
                reply = new IGDE_Message(OWNER_TYPE, OC_EDITOR_LIST, 0, 0, data);
            }
            else
            {
                reply = new IGDE_Message(OWNER_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            }
            result = reply->send_msg(this->controller.fd);
            break;
        default:
            break;
    }
    return result;

}
int IGDE_Owner::start_session(std::string data)
{
    int epos = (int)data.find(":");
    this->session_name = data.substr(0,epos);
    data = data.substr(epos+1,data.length());
    
    epos = (int)data.find(":");
    this->session_portno = std::stoi(data.substr(0,epos));
    data = data.substr(epos+1,data.length());
    
    epos = (int)data.find(":");
    this->session_filename = data.substr(0,epos);
    data = data.substr(epos+1,data.length());
    
    epos = (int)data.find(":");
    this->session_max_editors = std::stoi(data.substr(0,epos));

    return(this->open_session_file());
    
}
int IGDE_Owner::upload_chunk(std::string data)
{
    this->session_ofd << data;
    return((int)data.size());
}

int IGDE_Owner::finalize_upload(std::string data)
{
    this->session_ofd << data;
    this->session_ofd.close();
    this->state = O_SESSION_READY;
    return((int)data.size());
}
//
// Open port for session editor to connect in
//
int IGDE_Owner::open_session_socket()
{
    
    // Build hints structure
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string session_portno_str = std::to_string(this->session_portno);
    char const *session_portno_char = session_portno_str.c_str();
    
    int addr_error = getaddrinfo("127.0.0.1", session_portno_char, &hints, &(this->owner_addr));
    
    if(addr_error)
        perror("getaddrinfo");
    
    //create a menu socket
    if((this->session_socket_fd = socket(this->owner_addr->ai_family, this->owner_addr->ai_socktype, this->owner_addr->ai_protocol)) == 0)
    {
        perror("socket failed");
        return(EXIT_FAILURE);
    }
    
    if (bind(this->session_socket_fd, this->owner_addr->ai_addr, this->owner_addr->ai_addrlen)<0)
    {
        perror("bind failed");
        return(EXIT_FAILURE);
    }
    
    //try to specify maximum of pending connections for the master socket
    if (listen(this->session_socket_fd, MAX_SERVER_CONNECTIONS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Listener on port %d \n", this->session_portno);
    
    return(EXIT_SUCCESS);
}


int IGDE_Owner::end_session()
{
    
    for(std::list<editor *>::iterator e_it = this->editors.begin();
        e_it != this->editors.end();
        e_it++)
    {
        IGDE_Message * editor_msg = new IGDE_Message(OWNER_TYPE, OE_BYE, 0, 0, NULL);
        editor_msg->send_msg((*e_it)->fd);
    }
    
    // Send end message to any connected editors
    // Remove event from queue
    EV_SET(&(this->owner_event_set), this->session_socket_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    
    close(this->session_socket_fd);
    this->session_socket_fd = 0;
    return 1;
}


int IGDE_Owner::open_session_file()
{
    this->session_filepath  = this->temp_dir + this->session_filename;
    this->session_ofd.open(this->session_filepath);

    return(this->session_ofd.is_open());
}

long IGDE_Owner::close_session_ofd()
{
    IGDE_Message * reply;
    
    long chars_written = this->session_ofd.tellp();
    this->session_ofd.close();
 
    return chars_written;
}

