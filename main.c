#include "a1.h"

// Function to calculate histogram from file data
int *calculateHistogram(char *fileName, int fileSize) {

    // Allocate memory for the histogram with size for each alphabet
    int *histogram = calloc(NUM_ALPHABETS, sizeof(int));

    // Check if memory allocation was successful
    if (histogram == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Iterate through each character in the file content
    for (int j = 0; j < fileSize; j++) {
        // Retrieve the character from the file data and convert to lowercase 
        char character = tolower(fileName[j]);

        // Check if the character is an alphabet (case-insensitive)
        if (isalpha(character)) {
            // Map the character to an index in the histogram array
            int index = character - 'a';

            // Increment the count for the respective alphabet in the histogram
            histogram[index]++;
        }
    }

    //For formatting
    printf("\n");

    // Return the calculated histogram
    return histogram;
}

// SIGINT handler for child process
void sigint_handler(int sig) {
    // Exit immediately
    exit(SIGINT);
} 

// Signal handler for SIGCHLD
void sigchld_handler(int sig) {

    int childStatus; // Variable to store child process status
    pid_t childPID; // Variable to store child process ID
    int characterCounts[NUM_ALPHABETS]; // Array to store character counts for histogram
    char histFileName[BUFFER_SIZE]; // Buffer to store histogram file name
    char buffer[BUFFER_SIZE]; // Buffer to store line to write to file

    // Loop to handle all terminated children
    while ((childPID = waitpid(-1, &childStatus, WNOHANG)) > 0) {

        // Print a message indicating which child process was caught
        printf("Parent caught SIGCHLD from child: %d\n", childPID);

        // Increment the counter for terminated child processes
        numTerminatedChildProcesses++;

        if (WIFEXITED(childStatus)) {

            int code = WEXITSTATUS(childStatus);
            // Find the correct pipe associated with the terminated child process
            if(code == 0){
            int currentPipe = -1;
            for (int j = 0; j < MAX_NUM_FILES; j++) {
                if (childPID == childPIDs[j]) {
                    currentPipe = j;
                    break;
                }
            }

            // Check if the pipe was found
            if (currentPipe == -1) {
                printf("Child PID %d not found.\n", childPID);
                continue;
            }

            // Read histogram from pipe into memory
            read(communicationPipes[currentPipe][0], characterCounts, sizeof(int) * NUM_ALPHABETS);
            close(communicationPipes[currentPipe][0]);
            printf("The parent reads histogram from pipe number %d.\n", currentPipe);
            currentPipe++;

            // Open a hist file to write information
            sprintf(histFileName, "file%d.hist", childPID);
            printf("Saved to file %s.\n", histFileName);

            //Open the file for writing
            int fileDescriptor = open(histFileName, O_CREAT | O_WRONLY, 0644);

            // Check if the file was opened successfully
            if (fileDescriptor == -1) {
                perror("Cannot open the file");
                exit(EXIT_FAILURE);
            }

            // Write character counts to the file
            for (char alphabet = 'a'; alphabet <= 'z'; alphabet++) {
                //Stores output in char buffer 
                sprintf(buffer, "%c=%d\n", (char) alphabet, characterCounts[alphabet - 'a']);
                write(fileDescriptor, buffer, strlen(buffer));
            }
            close(fileDescriptor);

            printf("Child %d was terminated successfully!\n", childPID);
            }else{
                printf("Child terminated abnormally\n");
            }
        } else if (WIFSIGNALED(childStatus)) {
            printf("Child %d was terminated abnormally!\n", childPID);
        }
    }

    // Reinstall the signal handler
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        perror("Couldn't register SIGCHLD\n");
        exit(EXIT_FAILURE);
    } 
}



int main(int argc, char *argv[]) {

    int *histogram;  // Pointer to store the histogram data
    int numBytes;    // Variable to store the number of bytes read from the file
    int fileSize;    // Variable to store the size of the file in bytes
    int fd;          // File descriptor for the opened file
    char *data;      // Pointer to store the content of the file being read

    // Check if file names are provided
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check if there are too many files
    if (argc > MAX_NUM_FILES + 1) {
        printf("Too many files! \n");
        exit(EXIT_FAILURE);
    }

    // Get the signal handler for SIGCHLD and register it
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        perror("Caught SIGCHILD");
        exit(EXIT_FAILURE);
    }

    // Fork a child process for each file provided as argument
    int pid;
    for (int i = 0; i < (argc -1); i++) {
        // Create a pipe for communication with the child process
        int response = pipe(communicationPipes[i]); 

        //Could not create the pipe 
        if (response < 0) {
            perror("Couldn't create pipe");
            exit(EXIT_FAILURE);
        }

        // Fork a child process
        pid = fork(); 

        // Checking for errors
        if (pid == -1) {
            perror("fork() error");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            close(communicationPipes[i][0]); // Close read end of the pipe in the child

            // Register SIGINT handler
            //if (signal(SIGINT, sigint_handler) == SIG_ERR) {
            //    perror("Couldn't register SIGINT");
            //    exit(EXIT_FAILURE);
            //} 

            if(strcmp(argv[i+ 1], "SIG") == 0){
                for(;;){
                    pause();
                }
            }            

            // Get file information
            struct stat st;
            if (stat(argv[i+1], &st) == -1) {
                perror("Error while getting file information");                    close(communicationPipes[i][1]);
                exit(EXIT_FAILURE);
            }
            fileSize = st.st_size;

            // Open the file for reading
            fd = open(argv[i+1], O_RDONLY);

            if (fd < 0) {
                perror("Error while opening the file");
                close(communicationPipes[i][1]);
                exit(EXIT_FAILURE);
            }

            // Allocate memory to read file content
            data = malloc(fileSize + 1);

            if (data == NULL) {
                perror("Memory allocation error");
                close(fd); //Close the file descriptor
                close(communicationPipes[i][1]); //Close the pipe write end 
                exit(EXIT_FAILURE);
            }

            // Read file content into memory
            while ((numBytes = read(fd, data, fileSize)) > 0) {
                if (numBytes < 0) {
                    perror("Error while reading the file");
                    close(fd);  //Close the file descriptor
                    close(communicationPipes[i][1]);  //close the pipe write end 
                    exit(EXIT_FAILURE);
                }
            }
            close(fd); //close the file descriptor after successfully reading

            // Calculate Histogram and write it to pipe
            sleep(10 + 3 * i);
            histogram = calculateHistogram(data, fileSize);                write(communicationPipes[i][1], histogram, NUM_ALPHABETS * sizeof(int));
            free(histogram);

            // Print a message indicating successful writing
            printf("Child with PID: %d successfully written using pipe %d.\n", getpid(), i);

            // Child process sleeps for a specified time
            printf("Child with PID: %d is going to sleep for %d seconds.\n", getpid(), (10 + 3 * i));    

            // Closing write end of the pipe
            close(communicationPipes[i][1]);
            free(data);
            exit(EXIT_SUCCESS);
            
        } else {
            // Parent process
            close(communicationPipes[i][1]); // Close write end of the pipe in the parent
            numChildProcesses++;   // Increment the count of child processes
            childPIDs[i] = pid;   // Store the child process ID
            if (strcmp(argv[i+1], "SIG") == 0) {
                // If argument is "SIG", send SIGINT signal to the child process
                kill(pid, SIGINT);
            }
        }
    }

    // Wait until all children are terminated
    while (numTerminatedChildProcesses < numChildProcesses) {
        sleep(1);
    }

    return 0;
}

