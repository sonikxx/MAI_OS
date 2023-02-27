#include "zmq_tools.h"

void create_addr(char* addr, int id, addr_pattern pattern) {   // адрес + id
    char str[MAX_LEN];
    memset(str, 0, MAX_LEN);
    sprintf(str, "%d", id);              
    memset(addr, 0, MAX_LEN);
    switch (pattern) {
        case tcp_serv:
            memcpy(addr, SERVER_SOCKET_PATTERN, sizeof(SERVER_SOCKET_PATTERN));
            break;
        case tcp_node:
            memcpy(addr, TCP_SOCKET_PATTERN, sizeof(TCP_SOCKET_PATTERN));
            break;
        case inproc:
            memcpy(addr, PING_SOCKET_PATTERN, sizeof(PING_SOCKET_PATTERN));
            break;
        default:
            fprintf(stderr, "[%d] ", getpid());
            perror("ERROR create_addr wrong argument: addr pattern");
            exit(1);
    }
    strcat(addr, str);  
}

void clear_token(message* msg) {
    msg->cmd = delete;
    msg->dest_id = 0;
    msg->value = 0;
    memset(msg->str, 0, MAX_LEN);
}

void* create_zmq_context() {
    void* context = zmq_ctx_new();
    if (context == NULL) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_ctx_new ");
        exit(ERR_ZMQ_CTX);
    }
    return context;
}

void bind_zmq_socket(void* socket, char* endpoint) {
    if (zmq_bind(socket, endpoint) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_bind ");
        exit(ERR_ZMQ_BIND);
    }
}

void disconnect_zmq_socket(void* socket, char* endpoint) {
    if (zmq_disconnect(socket, endpoint) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_disconnect ");
        exit(ERR_ZMQ_DISCONNECT);
    }
}

void connect_zmq_socket(void* socket, char* endpoint) {
    if (zmq_connect(socket, endpoint) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_connect ");
        exit(ERR_ZMQ_CONNECT);
    }
}

void* create_zmq_socket(void* context, const int type) {
    void* socket = zmq_socket(context, type);               // type REQ/REP
    if (socket == NULL) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_socket ");
        exit(ERR_ZMQ_SOCKET);
    }
    return socket;
}

void reconnect_zmq_socket(void* socket, int to, char* addr) {
    if (addr[16] != '\0') {                              // можно переподключаться, если уже только подключался 
        disconnect_zmq_socket(socket, addr);    
    }
    create_addr(addr, to, tcp_serv);   
    connect_zmq_socket(socket, addr);
}

void close_zmq_socket(void* socket) {
    if (zmq_close(socket) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_close ");
        exit(ERR_ZMQ_CLOSE);
    }
}

void destroy_zmq_context(void* context) {
    if (zmq_ctx_destroy(context) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_ctx_destroy ");
        exit(ERR_ZMQ_CLOSE);
    }
}

void receive_msg_wait(void* socket, message* token) {
    zmq_msg_t reply;
    zmq_msg_init(&reply);
    if (zmq_msg_recv(&reply, socket, 0) == -1) {      // блокирующий вызов
        zmq_msg_close(&reply);
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_recv ");
        exit(ERR_ZMQ_MSG);
    }
    (*token) = * ((message*) zmq_msg_data(&reply));   
    if (zmq_msg_close(&reply) == -1) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_close ");
        exit(ERR_ZMQ_MSG);
    }
}

void send_msg_wait(void* socket, message* token) {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if (zmq_msg_init_size(&msg, sizeof(message)) == -1) {   
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_init ");
        exit(ERR_ZMQ_MSG);
    }
    if (zmq_msg_init_data(&msg, token, sizeof(message), NULL, NULL) == -1) {  
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_init ");
        exit(ERR_ZMQ_MSG);
    }
    if (zmq_msg_send(&msg, socket, 0) == -1) {
        zmq_msg_close(&msg);
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_send ");
        exit(ERR_ZMQ_MSG);
    }
}

bool ping_process(int id) {            
    char addr_monitor[MAX_LEN];
    char addr_connection[MAX_LEN];
    create_addr(addr_connection, id, tcp_serv);                 //адрес для подключения, которое проверяем 
    create_addr(addr_monitor, id, inproc);                      //адрес для отслеживания

    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);

     
    zmq_socket_monitor(requester, addr_monitor, ZMQ_EVENT_CONNECTED | ZMQ_EVENT_CONNECT_RETRIED);   //отслеживание событий
    void* socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, addr_monitor);
    zmq_connect(requester, addr_connection);

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_msg_recv(&msg, socket, 0);
    uint8_t* data = (uint8_t*)zmq_msg_data(&msg);                              //берем первый адрес, во втором инфа об этом событии
    uint16_t event = *(uint16_t*)(data);                                       //получаем число 

    zmq_close(requester);
    zmq_close(socket);
    zmq_msg_close(&msg);
    zmq_ctx_destroy(context);



    if (event == ZMQ_EVENT_CONNECT_RETRIED) {
        return false;
    } else {
        return true;
    }
}



