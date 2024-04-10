#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <poll.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pwd.h>

#include "include/talk.h"

/* gcc -g -Wall -L ~pn-cs357/Given/Talk/lib64 mytalk.c -ltalk -lncurses */

#define MIN_PORT 1024
#define MAX_PORT 65535
#define HOST 0
#define CLIENT 1
#define LOCAL 0
#define REMOTE 1
#define DECIMAL 10
#define BUFFER_SIZE 1024
#define ACCEPT "ok"
#define DECLINE "unlucky, they might have been hit by a truck"
#define CLOSE_MSG "Connection closed. ^C to terminate."

/*  note that many functions specify arguments or results are in nbo
    conversions can be done using htonl, htons, ntohl, ntohs */


int server(char* hostname, int verbosity, int port, int a, int windowing) {
    /*  1. Create the socket with socket(2)
        2. Attach an address to it with bind(2)
        3. Wait for a connection with listen(2)
        4. Accept a connection with accept(2)
        5. Send and receive with send(2) and recv(2) until done
        6. close(2) any remaining sockets */
    
    int fd;
    int clientfd;
    int receive_len;
    int rs_len; /* recieving and sending length */
    int decl = 0;
    char buffer[BUFFER_SIZE];
    char clientAddr[INET_ADDRSTRLEN];
    char username[LOGIN_NAME_MAX + 1];
    socklen_t len; /* to be used for accept */

    struct pollfd fds[2];
    /* The  caller  should specify the number of items
        in the fds array in nfds.  
    struct pollfd {
            int   fd;          file descriptor 
            short events;      requested events 
            short revents;     returned events 
        if fd is negative, events is ignored and revents returns 0
        events is an input paramater, a bit mask specifiying events interested
        if events is 0, then revents also returns 0
        revents is an output parameter, filled by kernel with occured events
        negative value in timeout means infinite timeout
    */

    struct sockaddr_in sa;
    struct sockaddr_in clientInfo;

    set_verbosity(verbosity);

    /* 1. creating socket */
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: Unable to socket host");
        exit(EXIT_FAILURE);
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 2. attach address with bind */
    if(bind(fd, (struct sockaddr *)&sa, sizeof(sa))) {
        perror("bind: Unable to bind fd to socket");
        exit(EXIT_FAILURE);
    }

    /* 3. listen for connection */
    if(listen(fd, 1)) {
        perror("listen: Unable to listen for incoming connections");
        exit(EXIT_FAILURE);
    }

    /* 4. accept a connection */
    len = sizeof(clientInfo);
    if((clientfd = accept(fd, (struct sockaddr*) &clientInfo, &len)) == -1) {
        perror("accept: issue with accepting connection");
        exit(EXIT_FAILURE);        
    }

    fds[LOCAL].fd = STDIN_FILENO;
    fds[LOCAL].events = POLLIN;
    fds[LOCAL].revents = 0;
    fds[REMOTE] = fds[LOCAL];
    fds[REMOTE].fd = clientfd;

    /*err = getpeername(clientfd, (struct sockaddr*)&addr, &addr_len);

    if (err!=0) {
        perror("getpeername: failed to fetch remote address");
    }

    err = getnameinfo(((struct sockaddr*)&addr,
            addr_len, clientAddr, sizeof(clientAddr), 0, 0, NI_NUMERICHOST));
    
    if (err!=0) {
        perror("getname info: failed to convert address to string");
    } */

    /* inet_ntop converts IPv4 addresses from binary to text 
        (int af, void *src, char *dst, socklen_t size) */

    inet_ntop(AF_INET,
        &clientInfo.sin_addr, clientAddr, sizeof(clientAddr));

    /* recv returns the length of the message on successful completion */
    if((receive_len = recv(clientfd, username, sizeof(username), 0)) == -1) {
        perror("recv: Couldn't receive from client");
        exit(EXIT_FAILURE);
    }

    username[receive_len] = '\0';

    printf("Mytalk request from %s@%s. Accept (y/n)? ", username, clientAddr);

    /* if -a was given automatically accept */
    if(a) {
        printf("Auto-accepting. (-a flag)\n");
    }

    /* send(sockfd, buf, len, flags) 
        sockfd is file des of sending socket*/
    if(a || tolower(getchar()) == 'y') {
        if(send(clientfd, ACCEPT, strlen(ACCEPT), 0) == -1) {
            perror("send: Unable to send acceptance");
            exit(EXIT_FAILURE);
        }
    } else {
        if(send(clientfd, DECLINE, strlen(DECLINE), 0) == -1) {
            perror("send: Unable to send decline");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
        decl = 1;
        return 0;
    }

    /* for some reason even with exit success or return, program still ran */
    if(decl) {
        exit(EXIT_SUCCESS);
    }

    if (windowing) {
        start_windowing();
    }

    /* 5. Send and receive with send(2) and recv(2) until done */
    while (888) {
        /* poll(struct pollfd * fds, nfds_t nfds, int timeout)*/
        if(poll(fds, sizeof(fds) / sizeof(struct pollfd), -1) == -1) {
            perror("poll: server");
            exit(EXIT_FAILURE);
        }

        /* sending part */
        /* read from input then send, update buffer before that */
        if(fds[LOCAL].revents && POLLIN) {
            update_input_buffer();
            if(has_whole_line()) {
                if((rs_len = read_from_input(buffer, BUFFER_SIZE)) > 0) {
                    if(send(clientfd, buffer, rs_len, 0) == -1) {
                        perror("send: Unable to send from server to client");
                        exit(EXIT_FAILURE);
                    }

                    /* 6. start closing remaining sockets*/
                    if (has_hit_eof()) {
                        stop_windowing();
                        close(clientfd);
                        close(fd);
                        exit(EXIT_SUCCESS);
                    }
                } else if (rs_len == -1) {
                    fprintf(stderr, "Unable read from input\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        /* reading, receiving part */
        if (fds[REMOTE].revents && POLLIN) {
            if((rs_len = recv(clientfd, buffer, BUFFER_SIZE, 0)) == -1) {
                perror("recv: Unable to receive from client");
                exit(EXIT_FAILURE);
            }  

            /* recv returns 0 when peer has performed an orderly shutdown */  
            if (rs_len == 0) {
                write_to_output(CLOSE_MSG, strlen(CLOSE_MSG));

                while (1){
                    if (has_hit_eof()){
                        close(clientfd);
                        close(fd);
                        stop_windowing();
                        exit(EXIT_SUCCESS);
                    }
                }
            }

            write_to_output(buffer, rs_len);        
        }
    }
}


int client(char* hostname, int verbosity, int port, int a, int windowing) {
    /*  1. Look up peer address with getaddrinfor(3)
        2. Create the socket with socket(2) 
        3. Connect to the server using connect(2) 
        4. Send and receive with send(2) and recv(2) until done
        5. close(2) any remaining sockets */

    char buffer[BUFFER_SIZE];
    char* username;

    int sockett;
    int rs_len;

    struct pollfd fds[2];
    struct sockaddr_in sa;  
    struct hostent* hostent;
    /* struct hostent {
               char  *h_name;             official name of host 
               char **h_aliases;          alias list 
               int    h_addrtype;         host address type 
               int    h_length;           length of address 
               char **h_addr_list;        list of addresses 
           } */

    set_verbosity(verbosity);

    /* 2. create socket */
    if((sockett = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: Unable to socket client");
        exit(EXIT_FAILURE);
    }

    /* 1. looking up peer address, using gethostbyname */
    hostent = gethostbyname(hostname);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = *(uint32_t*)hostent->h_addr_list[0];

    fds[LOCAL].fd = STDIN_FILENO;
    fds[LOCAL].events = POLLIN;
    fds[LOCAL].revents = 0;
    fds[REMOTE] = fds[LOCAL];
    fds[REMOTE].fd = sockett;

    /* 3. Connect to server  */
    if(connect(sockett, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("connect: Unable to connect client socket");
        exit(EXIT_FAILURE);
    }

    username = (getpwuid(getuid()))->pw_name;

    /* Send username to the host */
    if(send(sockett, username, strlen(username), 0) == -1) {
        perror("send: Unable to send username");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for response from %s\n", hostname);

    /* Wait for initial packet (hopefully acceptance!) */
    if(poll(&fds[REMOTE], 1, -1) == -1) {
        perror("poll: Unable to poll remote");
        exit(EXIT_FAILURE);
    }

    if(recv(sockett, &buffer, sizeof(ACCEPT), 0) == -1) {
        perror("recv: client");
        exit(EXIT_FAILURE);
    }

    /* check if it "ok" */
    if(strncmp(buffer, ACCEPT, strlen(ACCEPT)) != 0) {
        printf("%s declined connection\n", hostname);
        exit(EXIT_SUCCESS);
    }

    if(windowing) {
        start_windowing();
    }

    while(888) {
        if(poll(fds, sizeof(fds) / sizeof(struct pollfd), -1) == -1) {
            perror("poll: client");
            exit(EXIT_FAILURE);
        }

        if(fds[LOCAL].revents & POLLIN) {
            update_input_buffer();
            if(has_whole_line()) { 
                int rs_len;
                if((rs_len = read_from_input(buffer, BUFFER_SIZE)) > 0) {
                    if(send(sockett, buffer, rs_len, 0) == -1) {
                        perror("send: Unable to send to remote");
                        exit(EXIT_FAILURE);
                    }
                    
                    if(has_hit_eof()) {
                        stop_windowing();
                        close(sockett);
                        exit(EXIT_SUCCESS);
                    }

                }
                else if(rs_len == -1) {
                    fprintf(stderr, "Unable read from input.\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        if(fds[REMOTE].revents & POLLIN) {
            if((rs_len = recv(sockett, buffer, BUFFER_SIZE, 0)) == -1) {
                perror("Couldn't recv from remote");
                exit(EXIT_FAILURE);
            }
            
            /* If at EOF, exit */
            if(rs_len == 0) {
                write_to_output(CLOSE_MSG, strlen(CLOSE_MSG));

                while (1){
                    if (has_hit_eof()){
                        close(sockett);
                        stop_windowing();
                        exit(EXIT_SUCCESS);
                    }
                }
            }
            write_to_output(buffer, rs_len);
        }
    }
}

int main(int argc, char* argv[]) {
    int verbosity = 0;
    int accept = 0;
    int port;
    int mode;
    int windowing = 1;
    char* hostname;
    char* endPtr;
    int opt;
    int remainingArgs;

    while ((opt = getopt(argc, argv, "vaN:")) != -1) {
        switch (opt) {
            /* increases verbosity, more talkative */
            case 'v':
                verbosity = 1;
                break;

            /* tells server to accept all connections without asking */
            case 'a':
                accept = 1;
                break;
            
            /* tells mytalk to not start ncurses windowing */
            case 'N':
                windowing = 0;
                break;
            
            case '?':
                fprintf(stderr,
                "usage: mytalk [ -a ] [ -N ] [ hostname ] port\n");
                exit(EXIT_FAILURE);
            break;
        }  
    }

    remainingArgs = argc - optind;

    /* there are 2 options here, either only the port or port and hostname */
    if (remainingArgs == 2) {
        hostname = argv[argc - 2];
        port = strtol(argv[argc - 1], &endPtr, DECIMAL);
        
        if(*endPtr != '\0') {
            fprintf(stderr, "Invalid port number\n");
            exit(EXIT_FAILURE);
        }

        if (port < MIN_PORT || port > MAX_PORT) {
            fprintf(stderr,
                "Please choose a port between 1024 and 65535 not %d\n", port);
            exit(EXIT_FAILURE);
        }

        if(verbosity) {
            printf("Currently:\n");
            printf("%-8s %-16s = %d \n", "int", "opt_verbose", verbosity);
            printf("%-8s %-16s = %s \n", "talkmode", "opt_mode", "client");
            printf("%-8s %-16s = %d \n", "int", "opt_port", port);
            printf("%-8s %-16s = %s \n", "char", "*opt_host", hostname);
            printf("%-8s %-16s = %d \n", "int", "opt_accept", accept);
            printf("%-8s %-16s = %d \n", "int", "opt_windows", windowing);
        }

        mode = CLIENT;

    } else if (remainingArgs == 1) {
        port = strtol(argv[argc - 1], &endPtr, DECIMAL);
        
        if(*endPtr != '\0') {
            fprintf(stderr, "Invalid port number\n");
            exit(EXIT_FAILURE);
        }

        if (port < MIN_PORT || port > MAX_PORT) {
            fprintf(stderr,
                "Please choose a port between 1024 and 65535 not %d\n", port);
            exit(EXIT_FAILURE);
        }

        if(verbosity) {
            printf("Currently:\n");
            printf("%-8s %-16s = %d \n", "int", "opt_verbose", verbosity);
            printf("%-8s %-16s = %s \n", "talkmode", "opt_mode", "server");
            printf("%-8s %-16s = %d \n", "int", "opt_port", port);
            printf("%-8s %-16s = %s \n", "char", "*opt_host", "(none)");
            printf("%-8s %-16s = %d \n", "int", "opt_accept", accept);
            printf("%-8s %-16s = %d \n", "int", "opt_windows", windowing);
        }

        mode = HOST;

    } else {
        fprintf(stderr,
            "usage: mytalk [ -v ] [ -a ] [ -N ] [ hostname ] port\n");
        exit(EXIT_FAILURE);
    }

    if(mode == HOST) {
        server(hostname, verbosity, port, accept, windowing);
    } else 
        client(hostname, verbosity, port, accept, windowing);

    return 0;
}