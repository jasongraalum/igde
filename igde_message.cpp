//
//  igde_message.cpp
//  igde
//
//  Created by Jason Graalum on 5/16/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#include "igde_message.hpp"
#include "igde_common.hpp"
#include <iomanip>
#include <sstream>
#include <string>

ID_T IGDE_Message::msg_index = 1;

IGDE_Message::IGDE_Message()
{
    const char * blank_msg = "----";
    IGDE_Message(0,0,0,0,(char *)blank_msg);
}

IGDE_Message::IGDE_Message(TYPE_T ct_val, CMD_T cmd_val, ID_T id_val, TBD_T length, char * msg)
{
    this->msg_index++;
    
    this->type = ct_val;
    this->cmd = cmd_val;
    this->id = this->msg_index;
    
    if(msg != NULL)
    {
        this->len = (TBD_T)std::strlen(msg);
        this->data = new char[this->len+1];
        strncpy(this->data, msg, this->len+1);
        this->data[this->len] = '\0';
    }
    else
    {
        char null_msg[] = "NULL Message";
        this->len = (TBD_T)std::strlen(null_msg);
        this->data = new char[this->len+1];
        std::strcpy(this->data,null_msg);
    }

}

IGDE_Message::IGDE_Message(TYPE_T ct_val, CMD_T cmd_val, ID_T id_val, TBD_T len_val, std::string data_str)
{
    this->msg_index++;
    
    this->type = ct_val;
    this->cmd = cmd_val;
    this->id = this->msg_index;
    this->len = (TBD_T)data_str.length();

    char * data_char = new char[this->len + 1];
    std::copy(data_str.begin(), data_str.end(), data_char);
    data_char[this->len] = '\0';

    this->data = new char[this->len+1];
    strncpy(this->data, data_char, this->len);
    this->data[this->len] = '\0';

    //this->print_message();
    std::cout << "Done building message" << std::endl;

}

int IGDE_Message::pack(char * msg_str)
{
    std::cout << "Packing message" << std::endl;

    this->print_message();
    int index = 0;
    memcpy(msg_str + index, &type, sizeof(TYPE_T));
    
    index += sizeof(type);
    memcpy(msg_str + index, &cmd, sizeof(CMD_T));

    index += sizeof(cmd);
    memcpy(msg_str + index, &id, sizeof(ID_T));

    index += sizeof(id);
    memcpy(msg_str + index, &len, sizeof(TBD_T));

    index += sizeof(len);
    memcpy(msg_str + index, this->data, len);

    index += len;

    return(index);
}


int IGDE_Message::unpack(char * msg_str)
{
    std::cout << "Unpacking message" << std::endl;
    
    memcpy(&(this->type), msg_str, sizeof(this->type));
    msg_str += sizeof(this->type);

    memcpy(&(this->cmd), msg_str, sizeof(this->cmd));
    msg_str += sizeof(this->cmd);
    
    memcpy(&(this->id), msg_str, sizeof(this->id));
    msg_str += sizeof(this->id);

    memcpy(&(this->len), msg_str, sizeof(this->len));
    msg_str += sizeof(this->len);

    this->data = new char[this->len+1];
    strncpy(this->data, msg_str, this->len);
    this->data[this->len] = '\0';

    this->print_message();
    
    return (int)this->len;
}


char * IGDE_Message::get_data()
{
    return this->data;
}

TYPE_T IGDE_Message::get_type()
{
    return this->type;
}
CMD_T IGDE_Message::get_cmd()
{
    return this->cmd;
}
ID_T IGDE_Message::get_id()
{
    return this->id;
}
TBD_T IGDE_Message::get_len()
{
    return this->len;
}
void IGDE_Message::print_message()
{
    
    std::cout << this->msg_index;
    std::cout << ":" << this->type;
    std::cout << ":" << this->cmd;
    std::cout << ":" << this->id;
    std::cout << ":" << this->len << std::endl;
    
    char data_to_print[20];
    std::strncpy(data_to_print,this->data,19);
    data_to_print[19] = '\0';
    std::cout << "Data:" << data_to_print << std::endl;
}


int IGDE_Message::get_msg(int fd)
{
    std::cout << std::endl << "Getting message from fd:" << fd << std::endl;
    size_t msg_size;
    char msg_buffer[MSG_MAX_DATA_SIZE];
    msg_size = recv(fd, msg_buffer, sizeof(msg_buffer)+1,0);
    return(this->unpack(msg_buffer));
}

int IGDE_Message::send_msg(int fd)
{
    std::cout << std::endl << "Sending message to fd:" << fd << std::endl;;
    char buffer[1024];
    int msg_len = this->pack(buffer);
    return (int) send(fd, buffer, msg_len, 0);
}
