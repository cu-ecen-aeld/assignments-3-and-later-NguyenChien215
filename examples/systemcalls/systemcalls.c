#include "systemcalls.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    int result = system(cmd);
    bool IsCommandRun;
    if(result == 0)
    {
        IsCommandRun = true;
    }
    else{
        IsCommandRun = false;
    }
/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/

    return IsCommandRun;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    bool result = true;

    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    pid_t child_process = fork();

    if(child_process < 0) // error
    {
        result = false;
    }
    else if (child_process == 0) // in child process
    {
        execv(command[0], command);
        exit(1);
    }
    else {
        int status;
        if (waitpid(child_process, &status, 0) < 0 || status != 0) result = false; // error
    }
    
    va_end(args);

    return result;
    

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    bool result = true;

    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    pid_t child_process2 = fork();
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) 
    {   
        perror("Error");
        exit(1);
    }
        
    
    if(child_process2 == -1)
    {
        result = false;
    }
    else if (child_process2 == 0)
    {
        if (dup2(fd, 1) < 0) 
        {
            exit(1);
        }
        execv(command[0], command);
        exit(1); // if we are here, execv failed
    }
    else
    {
        int status;
        if (waitpid(child_process2, &status, 0) < 0 || status != 0) result = false; // error
    }

    close(fd);
    va_end(args);

    return result;
}
