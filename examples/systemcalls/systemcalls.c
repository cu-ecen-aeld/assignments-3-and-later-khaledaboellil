#include "systemcalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int ret = system(cmd);

    if (ret == -1 || !WIFEXITED(ret))
    {
        return false ;
    }
    else
    {
        return true;
    }
    
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
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    va_end(args);
    pid_t pid = fork();
    if (pid == -1)
    {
        // Child process
        if (execv(command[0], command) == -1) {
            // execv failed
            perror("Execv failed");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process
        int status;
        int wait = waitpid(pid, &status, 0);

        if(wait  == -1){
            perror("wait error: ");
        }
        else {
            if (WEXITSTATUS(status) != EXIT_SUCCESS){
                return false;
            }
            else if (WIFSIGNALED(status) != EXIT_SUCCESS){
                    return false;
            }
        }
    }
    return true ;
    
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
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);
    // Create a file descriptor for the output file
    int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (fd == -1) {
        // Error opening the output file
        perror("Error opening output file");
        return false;
    }
    pid_t child_pid = fork();

    if (child_pid == -1) {
        // Fork failed
        perror("Fork failed");
        return false;
    }
     if (child_pid == 0) {
        // Child process

        // Redirect standard output to the output file
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("Dup2 failed");
            exit(EXIT_FAILURE);
        }

        // Close the file descriptor for the output file, as it's no longer needed
        close(fd);

        // Execute the command
        if (execv(command[0], command) == -1) {
            // execv failed
            perror("Execv failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process

        // Close the file descriptor for the output file in the parent process
        close(fd);

        int status;
        waitpid(child_pid, &status, 0);

        if (WIFEXITED(status)) {
            // Child process terminated
            return WEXITSTATUS(status) == 0;
        } else {
            // Child process terminated with an error
            return false;
        }
    }
    return true ;
}
