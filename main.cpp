#include "tcp.h"
#include "eventloop.h"
#include "http.h"
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char** argv) {
    printf("argc: %d, argv[1]: %s, argv[2]: %s\n", argc, argv[1], argv[2]);
    if (argc != 3 ) {
        printf("USAGE: %s s[elect]/p[oll]/e[poll] port\n", argv[0]);
        exit(1);
    }

    uint32_t port = (uint32_t)atoi(argv[2]);

    // TcpServer::ptr httpServer; 

    switch (argv[1][0]) {
    case 's': {
        printf("select start.\n");
        TcpServer::ptr httpServer(new TcpServer(port, 4, 0));
        printf("ready for run.\n");
        httpServer -> run();
        break;
    }
    case 'p': {
        printf("poll start.\n");
        TcpServer::ptr httpServer(new TcpServer(port, 4, 1));
        printf("ready for run.\n");httpServer -> run();
        break;
    }
    case 'e': {
        printf("epoll start.\n");
        TcpServer::ptr httpServer(new TcpServer(port, 4, 2));
        printf("ready for run.\n");httpServer -> run();
        break;
    }
    default:
        printf("USAGE: %s s[elect]/p[oll]/e[poll] port\n", argv[0]);
        exit(1);
    }

    // delete httpServer;

    return 0;


}