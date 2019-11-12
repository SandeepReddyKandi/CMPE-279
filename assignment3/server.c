// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#define PORT 8080 
#define UID 65534
#define CHAR_BUF_SIZE 1024
#define NO_FILE "File Not Found!"
#define EMPTY_FOLDER "./empty_folder"

const char *hello = "Hello from server"; 
void setup_socket();
void do_execvp();
void send_message();
void parse_parameter(int argc, char const *argv[]);
void chroot_to_empty_folder();

int port = PORT;
char fileName[FILENAME_MAX];
int socket_id;
int use_file = 0;
int fptr;

// Server Running command: ./server -p {PORT_NUM} -f {FILE_NAME}
// Example: ./server -p 9999 -f test

// Client Running command: ./client {PORT_NUM}
// Example: ./client 9999
int main(int argc, char const *argv[]) 
{ 
    parse_parameter(argc, argv);
    // main process
    if (argc == 1 || strcmp(argv[1], "-s") != 0) 
    {
        //setup socket
        setup_socket();

        pid_t sub_process = fork();
        
        //fork failed
        if (sub_process < 0) 
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        
        // main process
        if (sub_process != 0) 
        {
            chroot_to_empty_folder();
            int status = 0;
            while ((wait(&status)) > 0);
        }
        
        //forked process, set uid and do execvp function to excecue sub program.
        else 
        {
            do_execvp();
        }
    }
    // image process created by fork process which user id is UID will run this part.
    else 
    {
        if(setuid(65534) < 0){
            perror("drop previlege failed");
            exit(EXIT_FAILURE);
        }
        send_message(socket_id);
    }
    return 0;
}


void setup_socket() 
{
    int server_fd;
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    
    printf("port number is %d \n", port);
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((socket_id = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
} 

// This is a function that run execvp.
void do_execvp() 
{
    // convert socket id to string
    char socket_id_str[12];
    sprintf(socket_id_str, "%d", socket_id);
    
    // pass arguments to sub-program
    
    if (use_file)
    {
        char file_descriptor[12];
        sprintf(file_descriptor, "%d", fptr);
        char *args[] = {"./server", "-s", socket_id_str, "-d", file_descriptor, NULL};
        if (execvp(args[0], args) < 0) {
            perror("exec");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        char *args[] = {"./server", "-s", socket_id_str, NULL};
        if (execvp(args[0], args) < 0) {
            perror("exec");
            exit(EXIT_FAILURE);
        }
    }
}


// Image process handle sending message to client.
void send_message() 
{
    int new_socket, valread;
    char buffer[1024] = {0};
    valread = read(socket_id, buffer, 1024);
    if(valread < 0) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }
    //case of use file
    if (use_file)
    {
        char char_buffer[CHAR_BUF_SIZE];
        read(fptr, char_buffer, CHAR_BUF_SIZE);
        send(socket_id , char_buffer , CHAR_BUF_SIZE, 0 );
        close(fptr);
    }
    // send hello
    else {
        send(socket_id , hello , strlen(hello) , 0 );
    }
    printf("Message sent\n");
}

//parse the paremeter for ./run_server
void parse_parameter(int argc, char const *argv[])
{
    for (int i = 1; i < argc - 1; i += 2)
    {
        if (strcmp(argv[i], "-p") == 0) 
        {
            port = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            strcpy(fileName, argv[i+1]);
            if ((fptr = open(fileName, O_RDONLY)) < 0)
            {
                printf("Error! opening file");
                exit(EXIT_FAILURE);
            }
            use_file = 1;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            socket_id = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
            use_file = 1;
            fptr = atoi(argv[i+1]);
        }
        else
        {
            printf("Unknown parameter %s", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
}

//Create empty folder and change root to empty folder
void chroot_to_empty_folder()
{
    struct stat st = {0};

    if (stat(EMPTY_FOLDER, &st) == -1) {
        mkdir(EMPTY_FOLDER, 0700);
    }
    
    if (chdir(EMPTY_FOLDER) < 0) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }
    if(chroot(EMPTY_FOLDER) < 0) {
        perror("chroot");
        exit(EXIT_FAILURE);
    }
    printf("Change root to empty folder successfully\n");
}
