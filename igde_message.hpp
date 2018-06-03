//
//  igde_message.hpp
//  igde
//
//  Created by Jason Graalum on 5/16/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#ifndef igde_message_hpp
#define igde_message_hpp

#include <iostream>


#define MSG_TYPE_SIZE 2
#define MSG_CMD_SIZE 4
#define MSG_ID_SIZE 10
#define MSG_TBD_SIZE 8
#define MSG_MAX_DATA_SIZE 2048

#define MSG_TYPE_SERVER 1
#define MSG_TYPE_OWNER 2
#define MSG_TYPE_EDITOR 3

#define TYPE_T uint32_t
#define CMD_T uint32_t
#define ID_T uint32_t
#define TBD_T uint32_t

class IGDE_Message
{
private:
    TYPE_T type;
    CMD_T cmd;
    ID_T id;
    TBD_T len;
    char * data;
    
public:
    IGDE_Message();
    IGDE_Message(TYPE_T, CMD_T, ID_T, TBD_T, char *);
    IGDE_Message(TYPE_T, CMD_T, ID_T, TBD_T, std::string );

    int get_msg(int);
    int send_msg(int);

    int pack(char *);
    int unpack(char *);
    
    void print_message();
    
    char * get_data();
    TYPE_T get_type();
    CMD_T get_cmd();
    ID_T get_id();
    TBD_T get_len();

    static ID_T msg_index;
    
    
};
#endif /* igde_Message_hpp */
