/*Daniel Raz*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>

//In any case of wrong command usage, print this messege:
#define usegeError "Usage: ./chatserver <port> <max_clients>\n"
#define SIZE_BUFFER 1024

int create_socket(int port);

int main(int argc, char **argv)
{

    if (argc != 3)
    {
        printf("%s", usegeError);
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0)
    {
        printf("%s", usegeError);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]),
        max_clients = atoi(argv[2]),
        array_size = max_clients + 1,
        fdis_counter = 0,
        rc;

    /*create the main socket*/
    int main_socket = create_socket(port);
    if (main_socket < 0)
        exit(EXIT_FAILURE);

    /*Array for active clients */
    int *active_clients = (int *)malloc(array_size * sizeof(int));
    if (active_clients == NULL)
    {
        perror("malloc\n");
        exit(EXIT_FAILURE);
    }
    memset(active_clients, -1, (array_size) * sizeof(int));
    active_clients[0] = main_socket;
    fdis_counter++;

    char buffer[SIZE_BUFFER];
    memset(buffer, 0, SIZE_BUFFER);

    fd_set rfd, wfd, c_rfd;
    FD_ZERO(&rfd);
    FD_ZERO(&wfd);
    FD_SET(main_socket, &rfd);

    while (1)
    {
        c_rfd = rfd;

        select(getdtablesize(), &c_rfd, &wfd, NULL, NULL);
        
        /*write the messege from buffer to all active clients*/
        for (int i = 1; i < array_size; i++)
        {
            if (FD_ISSET(active_clients[i], &wfd))
            {
                printf("fd %d is ready to write\n", active_clients[i]);
                int w = write(active_clients[i], buffer, rc);
                if (w < 0)
                {
                    perror("write\n");
                    free(active_clients);
                    exit(EXIT_FAILURE);
                }
                FD_CLR(active_clients[i], &wfd);
            }
        }

        /*check if clients want to connect and if num of clients small then max_client*/
        if (FD_ISSET(main_socket, &c_rfd) && fdis_counter <= array_size)
        {
            int serving_socket = accept(main_socket, NULL, NULL);
             if (serving_socket < 0)
            {
                perror("accept\n");
                free(active_clients);
                exit(EXIT_FAILURE);
            }
            FD_SET(serving_socket, &rfd);
            fdis_counter++;
            if (fdis_counter == array_size)
                FD_CLR(main_socket, &rfd);

            for (int i = 1; i < array_size; i++)
                if (active_clients[i] == -1)
                {
                    active_clients[i] = serving_socket;
                    break;
                }
        }

        /*check if clients write somthing and read to the buffer*/
        for (int i = 1; i < array_size; i++)
        {
            if (FD_ISSET(active_clients[i], &c_rfd))
            {
                rc = read(active_clients[i], buffer, SIZE_BUFFER);
                if (rc < 0)
                {
                    perror("read\n");
                    free(active_clients);
                    exit(EXIT_FAILURE);
                }
                else if (rc == 0)
                {
                    close(active_clients[i]);
                    FD_CLR(active_clients[i], &rfd);
                    FD_SET(main_socket, &rfd);
                    active_clients[i] = -1;
                    fdis_counter--;
                }
                else //set all the active clients in the write set
                {   
                    printf("fd %d is ready to read\n", active_clients[i]);
                    for (int j = 1; j < array_size; j++)
                    {
                        if (active_clients[j] != -1)
                            FD_SET(active_clients[j], &wfd);
                    }
                }
                break;
            }
        }
    }

    return 0;
}

//this function create and return the main socket, else return -1 if there is any error.
int create_socket(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("error: socket\n");
        return -1;
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("error: bind\n");
        close(sockfd);
        return -1;
    }
    if (listen(sockfd, 5) < 0)
    {
        perror("error: listen\n");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

