#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

// argc is the number of input
// argv is the argument of input
// argv[1]: The path of .txt file
// argv[2]: The message write to .txt file

int main(int argc, char *argv[])
{
    // Setup syslog logging for my utility ussing the LOG_USER facility
    openlog("myLog", 0, LOG_USER);

    if (argc != 3)  
    {
        printf("Your number of inputs is wrong, please just type 2 inputs !!\n");
        syslog(LOG_ERR, "Error, Your number of inputs is wrong, please just type 2 inputs !!\n");
        exit(1);
    }
    else if(strcmp(argv[1], "") == 0)
    {
        printf("Your first input can't be empty, please type the path of .txt file here (include the .txt file in the path) !!\n");
        syslog(LOG_ERR, "Error, Your first input can't be empty, please type the path of .txt file here (include the .txt file in the path) !!\n");
        exit(1);
    }
    else if(strcmp(argv[2], "") == 0)
    {
        printf("Your second input can't be empty, pleasy type the message you want to write in the .txt file !!\n");
        syslog(LOG_ERR, "Error, Your second input can't be empty, pleasy type the message you want to write in the .txt file !!\n");
        exit(1);
    }

    // Open and write to the file 
    FILE *f;
    f = fopen(argv[1], "w");
    if (f == NULL)
    {
        printf("Can not open the file. Maybe your path is not exist !!\n");
        syslog(LOG_ERR, "Error, Can not open the file. Maybe your path is not exist !!\n");
        exit(1);
    }
    else
    {
        // Write the message to file
        fputs(argv[2], f);
        printf("Writing %s to %s successfully\n", argv[2], argv[1]);     
        syslog(LOG_DEBUG, "Writing %s to %s successfully\n", argv[2], argv[1]);   
    }
    
    fclose(f);
    closelog();

    return 0;
}