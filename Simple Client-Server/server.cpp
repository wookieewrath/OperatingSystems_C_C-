#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <map>
#include <iostream>
#include <string>

#include "tands.c"

/************************************************************
*   Submitted by: Neel Kumar                                *
*                                                           *
*   No collaboration. Sources listed at end of code.        * 
*************************************************************/

/*******************************************************************************
*                             GLOBAL DECLARATIONS                              *
*******************************************************************************/
// The file for output
FILE *my_file;

// A map to keep track of how many requests were processed for a client
std::map<std::string, int> client_map;

// A function that returns the epoch time to two decimal places
double get_epoch(){
    struct timespec my_time;
    clock_gettime(CLOCK_REALTIME, &my_time);
    return ((double) my_time.tv_sec + (double) my_time.tv_nsec/1000000000);
}


/*******************************************************************************
*                                  FILE WRITES                                 *
*******************************************************************************/
void receive_write(double time, int job_no, int t_n, char* machine){
    fprintf(stdout, "%.2lf: #%3d (T%3d) from %s\n", time, job_no, t_n, machine);
    fprintf(my_file, "%.2lf: #%3d (T%3d) from %s\n", time, job_no, t_n, machine);
}

void completed_write(double time, int job_no, char* machine){
    fprintf(stdout, "%.2lf: #%3d (Done) from %s\n", time, job_no, machine);
    fprintf(my_file, "%.2lf: #%3d (Done) from %s\n", time, job_no, machine);
}

void summary_write(int total, double elapsed_time){
    fprintf(stdout, "\nSUMMARY\n");
    fprintf(my_file, "\nSUMMARY\n");
    std::map<std::string, int>::iterator it;
    for ( it=client_map.begin() ; it!=client_map.end() ; it++ )
    {
        fprintf(stdout, "  %d transactions from %s\n", it->second, (it->first).c_str());
        fprintf(my_file, "  %d transactions from %s\n", it->second, (it->first).c_str());
    }
    fprintf(stdout, "%.2f transactions/sec (%d/%.2f)\n", (double) total/elapsed_time, total, elapsed_time);
    fprintf(stdout, "(Timeout period not included in elapsed time)\n");
    fprintf(my_file, "%.2f transactions/sec (%d/%.2f)\n", (double) total/elapsed_time, total, elapsed_time);
    fprintf(my_file, "(Timeout period not included in elapsed time)\n");
}


/*******************************************************************************
*                                   MAIN                                       *
*******************************************************************************/
int main(int argc, char *argv[])
{

    /*******************************************************************
    *                         PREPARE LOG FILE                         *
    *******************************************************************/
    // Ensure two command line arguments
    if(argc != 2) {printf("\nPlease enter port number \n"); return 1;}

    // Open the log file
    char file_name[100];
    snprintf(file_name, sizeof(file_name), "Server.log");
    my_file = fopen(file_name, "w");
    fprintf(my_file, "Using port %s\n", argv[1]);
    fprintf(stdout, "Using port %s\n", argv[1]); // Also to stdout
    fclose(my_file);

    /*******************************************************************
    *                       THE NETWORKING PART                        *
    *******************************************************************/
    // Create the socket, store the result in 'listenfd'
    // First parameter is the domain, second is type of socket I think TCP in this case?, last is protocol 0=default
    int listenfd = 0;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // Specify an address for the socket by declaring the structure
    // AF_INET (same socket family, what type of address we are workign with)
    // The address is just our local machine, should just be 0.0.0.0
    // The port is our specified command line arg
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Bind the address and socket (where it is going to listen for connections)
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    // Data structures used for sending message
    int connfd = 0;

    // Listen for upto a hundred connections
    listen(listenfd, 100); 

    // Initialize necesary parameters for statistics etc.
    int total = 0;
    int time_out = 60;
    double elapsed_time = 0;

    while(1)
    {
        // Open the log file for writing
        my_file = fopen(file_name, "a");

        // Set the timeout period for our socket
        struct timeval timeout;      
        timeout.tv_sec = time_out;
        timeout.tv_usec = 0;
        setsockopt (listenfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

        // Accept a connection
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if(connfd == -1){
            break;
        }

        // Start the server 'processing' timer (not including wait time)
        double start_work = get_epoch();

        // Receive the machine name 
        char machine_name[100];
        recv(connfd, &machine_name, sizeof(machine_name), 0);
        
        // Send ack for machine name
        char ack[100] = "Machine name received";
        send(connfd, ack, sizeof(ack), 0);
        
        // Receive the integer for the Trans command
        uint32_t recBuff = 0;
        recv(connfd, &recBuff, sizeof(uint32_t), 0);
        int received_num;
        received_num = ntohl(recBuff);

        // Do the appropriate file writes and Trans() command
        total++;
        receive_write(get_epoch(), total, received_num, machine_name);
        Trans(received_num);
        completed_write(get_epoch(), total, machine_name);

        // Send an ack confirming that Trans(n) has been completed
        uint32_t response_int = htonl(total);
        send(connfd, &response_int, sizeof(uint32_t), 0);

        // Close the connection
        close(connfd);

        // End the 'processing' timer
        elapsed_time = elapsed_time + get_epoch() - start_work;

        // Increment the number of jobs completed for a specific machine in the map
        std::string str(machine_name);
        client_map[str]++;

        // Close the log file
        fclose(my_file);

    }

    // Calculate the elapsed time and do the final log write
    summary_write(total, elapsed_time);

    // Indicate that the server has timed out and exit
    printf("Sever has timed out!\n");
    return 0;

}

/*

Along with the man pages and lecture notes, the following sources were consulted:

TEXTBOOK: Operating Systems: Three Easy Pieces - Remzi and Andrea Arpaci-Dusseau
ORIGINAL SOURCE: www.thegeekstuff.com/2011/12/c-socket-programming

https://www.epochconverter.com/
https://www.youtube.com/watch?v=LtXEMwSG5-8 Socket Programming Tutorial In C For Beginners | Part 1 | Eduonix
https://stackoverflow.com/questions/1790750/what-is-the-difference-between-read-and-recv-and-between-send-and-write
https://stackoverflow.com/questions/409348/iteration-over-stdvector-unsigned-vs-signed-index-variable
https://stackoverflow.com/questions/19794764/linux-socket-how-to-make-send-wait-for-recv
https://stackoverflow.com/questions/11147633/send-and-receive-an-integer-value-over-tcp-in-c
https://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c
https://stackoverflow.com/questions/17131863/passing-string-to-a-function-in-c-with-or-without-pointers
https://stackoverflow.com/questions/11765301/how-do-i-get-the-unix-timestamp-in-c-as-an-int
https://stackoverflow.com/questions/19429138/append-to-the-end-of-a-file-in-c
https://stackoverflow.com/questions/26281979/c-loop-through-map
https://stackoverflow.com/questions/4181784/how-to-set-socket-timeout-in-c-when-making-multiple-connections

*/
