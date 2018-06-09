// Chunk Sizes
#define DOWNLOAD_CHUNK_SIZE 500
#define UPLOAD_CHUNK_SIZE 500

// Message Definitions

#define SERVER_TYPE  1001
#define OWNER_TYPE   1002
#define EDITOR_TYPE  1003
#define CONTROL_TYPE 1004
#define UNKNOWN_TYPE 1005


// Server States
// New Client State in the Server - Before Type is Identified
#define S_NEW_CONN      300
#define S_AUTH          301
#define S_SEND_TYPE     302

// Editor State in the Master Server
#define E_NEW_EDITOR     400
#define E_EDITOR_CLOSED  401
#define E_EDITOR_WAITING 402

// Owner State in the Master Server
#define O_ACCEPTING      400
#define O_CONNECTED      401
#define O_DISCONNECTED   402
#define O_DOWNLOADING    403
#define O_ENDING         404
#define O_FREE           405
#define O_FULL           406
#define O_NEW_OWNER      407
#define O_OPEN           408
#define O_SESSION_READY  409
#define O_UPLOADING      410

// Owner to Server Messages
#define OS_OK                   1000
#define OS_BYE                  1001
#define OS_SET_OWNER_TYPE       1002
#define OS_OPEN_SESSION         1003
#define OS_REG_SESSION          1004
#define OS_END_SESSION          1005
#define OS_CLOSE_SESSION        1006


// Server to Owner Messages
#define SO_BYE                  2001
#define SO_OK                   2002
#define SO_OWNER_TYPE_SET       2003
#define SO_SESSIONS_NOT_OPENED  2004
#define SO_SESSION_CLOSED       2005
#define SO_SESSION_ENDED        2006
#define SO_SESSION_NOT_ENDED    2007
#define SO_SESSION_NOT_REGD     2008
#define SO_SESSION_OPENED       2009
#define SO_SESSION_REGD         2010

// Owner to Controller Messages
#define OC_CLOSE_SESSION        3000
#define OC_DOWNLOAD_CHUNK       3001
#define OC_DOWNLOAD_LAST_CHUNK  3002
#define OC_DOWNLOAD_STARTING    3003
#define OC_EDITOR_LIST          3004
#define OC_ENDING_SESSION       3005
#define OC_SESSION_ENDED        3006
#define OC_SESSION_NOT_ENDED    3007
#define OC_SESSION_NOT_STARTED  3008
#define OC_SESSION_STARTED      3009
#define OC_SRV_CONNECTED        3010
#define OC_SRV_DISCONNECTED     3011
#define OC_SRV_NOT_CONNECTED    3012
#define OC_SRV_NOT_DISCONNECTED 3013
#define OC_UPLOAD_CHUNK_ERR     3014
#define OC_UPLOAD_CHUNK_OK      3015
#define OC_UPLOAD_DONE          3016
#define OC_UPLOAD_FILE          3017
#define OC_UPLOAD_GO            3018


// Controller to Owner Messages
#define CO_CMD_FAILED           4001
#define CO_CONNECT_SRV          4002
#define CO_DISCONNECT_SRV       4003
#define CO_DOWNLOAD_CHUNK_ERR   4004
#define CO_DOWNLOAD_CHUNK_OK    4005
#define CO_DOWNLOAD_DONE        4006
#define CO_DOWNLOAD_FILE        4007
#define CO_DOWNLOAD_GO          4008
#define CO_END_SESSION          4009
#define CO_LIST_EDITORS         4010
#define CO_NOT_ACTIVE           4011
#define CO_NOT_CONNECTED        4012
#define CO_OPEN_SESSION         4013
#define CO_SET_CONTROL_TYPE     4014
#define CO_START_SESSION        4015
#define CO_UPLOAD_CHUNK         4016
#define CO_UPLOAD_LAST_CHUNK    4017
#define CO_UPLOAD_STARTING      4018

// Owner to Editor Messages
#define OE_OK                   5000
#define OE_HELLO                5001
#define OE_BYE                  5002
#define OE_INVITE               5003
#define OE_DOC_BLOCK            5004
#define OE_REFRESH              5005

// Editor to Owner Messages
#define EO_OK                   6001
#define EO_HELLO                6002
#define EO_BYE                  6003
#define EO_JOIN                 6004
#define EO_CMD                  6006
#define EO_RELOAD               6007

// Editor to Server Messages
#define ES_OK                   7001
#define ES_GET_SESSIONS         7002
#define ES_JOIN_SESSION         7003
#define ES_LEAVE_SESSION        7004
#define ES_SET_EDITOR_TYPE      7005

// Editor to Server Messages
#define SE_EDITOR_TYPE_SET      8001
#define SE_OK                   8002
#define SE_SESSION_JOINED       8003
#define SE_SESSION_LEFT         8004
#define SE_SESSION_LIST         8005
#define SE_SESSION_NOT_JOINED   8006

