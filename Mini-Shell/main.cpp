
/************************************************************
*   Submitted by: Neel Kumar                                *
*                                                           *
*   No collaboration. Sources listed at end of code.        * 
*************************************************************/

#include "main.h"

#define LINE_LENGTH 100     // Max # of characters in an input line
#define MAX_ARGS 7          // Max number of arguments to a command
#define MAX_LENGTH 20       // Max # of characters in an argument
#define MAX_PT_ENTRIES 32   // Max entries in the Process Table

int main(int argc, char** argv){

    // Initialize times
    struct tms time2;
    times(&time2);

    // Display welcome message
    printf("\n★★★ WELCOME TO SHELL379! ★★★\n\n");

    // Vector to hold PIDs
    std::vector<int> id_vector;

    // Vector to hold supsended things
    std::vector<int> suspend_vector;

    // Map to hold PIDs with its associated command
    std::map<int, std::string> pid_command_dict;
 
    while(true){
        
        /*******************************************************************************
        *                                INITIALIZE LOOP                               *
        *******************************************************************************/
        int status;
        waitpid(-1, &status, WNOHANG);       

        // Initialize a string to hold the user input
        // Initialize a vector that will hold the broken-up input
        std::string user_input;
        std::vector<std::string> my_vector;
        std::vector<std::string> output_vector;
        std::vector<int> file_vector;
        my_vector.clear();

        // Prompt shell name and wait for user input, store it in user_input
        std::cout << "SHELL379: ";
        getline(std::cin, user_input);

        // Split string based on spaces
        std::istringstream my_input_string_stream(user_input);
        std::string item;   
        while (std::getline(my_input_string_stream, item, ' ')) {
            my_vector.push_back(item);
        }        


        /*******************************************************************************
        *                                CHECK FOR INPUT                               *
        *******************************************************************************/
        if (my_vector.size() <= 0)
        {
            printf("No input\n");
        }


        /*******************************************************************************
        *                                CHECK NUM ARGS                                *
        *******************************************************************************/
        else if (my_vector.size() > MAX_ARGS) 
        {
            printf("\nToo many arguments");
        }      
        

        /*******************************************************************************
        *                                     EXIT                                     *
        *******************************************************************************/
        else if (std::find(my_vector.begin(), my_vector.end(), "exit") != my_vector.end())
        {   
            printf("\nWaiting for all jobs to complete...\n");
            for(int i=0 ; i<id_vector.size() ; i++){
                waitpid((pid_t) (id_vector[i]), &status, 0);
            }

            times(&time2);
            long double clock_ticks_per_second = sysconf(_SC_CLK_TCK);
            double user_time = (time2.tms_cutime)/(double)clock_ticks_per_second;
            double system_time = (time2.tms_cstime)/(double)clock_ticks_per_second;                       
            printf("Resources used:\n");
            printf("User Time =         %.2lf seconds\n", user_time);
            printf("Sys  Time =         %.2lf seconds\n", system_time);
            
            printf("\nGoodbye dear friend :) !\n\n");
            return 0;
        }


        /*******************************************************************************
        *                                     KILL                                     *
        *******************************************************************************/
        else if (std::find(my_vector.begin(), my_vector.end(), "kill") != my_vector.end())
        {
            if(my_vector.size() < 2){
                printf("Enter the PID to kill.");
            }
            else{
                // Send Signal 9, "SIGKILL"
                int to_kill = atoi(my_vector[1].c_str());
                if(kill(atoi(my_vector[1].c_str()), 9) == 0){
                    if(std::find(id_vector.begin(), id_vector.end(), to_kill) != id_vector.end()){
                        id_vector.erase(std::remove(id_vector.begin(), id_vector.end(), to_kill), id_vector.end());
                    }
                    printf("SUCCESS: Process has been killed or is a Zombie");
                }
                else
                {
                    printf("FAILURE: Process was not killed");
                }
                
            }
        }


        /*******************************************************************************
        *                                    SUSPEND                                   *
        *******************************************************************************/
        else if (std::find(my_vector.begin(), my_vector.end(), "suspend") != my_vector.end())
        {
            if(my_vector.size() < 2){
                printf("Enter the PID to kill.");
            }
            else{
                if(kill(atoi(my_vector[1].c_str()), 19) == 0){
                    suspend_vector.push_back(atoi(my_vector[1].c_str()));
                    printf("SUCCESS: Process was suspended");
                }
                else
                {
                    printf("FAILURE: Process was not suspended");
                }
                
            }
        }  


        /*******************************************************************************
        *                                    RESUME                                    *
        *******************************************************************************/
        else if (std::find(my_vector.begin(), my_vector.end(), "resume") != my_vector.end())
        {
            if(my_vector.size() < 2){
                printf("Enter the PID to kill.");
            }
            else{
                int to_resume = atoi(my_vector[1].c_str());
                if(kill(atoi(my_vector[1].c_str()), 18) == 0){
                    if(std::find(suspend_vector.begin(), suspend_vector.end(), to_resume) != suspend_vector.end()){
                        suspend_vector.erase(std::remove(suspend_vector.begin(), suspend_vector.end(), to_resume), suspend_vector.end());
                    }
                                       
                    printf("SUCCESS: Process was resumed");
                }
                else
                {
                    printf("FAILURE: Process was not resumed");
                }
                
            }
        } 


        /*******************************************************************************
        *                                     SLEEP                                     *
        *******************************************************************************/
        else if (std::find(my_vector.begin(), my_vector.end(), "sleep") != my_vector.end())
        {
            sleep(atoi(my_vector[1].c_str()));
        }


        /*******************************************************************************
        *                                     WAIT                                     *
        *******************************************************************************/
        else if (std::find(my_vector.begin(), my_vector.end(), "wait") != my_vector.end())
        {
            int status;
            waitpid((pid_t) (atoi(my_vector[1].c_str())), &status, 0);
        }


        /*******************************************************************************
        *                                     JOBS                                     *
        *******************************************************************************/         
        else if (std::find(my_vector.begin(), my_vector.end(), "jobs") != my_vector.end())
        {
                       
            // Somehow cleans zombies...???
            int status;
            waitpid(-1, &status, WNOHANG);            

            // Prepare and execute a popen for the ps command
            FILE *p;
            char ch;
            p = popen("ps", "r");
            char buf[500];
            
            int total = 0; 

            std::cout << std::endl;

            while (fscanf(p,"%s",buf)==1)
            {
                // Can be used to print the whole thing
                //printf("%s\n", buf);

                for (int i=0 ; i<id_vector.size() ; i++)
                {
                    // All this just to turn an element in my ID vector to a string...
                    int foo = id_vector[i];
                    std::string out_string;
                    std::stringstream ss;
                    ss << foo;
                    out_string = ss.str();
                    char const* my_int_char = out_string.c_str();                    
                    
                    // Check for IDs
                    if(strcmp(buf, my_int_char)==0)
                    {
                        FILE *time_file;
                        char check_time[50];
                        strcpy(check_time, "ps -o etimes= -p \"");
                        char int_str[12];
                        sprintf(int_str, "%d", id_vector[i]);
                        strcat(check_time, int_str);
                        strcat(check_time, "\" | tr -s \" \"");
                        time_file = popen(check_time, "r");
                        char process_time[5];
                        fgets(process_time, 40, time_file);
                        strtok(process_time, "\n");

                        char running = 'R';
                        if(std::find(suspend_vector.begin(), suspend_vector.end(), foo) != suspend_vector.end()){
                            running='S';
                        }

                        if(total==0){
                            printf("#     PID S SEC   COMMAND\n");
                        }
                        printf("%d: %6s %c %-3.3s   %s\n", total, buf, running, process_time+1, pid_command_dict[id_vector[i]].c_str());
                        total ++;
                    }
                    
                }
            }

            // Print times of completed processes
            times(&time2);
            long double clock_ticks_per_second = sysconf(_SC_CLK_TCK);
            double user_time = (time2.tms_cutime)/(double)clock_ticks_per_second;
            double system_time = (time2.tms_cstime)/(double)clock_ticks_per_second;                       
            printf("Processes =         %d active\n", total);
            printf("Completed processes:\n");
            printf("User Time =         %.2lf seconds\n", user_time);
            printf("Sys  Time =         %.2lf seconds\n", system_time);

        }   


        /*******************************************************************************
        *                                    RUN CMD                                   *
        *******************************************************************************/
        else
        {
            // Save the command as a string for future reference
            std::string string_command;
                for(std::vector<std::string>::const_iterator i=my_vector.begin() ; i != my_vector.end() ; ++i){
                    string_command = string_command + *i + " ";
            }

            // Check if command is background process
            bool silent = false;
            if(my_vector[my_vector.size()-1] == "&"){
                silent = true;
                my_vector.erase(my_vector.end());
            }

            // File name for output
            bool output_bool = false;
            std::string my_output_file;
            for(int i=0 ; i<my_vector.size() ; i++){
                if(my_vector[i].find('>') != std::string::npos){
                    output_bool = true;
                    my_output_file = my_vector[i];
                    my_output_file.erase(std::remove(my_output_file.begin(), my_output_file.end(), '>'), my_output_file.end());
                    my_vector.erase(my_vector.begin() + i);
                }
            }

            // File name for input
            bool input_bool = false;
            std::string my_input_file;
            for(int i=0 ; i<my_vector.size() ; i++){
                if(my_vector[i].find('<') != std::string::npos){
                    input_bool = true;
                    my_input_file = my_vector[i];
                    my_input_file.erase(std::remove(my_input_file.begin(), my_input_file.end(), '<'), my_input_file.end());
                    my_vector.erase(my_vector.begin() + i);
                }
            }
                   
            // Create a vector containg <char*> to put into execvp
            std::vector<char*> the_arguments;
            the_arguments.clear();            

            // Insert elements from the vector<string> into the vector<char*>
            // Chceck if input is coming from a file
            if(input_bool){
                std::ifstream file(my_input_file.c_str());
                std::string temp_output_string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                std::vector<std::string> my_input_vector;
                my_input_vector.clear();
                std::istringstream my_input_string_stream_number_two(temp_output_string);
                std::string item2;   
                while (std::getline(my_input_string_stream_number_two, item2, ' ')) {
                    my_input_vector.push_back(item2);
                }                
                the_arguments.push_back(const_cast<char*>(my_vector[0].c_str()));
                for(int i=0 ; i<my_input_vector.size() ; i++){
                    the_arguments.push_back(const_cast<char*>(my_input_vector[i].c_str()));
                }                
            }
            else{
                for (int i=0 ; i<my_vector.size() ; i++){
                    the_arguments.push_back(const_cast<char*>(my_vector[i].c_str()));
                }
            }            

            // Add a null at the end of vector<char*>
            the_arguments.push_back(NULL);                      
            
            // Execution of background process
            if(silent)
            {
                int rc = fork();
                if (rc == 0)
                {
                    setpgid(0, 0);

                    if(output_bool){
                        int foo_file = open(my_output_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                        if(foo_file<0){
                            printf("FILE ERROR\n");
                        }
                        dup2(foo_file,1);
                    }

                    if (execvp(the_arguments[0], the_arguments.data()) < 0)
                    {
                        kill((int)getpid(), 9);
                    }
                }
                else
                {
                    printf(">>>Hey I'm the parent (%d) of child (%d)\n", (int)getpid(), rc);
                    fflush(stdout);

                    id_vector.push_back(rc);                   
                    pid_command_dict[rc] = string_command;
                }
            }
            // Execution of non-background process
            else
            {
                int rc = fork();
                if (rc == 0)
                {
                    setpgid(0, 0);
                    
                    if(output_bool){
                        int foo_file = open(my_output_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                        if(foo_file<0){
                            printf("FILE ERROR\n");
                        }
                        dup2(foo_file,1);
                    }

                    if (execvp(the_arguments[0], the_arguments.data()) < 0)
                    {
                        kill((int)getpid(), 9);
                    }
                    
                }
                else
                {
                    // EXTREMELY useful for debugging:
                    // printf(">>>Hey I'm the parent (%d) of child (%d)\n", (int)getpid(), rc);
                  
                    // Wait for the process to finish and sleep for a tenth of a second
                    fflush(stdout);
                    wait(NULL);
                    usleep(100000);                    
                }
            }               


        }

        // End of loop. Print newline.
        std::cout << std::endl;

    }            
   
}


/*

Along with the man pages and lecture notes, the following sources were consulted:

https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
https://stackoverflow.com/questions/35247570/execvp-not-working-when-converting-from-vectorstring-to-vectorchar-to-char
https://stackoverflow.com/questions/21528432/c-string-combining
https://stackoverflow.com/questions/7352099/stdstring-to-char
https://stackoverflow.com/questions/27541910/how-to-use-execvp
https://stackoverflow.com/questions/33901011/strcpy-is-undefined-c
https://stackoverflow.com/questions/1716296/why-does-printf-not-flush-after-the-call-unless-a-newline-is-in-the-format-strin
https://stackoverflow.com/questions/7663709/how-can-i-convert-a-stdstring-to-int
https://unix.stackexchange.com/questions/317492/list-of-kill-signals
https://stackoverflow.com/questions/1619020/how-to-check-if-a-process-with-a-pid-x-is-a-zombie
https://stackoverflow.com/questions/7155810/example-of-waitpid-wnohang-and-sigchld
https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rttims.htm
https://pubs.opengroup.org/onlinepubs/009695399/functions/times.html
https://stackoverflow.com/questions/1083142/what-s-the-correct-way-to-use-printf-to-print-a-clock-t
https://stackoverflow.com/questions/556405/what-do-real-user-and-sys-mean-in-the-output-of-time1
https://stackoverflow.com/questions/19919881/sysconf-sc-clk-tck-what-does-it-return
https://stackoverflow.com/questions/4668760/converting-an-int-to-stdstring
https://www.techiedelight.com/convert-string-to-int-cpp/

*/

