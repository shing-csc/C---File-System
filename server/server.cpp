#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include <string>
#include <cstring>

#include "server_upload.h"

using namespace std;
const int PORT = 8080;
const int BUFFER_SIZE = 1024;


int main(){
    
    // socket structure: In <sys/socket.h>
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Define the structure of the socket
    

    // socket address struture: In <netinet/in.h>
    sockaddr_in serverAddress;                  
    serverAddress.sin_family = AF_INET;         // Setting the server socket address' format (IPv4)
    serverAddress.sin_port = htons(PORT);       // Setting the server socket port: in binary form 
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Setting the server socket's address

    // Binding the address and structure
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)); // Convert the type of sockaddr_in -> sockaddr


    // Listens for client's connection REQUEST
    listen(serverSocket, 10); 
    cout << "Server is listening on port " << PORT << endl;

    // Accepts the CONNECTION REQUEST from client
    int clientSocket = accept(serverSocket, nullptr, nullptr);


    // TODO: Add a LOOP here

    while(true){
        // Handling of user command: "UPLOAD"/"DOWNLOAD"/"REMOVAL/EXIT"
        char buffer_command[1024];
        
        // Remember the blockage functionality of recv function
        ssize_t bufferLength = recv(
            clientSocket,
            buffer_command,
            BUFFER_SIZE,
            0
        );

        // Testing: blockage of recv: cout << "blocking2?"<<endl;
        buffer_command[bufferLength] = '\0';
        
        cout << "LOG: " << buffer_command << " command request " << endl;
        std::string command(buffer_command, bufferLength);

        if (strcmp(buffer_command, "UPLOAD") == 0){
            try{
                handleUpload(clientSocket, BUFFER_SIZE);

            } catch (const std::exception& e) {
                cerr << "LOG: Error in handleDownload: " << e.what() << endl;
            }
            
        }
        else if (strcmp(buffer_command, "DOWNLOAD") == 0){
            
        }
        else if (strcmp(buffer_command, "REMOVAL") == 0){

        }
        else if (strcmp(buffer_command, "EXIT") == 0){
            
            close(clientSocket);
            close(serverSocket);
            break;
        }

        cout << "looping?"<<endl;
    }
    // TODO: ERROR handling of the above functions causes an ERROR, the loop will still continue
    
    close(clientSocket);
    close(serverSocket);

    return 0;
}