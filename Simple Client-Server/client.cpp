#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <limits.h>
#include <sys/timeb.h>
#include <time.h>

#include "tands.c"

/************************************************************
*   Submitted by: Neel Kumar                                *
*                                                           *
*   No collaboration. Sources listed at end of code.        * 
*************************************************************/

/*******************************************************************************
*                             GLOBAL DECLARATIONS                              *
*******************************************************************************/
struct command{
    char letter;
    int number;
};

FILE *my_file;

// A function that returns the epoch time to two decimal places
double get_epoch(){
    struct timespec my_time;
    clock_gettime(CLOCK_REALTIME, &my_time);
    return ((double) my_time.tv_sec + (double) my_time.tv_nsec/1000000000);
}


/*******************************************************************************
*                                  FILE WRITES                                 *
*******************************************************************************/
void initial_write(int port, char* address, char* machine, int pid){
    fprintf(my_file, "Using port %d\nUsing server address %s\nHost %s.%d\n", port, address, machine, pid);
}

void sleep_write(int n){
    fprintf(my_file, "Sleep %d units\n", n);
}

void send_write(double time, int t_n){
    fprintf(my_file, "%.2lf: Send (T%3d)\n", time, t_n);
}

void recv_write(double time, int job_no){
    fprintf(my_file, "%.2lf: Recv (D%3d)\n", time, job_no);
}

void final_write(int transactions){
    fprintf(my_file, "Sent %d transactions\n", transactions);
}


/*******************************************************************************
*                                   MAIN                                       *
*******************************************************************************/
int main(int argc, char *argv[])
{

    /*******************************************************************
    *                         PREPARE LOG FILE                         *
    *******************************************************************/
    // Check for correct number of command line args
    if(argc != 3) {printf("\nPls enter port number and IP address\n"); return 1;}

    // Get this machine's host name and this process's PID
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    int the_pid = (int) getpid();

    // Determine what filename for output based on command line args
    char file_name[100];
    snprintf(file_name, sizeof(file_name), "%s.%d.log", hostname, the_pid);
    my_file = fopen(file_name, "w");

    // Write the preliminary information to the log file
    initial_write(atoi(argv[1]), argv[2], hostname, the_pid);


    /*******************************************************************
    *                         PARSE USER INPUT                         *
    *******************************************************************/
    // Display welcome message
    std::cout << "\n★★★ Hi there: " << hostname << "." << (int)getpid() << " ★★★" << std::endl;
    std::cout << "Please enter the T<n> and/or S<n> commands, with EACH COMMAND ON A NEW LINE!\n(Type 'done' when finished):\n" << std::endl;
    
    // Parse through the input and insert the elements into a vector
    std::string line;
    std::string from_stdin;
    std::vector<command> command_vector;
    while(getline(std::cin, line)){
        if(line == "done"){
            break;
        }
        else{                
            command c;
            c.letter = (char) line.at(0);
            c.number = atoi((line.erase(0,1)).c_str());
            command_vector.push_back(c);
        }
    }
    std::cout << "Running. Pls wait..." << std::endl;


    /*******************************************************************
    *                       THE NETWORKING PART                        *
    *******************************************************************/
    // int to keep track of how man T<n> commands are sent to server
    int total_transactions = 0;

    // Loop through the user input
    for(int i=0 ; i<command_vector.size() ; i++)
    {   
        // If there is a 'S' command, the client sleeps
        if(command_vector[i].letter == 'S'){
            sleep_write(command_vector[i].number);
            Sleep(command_vector[i].number);
        }
        // If there is a 'T' command, establish a connection with the server
        else if(command_vector[i].letter == 'T'){

            // Create the socket, store the result in 'sockfd'
            // First parameter is the domain, second is type of socket I think TCP in this case?, last is protocol 0=default
            int sockfd = 0;
            if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {printf("\n Error : Could not create socket \n");} 

            // Specify an address for the socket by declaring the structure
            // AF_INET (same socket family, what type of address we are workign with)
            // The address is specified in our command line arg
            // The port is our specified command line arg
            struct sockaddr_in serv_addr; 
            memset(&serv_addr, '0', sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(atoi(argv[1])); 
            if(inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0) {printf("\n inet_pton error occured\n");}
            
            // Make the connection to the specified port and address
            if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
                printf("Connection failed. Please verify IP and port and try again.\n");
            }

            // Send the Machine Name
            char machine_name[100];
            snprintf(machine_name, sizeof(machine_name), "%s.%d", hostname, the_pid);
            send(sockfd, machine_name, sizeof(machine_name), 0);

            // Receive the ack from server 
            char response[100];
            recv(sockfd, &response, sizeof(response), 0);

            // Send the Trans(n) integer
            uint32_t my_int = htonl(command_vector[i].number);
            send(sockfd, &my_int, sizeof(uint32_t), 0);
            send_write(get_epoch(), command_vector[i].number);

            // Receive the ack number from server
            uint32_t recBuff = 0;
            recv(sockfd, &recBuff, sizeof(uint32_t), 0);
            int received_num;
            received_num = ntohl(recBuff);
            recv_write(get_epoch(), received_num);

            // Increment transaction number for this client
            total_transactions++;
        }
    }

    final_write(total_transactions);
    std::cout << "Machine: " << hostname << "." << (int)getpid() << " has completed!" << std::endl;    
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
