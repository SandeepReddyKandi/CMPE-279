// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8080 
#define UID 65534

const char *hello = "Hello from server"; 
int setup_socket();
void do_execvp(int new_socket);
void send_message(const char* new_socket_str);

int main(int argc, char const *argv[]) 
{ 
    //printf("Number of arg: %d \n", argc);
    // main process
    if (argc == 1) 
    {
        int new_socket;
        //setup socket and return socket id
        new_socket = setup_socket();

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
            int status = 0;
            while ((wait(&status)) > 0);
        }
        
        //forked process, set uid and do execvp function to excecue sub program.
        else 
        {
            do_execvp(new_socket);
        }
    }
    // image process created by fork process which user id is UID will run this part.
    else 
    {
        if(setuid(65534) < 0){
            perror("drop previlege failed");
            exit(EXIT_FAILURE);
        }
        send_message(argv[1]);
    }
    return 0;
}


int setup_socket() 
{
    int server_fd, new_socket;
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    
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
    address.sin_port = htons( PORT ); 
       
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
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    return new_socket;
} 

// This is a function that run execvp.
void do_execvp(int new_socket) 
{
    // convert socket id to string
    char socket_id_str[12];
    sprintf(socket_id_str, "%d", new_socket);
    
    // pass arguments to sub-program
    // new exec program will run as: ./server {ACTUAL_SOCKET_ID}
    char *args[] = {"./server", socket_id_str, NULL};
    if (execvp(args[0], args) < 0) {
        perror("exec");
        exit(EXIT_FAILURE);
    };
}


// Image process handle sending message to client.
void send_message(const char* new_socket_str) 
{
    int new_socket, valread;
    char buffer[1024] = {0};
    new_socket = atoi(new_socket_str);
    valread = read(new_socket, buffer, 1024);
    if(valread < 0) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }
    printf("%s\n",buffer );
    send(new_socket , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
}
