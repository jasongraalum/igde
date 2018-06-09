//
//  owner_menu.cpp
//  igde
//
//  Created by Jason Graalum on 6/1/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#include "igde_control.hpp"

IGDE_Control::IGDE_Control()
{
    IGDE_Control(3001, "localhost");
}

IGDE_Control::IGDE_Control(int owner_portno, const char *name)
{
    this->owner_portno = owner_portno;
    this->owner_socket_fd = 0;
    
    this->owner_hostname = new char[strlen(name)];
    std::strcpy(this->owner_hostname,name);
    
    this->status = O_DISCONNECTED;
}

void IGDE_Control::start()
{
    
    // Pass menu thread to TCP loop
    this->menu_loop();
    
}
void IGDE_Control::display_menu()
{
    std::cout << "IGDE Document Owner Menu" << std::endl;
    std::cout << "========================" << std::endl;
    switch (this->status) {
        case O_DISCONNECTED:
            std::cout << O_MENU_CONNECT << " - " << O_MENU_CONNECT_TXT << std::endl;
            std::cout << O_MENU_EXIT << " - " <<  O_MENU_EXIT_TXT << std::endl;
            break;
        case O_NEW_OWNER:
            break;
        case O_CONNECTED:
            std::cout << O_MENU_DISCONNECT << " - " <<  O_MENU_DISCONNECT_TXT << std::endl;
            std::cout << O_MENU_START_SESSION << " - " <<  O_MENU_START_SESSION_TXT << std::endl;
            break;
        case O_ACCEPTING:
        case O_FULL:
//            std::cout << O_MENU_UPLOAD << " - " <<  O_MENU_UPLOAD_TXT << std::endl;
//            std::cout << O_MENU_DOWNLOAD << " - " <<  O_MENU_DOWNLOAD_TXT << std::endl;
            std::cout << O_MENU_END_SESSION << " - " <<  O_MENU_END_SESSION_TXT << std::endl;
            std::cout << O_MENU_LIST_EDITORS << " - " <<  O_MENU_LIST_EDITORS_TXT << std::endl;
            break;
        default:
            std::cout << "Unknown state" << std::endl;
    }
}

void IGDE_Control::menu_loop()
{
    int menu_item;
    std::list<IGDE_Session *>::iterator session_to_end_it;
    
    // Open socket to listen
    // Add event to queue to listen on new port
    //
    // Open the socket
    //
    if(this->connect_to_owner() == 0) {
        error("Unable to open port");
        exit(EXIT_FAILURE);
    }
    
    do {
        this->display_menu();
        
        std::cin >> menu_item;
        std::cin.ignore();
        
        switch(menu_item) {
                // Connect to Server
            case O_MENU_CONNECT:
                this->connect_to_server();
                break;
                // Disconnect from Server
            case O_MENU_DISCONNECT:
                this->disconnect_from_server();
                break;
                // Start Session
            case O_MENU_START_SESSION:
                this->start_session();
                break;
                // End Session
            case O_MENU_END_SESSION:
                this->end_session();
                break;
                // List Sessions
                /*
                 case O_MENU_LIST_SESSIONS:
                 this->list_sessions();
                 break;
                 */
                // List Editors
            case O_MENU_LIST_EDITORS:
                this->list_editors();
                break;
            case O_MENU_EXIT:
                break;
            default:
                break;
        }
        
    } while(menu_item != O_MENU_EXIT);
    this->stop_owner();
}

//
// Handshake to connect to the server
//
// This runs when the controller starts up.
//
int IGDE_Control::connect_to_owner()
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int addr_error;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string portno_str = std::to_string(this->owner_portno);
    char const *portno_char = portno_str.c_str();
    if ((addr_error = getaddrinfo(this->owner_hostname, portno_char, &hints, &servinfo)) != 0) {
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
 
    this->owner_socket_fd = sockfd;
    memcpy(&(this->owner_addr), &(p->ai_addr), sizeof(struct sockaddr));
    freeaddrinfo(servinfo); // all done with this structure
    
    std::cout << sockfd << std::endl;
    
    // Send Controller Type Message
    IGDE_Message * owner_reply = new IGDE_Message;
    owner_reply->get_msg(this->owner_socket_fd);
    if(owner_reply->get_cmd() != S_SEND_TYPE)
    {
        std::cout << "Incorrect Owner Reply. Disconnecting" << std::endl;
        this->owner_socket_fd = 0;
        return -1;
    }
    else
    {
        std::cout << "Correct Owner Reply. Connected" << std::endl;
        char connect_msg[] = "I'm a controller";
        IGDE_Message * hello_msg = new IGDE_Message(CONTROL_TYPE, CO_SET_CONTROL_TYPE, this->owner_socket_fd, 0, connect_msg);
        hello_msg->send_msg(this->owner_socket_fd);
        return sockfd;
    }
    
}

//
// Sending command to owner for it to connect to master server
//
void IGDE_Control::connect_to_server()
{
   
    std::string server_name;
    int server_port;
    
    std::cout << "Enter server hostname:";
    std::getline(std::cin, server_name);
    std::cout << "Enter port number:";
    std::cin >> server_port;
    std::cin.ignore();
    
    std::string data = server_name + ":" + std::to_string(server_port);
    
    // Send command to owner to connect to server
    IGDE_Message * connect_msg = new IGDE_Message(CONTROL_TYPE, CO_CONNECT_SRV, (TBD_T)data.length(), 0, data);
    connect_msg->send_msg(this->owner_socket_fd);
    
    // Wait for owner to reply.
    IGDE_Message * reply = new IGDE_Message;
    reply->get_msg(this->owner_socket_fd);
    
    if(reply->get_cmd() == OC_SRV_CONNECTED)
    {
        std::cout << "Owner is connected to server" << std::endl;
        this->status = O_CONNECTED;
    }
    else
    {
        std::cout << "Owner was unable to connect to server" << std::endl;
        this->status = O_DISCONNECTED;
    }
}

//
// Send command to owner to disconnect from the server
//
void IGDE_Control::disconnect_from_server()
{
    char msg_text[] = "Disconnecting";
    
    if(this->status == O_CONNECTED)
    {
        IGDE_Message * connect_msg = new IGDE_Message(CONTROL_TYPE, CO_DISCONNECT_SRV, (TBD_T)std::strlen(msg_text), 0, msg_text);
        connect_msg->send_msg(this->owner_socket_fd);
        
        IGDE_Message * reply = new IGDE_Message;
        reply->get_msg(this->owner_socket_fd);
        
        if(reply->get_cmd() == OC_SRV_DISCONNECTED)
        {
            std::cout << "Owner is disconnected from server" << std::endl;
            this->status = O_DISCONNECTED;
        }
        else
        {
            std::cout << "Owner was unable to disconnect from the server" << std::endl;
        }
    }
    else
        std::cout << "Owner is not connected to a server" << std::endl;
    
}
void IGDE_Control::start_session()
{
  
    std::cin.clear();
    std::cout << "Enter session name:";
    std::getline(std::cin, this->session_name);
    
    std::cout << "Enter new session port number:";
    std::cin >> this->session_portno;
    std::cin.ignore();

    std::cout << "Enter filename:";
    std::getline(std::cin, this->session_filename);

    std::cout << "Enter maximum number of editors:";
    std::cin >> this->session_max_editors;
    std::cin.ignore();

    std::string data = this->session_name + ":" + std::to_string(this->session_portno) + ":" + this->session_filename + ":" + std::to_string(this->session_max_editors);
    IGDE_Message * start_msg = new IGDE_Message(CONTROL_TYPE, CO_START_SESSION, (TBD_T)data.length(), 0, data);
    start_msg->send_msg(this->owner_socket_fd);
    
    IGDE_Message * start_reply = new IGDE_Message;
    start_reply->get_msg(this->owner_socket_fd);
    
    if(start_reply->get_cmd() == OC_UPLOAD_FILE)
    {
        std::cout << "Session is started and open" << std::endl;
        this->status = O_UPLOADING;
        
        IGDE_Message * start2_msg = new IGDE_Message(CONTROL_TYPE, CO_UPLOAD_STARTING, (TBD_T)data.length(), 0, NULL);
        start2_msg->send_msg(this->owner_socket_fd);
        
        IGDE_Message * start2_reply = new IGDE_Message;
        start2_reply->get_msg(this->owner_socket_fd);
        
        if(start2_reply->get_cmd() == OC_UPLOAD_GO)
        {
            if(this->upload_file())
            {
                IGDE_Message * end_msg = new IGDE_Message(CONTROL_TYPE, CO_OPEN_SESSION, 0, 0, NULL);
                end_msg->send_msg(this->owner_socket_fd);
            }
            else {
                IGDE_Message * end_msg = new IGDE_Message(CONTROL_TYPE, CO_CMD_FAILED, 0, 0, NULL);
                end_msg->send_msg(this->owner_socket_fd);
                std::cout << "Owner was unable to start the session" << std::endl;
                this->status = O_CONNECTED;
                return;
            }
        }
        IGDE_Message * start3_reply = new IGDE_Message;
        start3_reply->get_msg(this->owner_socket_fd);
        
        if(start3_reply->get_cmd() == OC_SESSION_STARTED)
            this->status = O_ACCEPTING;
    }
    else
    {
        std::cout << "Owner was unable to start the session" << std::endl;
        return;
    }
    
    
}
void IGDE_Control::end_session()
{
    std::cout << "Enter filename for edits:";
    std::getline(std::cin, this->editted_filename);

    IGDE_Message * start_msg = new IGDE_Message(CONTROL_TYPE, CO_END_SESSION, 0, 0, NULL);
    start_msg->send_msg(this->owner_socket_fd);
    
    IGDE_Message * start_reply = new IGDE_Message;
    start_reply->get_msg(this->owner_socket_fd);
    
    if(start_reply->get_cmd() == OC_ENDING_SESSION)
    {
        std::cout << "Session is ending" << std::endl;
        this->status = O_ENDING;
        
        IGDE_Message * start2_msg = new IGDE_Message(CONTROL_TYPE, CO_DOWNLOAD_FILE, 0, 0, NULL);
        start2_msg->send_msg(this->owner_socket_fd);
        
        if(this->download_file())
        {
            IGDE_Message * end_msg = new IGDE_Message(CONTROL_TYPE, CO_DOWNLOAD_DONE, 0, 0, NULL);
            end_msg->send_msg(this->owner_socket_fd);
            this->status = O_CONNECTED;

        }
        else {
            IGDE_Message * end_msg = new IGDE_Message(CONTROL_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            end_msg->send_msg(this->owner_socket_fd);
            std::cout << "Owner was unable to end the session" << std::endl;
            this->status = O_CONNECTED;
            return;
        }
        
    }
}
    

void IGDE_Control::list_editors()
{
    
}

int IGDE_Control::upload_file()
{
    
    char buffer[UPLOAD_CHUNK_SIZE];
    // Can we read the file?
    this->session_ifd.open(this->session_filename);
    if(this->session_ifd.is_open())
    {
    
        std::streamsize char_count;
        IGDE_Message * reply = new IGDE_Message;

        do
        {
            this->session_ifd.read(buffer, UPLOAD_CHUNK_SIZE);
            char_count = this->session_ifd.gcount();
            buffer[char_count] = '\0';
            std::cout << "buffer == " << buffer << std::endl;
            

            if(char_count < UPLOAD_CHUNK_SIZE)
            {
                IGDE_Message * msg = new IGDE_Message(CONTROL_TYPE, CO_UPLOAD_LAST_CHUNK, (TBD_T)char_count, 0, buffer);
                msg->send_msg(this->owner_socket_fd);
            }
            else
            {
                IGDE_Message * msg = new IGDE_Message(CONTROL_TYPE, CO_UPLOAD_CHUNK, (TBD_T)char_count, 0, buffer);
                msg->send_msg(this->owner_socket_fd);
            }
            reply->get_msg(this->owner_socket_fd);
            
        } while(reply->get_cmd() == OC_UPLOAD_CHUNK_OK);
        
        if(reply->get_cmd() == OC_UPLOAD_DONE)
            return 1;
        else
            return 0;
    }
    else
    {
        std::cout << "Unable to open file: " << this->session_filename << std::endl;
        return 0;
    }
}

int IGDE_Control::download_file()
{
    IGDE_Message * reply = new IGDE_Message;
    // Can we read the file?
    this->session_ofd.open(this->editted_filename);
    if(this->session_ofd.is_open())
    {
        
        do {
            reply->get_msg(this->owner_socket_fd);
            if(reply->get_cmd() == OC_DOWNLOAD_CHUNK || reply->get_cmd() == OC_DOWNLOAD_LAST_CHUNK)
                this->session_ofd << reply->get_data();

        } while(reply->get_cmd() == OC_DOWNLOAD_CHUNK);
        
        if(reply->get_cmd() == OC_DOWNLOAD_LAST_CHUNK)
        {
            this->session_ofd << reply->get_data();
            IGDE_Message * msg = new IGDE_Message(CONTROL_TYPE, CO_DOWNLOAD_DONE, 0, 0, NULL);
            msg->send_msg(this->owner_socket_fd);
            this->status = O_FREE;
            this->session_ofd.close();
            return 1;
        }
        else
        {
            IGDE_Message * msg = new IGDE_Message(CONTROL_TYPE, CO_CMD_FAILED, 0, 0, NULL);
            msg->send_msg(this->owner_socket_fd);
            return 0;
        }


    }
    else
    {
        std::cout << "Unable to open new file for edits: " << this->editted_filename << std::endl;
        IGDE_Message * msg = new IGDE_Message(CONTROL_TYPE, CO_CMD_FAILED, 0, 0, NULL);
        msg->send_msg(this->owner_socket_fd);
        return 0;
    }
    
}


void IGDE_Control::stop_owner()
{
    
}
