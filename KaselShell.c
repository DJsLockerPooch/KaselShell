#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int shellExit(char **args);                                   // Function to exit the program
void shellLoop(void);                                         // Function declaration
char *readLine(void);
char **splitLine(char *line);
int execute(char **args);
int launch(char **args);
int position;                                                 // Creating a position variable (which will double as our size variable)

int pipeIndex = -1;                                           // Index of the pipe, otherwise it is -1
int numArguments = 0;                                         // The number of total arguments in our command
char *builtin_str[] = {                                       // Strings we want to have a specific functions for in our custom shell
  "exit"
};
int (*builtin_func[]) (char **) = {                           // Declaring the function location
  &shellExit
};
int getNumBuiltIns() {                                        // Getting the number of build in functions, in our case just the exit
  return sizeof(builtin_str) / sizeof(char *);
}
int shellExit(char **args)                                    // Escaping the program by returning 0
{
  return 0;
}

void shellLoop(void)                                          // Function that will loop through all the functions our shell will use
{
  char *line;                                                 // The line of input from the user
  char **args;                                                // The line broken up into arguments (in an array)
  int status;                                                 // The condition of whether or not we continue or exit
  do {
    printf(">>> ");                                           // Just to look like a shell
    line = readLine();                                        // Read the line the user input
    args = splitLine(line);                                   // Split the line given into an arguments array
    for (int i = 0; i < sizeof(args); i++){
      if (args[i] != NULL){                                   // If there IS something
        numArguments += 1;                                    // We incriment the number of arguments by 1
      }
    }
    status = execute(args);                                   // Execute the given arguments
    free(line);                                               // Deallocating the space of line
    free(args);                                               // Deallocating the space of args
  } while (status);                                           // While we don't exit, continue running
}

#define LSH_RL_BUFSIZE 1024                                   // Initializing the buffersize to 1024
char *readLine(void)                                          // Function to read the input the user entered
{
  int bufsize = LSH_RL_BUFSIZE;                               // Setting the buffersize to 1024
  int position = 0;                                           // Setting the position we start in the array to 0
  char *buffer = malloc(sizeof(char) * bufsize);              // Creating the buffer with the buffersize times the size of a char
  int c;                                                      // In case we get an End Of File character or new line character
  while (1) {                                                 // While true
    c = getchar();                                            // Get the next character the user input
    if (c == EOF || c == '\n') {                              // If it's an End Of File character or new line
      buffer[position] = '\0';                                // Set the position to be the NULL character
      return buffer;                                          // Return the buffer becasuse we have finished
    } else {                                                  // Otherwise we have a char we want to consider
      buffer[position] = c;                                   // Set the position in the buffer to be the char we got
    }
    position++;                                               // Incriment the position in our buffer
  }
}

#define LSH_TOK_BUFSIZE 64                                    // Initializing the buffer size (since we can't just allocate a space)
#define LSH_TOK_DELIM " \t\r\n\a"                             // Initializing the delimiters in which we will tokenize
char **splitLine(char *line)                                  // Function to split the line into individual arguments
{
  int bufsize = LSH_TOK_BUFSIZE;                              // Initializing the buffersize and the position (where we are in the array) to 0
  char **tokens = malloc(bufsize * sizeof(char*));            // Creating the array with the given buffer size multiplied by the size of a char
  char *token;                                                // Initializing the pointer to the specific token we got from tokenizing
  position = 0;                                               // Ensuring the position is reset to 0
  if (!tokens) {                                              // If the array isn't initialized correctly
    fprintf(stderr, "lsh: allocation error\n");               // Print the error
    exit(EXIT_FAILURE);                                       // Return EXIT_FAILURE
  }
  token = strtok(line, LSH_TOK_DELIM);                        // Tokenize the line by the delimiters
  while (token != NULL) {                                     // While the token isn't NULL
    tokens[position] = token;                                 // We set the position in the array to be the token we just got
    position++;                                               // Move the position to the next location
    token = strtok(NULL, LSH_TOK_DELIM);                      // Setting token to be NULL
  }
  return tokens;                                              // Returning the finalized array of tokens the user input
}

int execute(char **args)                                      // Function to execute the built in arguments
{
  if (numArguments == 0) {                                    // If args is empty
    return 1;                                                 // Return success
  }
  for (int i = 0; i < getNumBuiltIns(); i++) {                // For each built in function we initialized
    if (strcmp(args[0], builtin_str[i]) == 0) {               // If the first element in args is any of those built in functions
      return (*builtin_func[i])(args);                        // We return the built in function's method call
    }
  }
  return launch(args);                                        // Otherwise we perform the user input
}

int launch(char **args)                                       // Function to execute the commands the user entered (not built in)
{
  char **left = malloc(1024 * sizeof(char*));                 // Creating the left side of the command
  char **right = malloc(1024 * sizeof(char*));                // Creating the right side of the command
  left[0] = NULL;                                             // Making sure that the left side is always reset
  left[1] = NULL;
  right[0] = NULL;                                            // Making sure that the right side is always reset
  right[1] = NULL;
  pipeIndex = -1;                                             // Making sure the pipe index is reset
  for (int i = 1; i < 3; i++){                                // Checking to see if the pipe is the second or third argument entered
    if (args[i] != NULL && position >= 3){                    // Ensuring the argument isn't NULL
      if (strcmp(args[i], "|") == 0){                         // If the argument is the pipe
        pipeIndex = i;                                        // Set the pipeIndex to the number in the args array
      }
    }
  }
  left[0] = args[0];                                          // Setting the first argument to be the first argument in args
  if (args[1] != NULL && pipeIndex != 1){                     // Checking to see if the second argument is not NULL and not the pipe
    left[1] = args[1];                                        // Set the second argument to the second argument in args
  }
  if (pipeIndex != -1){                                       // If there is a pipe
    right[0] = args[pipeIndex + 1];                           // Set the first right argument to the first argument after the pipe
    if (args[pipeIndex + 2] != NULL){                         // If the next argument isn't a pipe (because we are only accounting for one pipe)
      right[1] = args[pipeIndex + 2];                         // Set the second right argument to the second argument after the pipe
    }
  }
  // Everything up until here is perfect

  int fd[2];                                                  // Write:0 then Read:1
  if (pipeIndex != -1){                                       // If there is a pipe
    fd[0] = 0;                                                // Initializing the front end of the pipe
    fd[1] = 1;                                                // Initializing the back end of the pipe
    pipe(fd);                                                 // Creates the "pipe" (one way directional from read to write)
  }
  pid_t pid, wpid;                                            // Creating the id's for the processes that we want to track
  int pid2;                                                   // Creating the int representing the second child id
  int status = 0;                                             // Location for our status to be placed into
  pid = fork();                                               // Fork the parent and place into pid
  if (pid == 0) {                                             // Child process
    if (pipeIndex != -1){                                     // If there is a pipe
      dup2(fd[1], STDOUT_FILENO);                             // Changing the output of the standard out to our file descriptor
      close(fd[0]);                                           // Closing the read end since we are writing to the pipe and not reading from it
    }
    if (left[1] == NULL){                                     // If the second argument in left is NULL
      execlp(left[0], left[0], (char*) NULL);                 // Performing the command
    } else {                                                  // Otherwise there EXISTS an argument in the position after the first argument after the pipe
      execlp(left[0], left[0], left[1], (char*) NULL);        // Performing the command
    }
  } else {                                                    // Parent process
    if (pipeIndex != -1){                                     // If there is a pipe we call the fork again for the right side of the command
      pid2 = fork();                                          // Fork again for the second program
      if (pid2 == 0){                                         // Second child process
        dup2(fd[0], STDIN_FILENO);                            // Changing the input of the standard in to our other file descriptor
        close(fd[1]);                                         // Closing the write end since we are reading from the pipe and not reading from it
        if (right[0] != NULL && right[1] == NULL){            // If the second argument in Right is NULL
          execlp(right[0], right[0], (char*) NULL);           // Perform the command
        } else if (right[0] != NULL && right[1] != NULL){     // Otherwise if the first AND second right side arguments are not NULL
          execlp(right[0], right[0], right[1], (char*) NULL); // Perform the command
        }
      } else {
        close(fd[1]);                                         // Closing the output end
        close(fd[0]);                                         // Closing the input end
        waitpid(pid, &status, 0);                             // Waiting on the first child (left side of argument)
        waitpid(pid2, &status, 0);                            // Waiting on the second child (right side of command)
      }
    } else {
      waitpid(pid, &status, 0);                               // Waiting on the first child (left side of argument)
    }
  }
  return 1;                                                   // Return success
}

int main(int argc, char **argv)
{
  printf("\nWelcome to KaselShell!\n");                       // Welcoming the user to the shell program
  shellLoop();                                                // Running the shell loop function
  return EXIT_SUCCESS;                                        // After we exit we return EXIT_SUCCESS
}
