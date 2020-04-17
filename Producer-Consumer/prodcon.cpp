#include "main.h"
#include "tands.h"


/************************************************************
*   Submitted by: Neel Kumar                                *
*                                                           *
*   No collaboration. Sources listed at end of code.        * 
*************************************************************/


/*******************************************************************************
*                             GLOBAL DECLARATIONS                              *
*******************************************************************************/
// Start the clock
clock_t begin = clock();

// File pointer for I/O
FILE *my_file;

// Condition variables and lock
pthread_cond_t  empty;
pthread_cond_t  fill;
pthread_mutex_t mutex;

// Struct that holds the commands
struct command{
    char letter;
    int number;
};

struct consumer_struct{
    pthread_t global_consumer_ID;
    int consumer_count;
};

std::map<pthread_t, int> consumer_hash;

// A queue that is used by both the producer and consumer
std::queue<command> command_queue;
int queue_size;

// num_commands is the amount of 'T's that need to be run
// 'done' globally indicates that the producer has finished producing
int num_commands;
bool done = false;


/*******************************************************************************
*                                  STATISTICS                                  *
*******************************************************************************/
int work_count = 0;
int ask_count = 0;
int receive_count = 0;
int complete_count = 0;
int sleep_count = 0;


/*******************************************************************************
*                                  FILE WRITES                                 *
*******************************************************************************/
// PRODUCER WRITES
void work_time_stamp(int i, std::vector<command> my_vector){
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    int id = 0;
    fprintf(my_file, "%.3lf ID=%-15d Q= %3lu  Work       %d\n", time_spent, id, command_queue.empty() ? 0 : command_queue.size(), my_vector[i].number);
    work_count++;
}
void sleep_time_stamp(int i, std::vector<command> my_vector){
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    int id = 0;
    fprintf(my_file, "%.3lf ID=%-15d         Sleep      %d\n", time_spent, id, my_vector[i].number);
    sleep_count++;
}
void end_time_stamp(){
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    int id = 0;
    fprintf(my_file, "%.3lf ID=%-15d         End\n", time_spent, id);
}

// CONSUMER WRITES
void receive_time_stamp(int n, unsigned long int id){
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    fprintf(my_file, "%.3lf ID=%lu Q= %3lu  Receive    %d\n", time_spent, id, command_queue.empty() ? 0 : command_queue.size(), n);
    receive_count++;
}
void ask_time_stamp(unsigned long int id){
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    fprintf(my_file, "%.3lf ID=%lu         Ask\n", time_spent, id);
    ask_count++;
}
void complete_time_stamp(int n, unsigned long int id){
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    fprintf(my_file, "%.3lf ID=%lu Q= %3lu  Complete   %d\n", time_spent, id, command_queue.empty() ? 0 : command_queue.size(), n);
    complete_count++;
}

// SUMMARY
void summary_time_stamp(pthread_t thread_list[], int num_threads){
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    fprintf(my_file, "Summary:\n");
    fprintf(my_file, "    Work             %d\n", work_count);
    fprintf(my_file, "    Ask              %d\n", ask_count);
    fprintf(my_file, "    Receive          %d\n", receive_count);
    fprintf(my_file, "    Complete         %d\n", complete_count);
    fprintf(my_file, "    Sleep            %d\n", sleep_count);

    // Print each individual thread's count
    std::map<pthread_t, int>::iterator summary_iterator;
    for(int i=0 ; i<num_threads ; i++){
        fprintf(my_file, "    Thread %lu   %d\n", thread_list[i], consumer_hash[thread_list[i]]);
    }

    fprintf(my_file, "Transactions per second: %f\n", complete_count/time_spent);
}

/*******************************************************************************
*                                    PRODUCER                                  *
*******************************************************************************/
void* producer(void *arg){

    // The 'input' for the producer.
    // The producer will take these commands and add them to the shared queue
    std::vector<command> *the_arg = (std::vector<command> *) arg;
    std::vector<command> my_vector = *the_arg;

    // Iterate through the given commands
    // If 'T', add to queue. If 'S', the producer will sleep
    // If the queue is full, the producer will wait
    for(std::size_t i=0; i<my_vector.size(); i++){

        pthread_mutex_lock(&mutex);

            // Start Critical section
            while(command_queue.size() == queue_size){
                pthread_cond_wait(&empty, &mutex);
            }    
            if (my_vector[i].letter == 'S'){
                sleep_time_stamp(i, my_vector);
                Sleep(my_vector[i].number);
            }
            else{
                work_time_stamp(i, my_vector);
                command_queue.push(my_vector[i]);
            }
            // End Critical Section

        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);

    }

    // When the inputs have been parsed, wait for the queue to empty and commands to complete
    end_time_stamp();
    while(!command_queue.empty() || num_commands>complete_count){
        done=false;
    }
    if(command_queue.empty() && num_commands==complete_count){
        done=true;
    }

    pthread_exit(NULL);
}


/*******************************************************************************
*                                    CONSUMER                                  *
*******************************************************************************/
void* consumer(void *arg){

    // ID of the consumer
    pthread_t *the_arg = (pthread_t *) arg;
    pthread_t consumer_ID = *the_arg;

    // Consumers keep 'asking' until all jobs are completed
    while( num_commands>complete_count ){        
        
        pthread_mutex_lock(&mutex);

            // Start Critical Section
            ask_time_stamp(consumer_ID);
            command temp;

            while(command_queue.size()==0){
                pthread_cond_wait(&fill, &mutex);
            }
            
            if(command_queue.size() > 0){
                temp = command_queue.front();
                command_queue.pop();
                receive_time_stamp(temp.number, consumer_ID);
            }
            // End Critical Section

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);

        Trans(temp.number);
        complete_time_stamp(temp.number, consumer_ID);
        consumer_hash[consumer_ID]++;

        // Useful for checking completed commands
        // printf("%c%d\n", temp.letter, temp.number);

        if(done==true && command_queue.empty()){
            break;
        }

    }
    
    pthread_exit(NULL);
}


/*******************************************************************************
*                                      MAIN                                    *
*******************************************************************************/
int main(int num_args, char** arg_strings){

    // Assert proper command line args
    assert(num_args==3 || num_args==2);     

    // Determine what filename for output based on command line args
    if(num_args==2){
        my_file = fopen("prodcon.log", "w");
    }
    else{
        int size = 12 + strlen(arg_strings[2]);
        char file_name[100];
        snprintf(file_name, sizeof(file_name), "prodcon.%s.log", arg_strings[2]);
        my_file = fopen(file_name, "w");
    }         

    // Declare the number of threads and size of queue
    int num_threads = atoi(arg_strings[1]);
    queue_size = num_threads*2;
    
    // Display welcome message
    printf("\n★★★ WELCOME TO 'prodcon'! ★★★\n\n"); 
    std::cout << "Please enter the thread commands with EACH COMMAND ON A NEW LINE!!!\n(Type 'run' when finished):\n" << std::endl;
    
    // Parse through stdin and put elements into a vector
    std::string line;
    std::string from_stdin;
    std::vector<command> command_vector;
    while(getline(std::cin, line)){

        if(line == "run"){
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

    // Check how many commands need to be run
    for(std::vector<int>::size_type i=0; i!=command_vector.size(); i++){
        if(command_vector[i].letter=='T'){
            num_commands++;
        }
    }

    
    // Initialize the thread ids for the multiple consumers, and single producer
    pthread_t thread_id[num_threads];
    pthread_t producer_id;
    std::vector<consumer_struct> consumer_vector;
    
    // Launch the producer thread
    pthread_create(&producer_id, NULL, producer, &command_vector);

    for(int i=0 ; i<num_threads ; i++){
        //consumer_hash[thread_id[i]] = 0;
    }

    // Launch the consumer threads
    for(int i=0 ; i<num_threads ; i++){
        pthread_create(&thread_id[i], NULL, consumer, &thread_id[i]);
    }
    
    // Wait for the producer to complete
    pthread_join(producer_id, NULL);

    summary_time_stamp(thread_id, num_threads);

    return 0;

}


/*

Along with the man pages and lecture notes, the following sources were consulted:

TEXTBOOK: Operating Systems: Three Easy Pieces - Remzi and Andrea Arpaci-Dusseau - Chapters 28, 30 and 31
https://www.youtube.com/watch?v=ynCc-v0K-do pthreads #1: Introduction
https://www.youtube.com/watch?v=iMD1Z3f9ioI Producer Consumer Problem | Process Synchronization Problem in Operating System
https://www.youtube.com/watch?v=9lAuS6jsDgE Mutex Lock
https://www.youtube.com/watch?v=GXXE42bkqQk Mutex Synchronization in Linux with Pthreads
https://www.youtube.com/watch?v=eQOaaDA92SI Pthread Condition Variables
https://stackoverflow.com/questions/7663709/how-can-i-convert-a-stdstring-to-int
https://stackoverflow.com/questions/19754593/declaring-queue-in-c
https://stackoverflow.com/questions/7968631/how-can-i-check-if-a-queue-is-empty
https://www.programiz.com/c-programming/c-file-input-output
https://www.rapidtables.com/code/linux/gcc/gcc-o.html

*/