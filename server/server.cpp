#include <iostream>
#include <fstream>

#include <string>
#include <cstring>

// For multithreading
#include <thread>
#include <mutex>
#include <vector>

// For netwoeking
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <arpa/inet.h>

// Header files
#include "../include/server_upload.h"

using namespace std;
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

void handleMultiRequests(int clientSocket, int bufferSize){

    while(true){
        // Handling of user command: "UPLOAD"/"DOWNLOAD"/"REMOVAL/EXIT"
        char buffer_command[1024];
        
        // Remember the blockage functionality of recv function
        ssize_t bufferLength = recv(
            clientSocket,
            buffer_command,
            bufferSize,
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
            break;
        }

        cout << "looping?"<<endl;
    }
    
    // TODO: ERROR handling of the above functions causes an ERROR, the loop will still continue
    
    close(clientSocket);
}


int main(){
    
    // socket structure: In <sys/socket.h>
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Define the structure of the socket
    

    // socket address struture: In <netinet/in.h>
    sockaddr_in serverAddress;                  
    serverAddress.sin_family = AF_INET;         // Setting the server socket address' format (IPv4)
    serverAddress.sin_port = htons(PORT);       // Setting the server socket port: in binary form 
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Setting the server socket's address

    // Convert the type of sockaddr_in -> sockaddr
    if (::bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        cout << "LOG: Failed to bind socket" << endl;
    }
    else{
        cout << "LOG: Successfully bind socket" << endl;
    }
    
    // Listens for client's connection REQUEST
    if (::listen(serverSocket, 10) < 0 ){
        cout << "LOG: Failed to listen client requests" << endl;
    }   
    else{
        cout << "LOG: Successfully listening client requests" << endl;
    }
    cout << "Server is listening on port " << PORT << endl;

    // Accepts the CONNECTION REQUEST from client

    vector<thread> clientThreads; // vector of threads
    
    while(true){
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1 ){
            cerr << "LOG: Failed to accept client connection" << endl;
            continue; // Skip this iteration and try again
        }
        clientThreads.emplace_back(std::thread(handleMultiRequests, clientSocket, BUFFER_SIZE));

    }

    for (auto &t : clientThreads){
        if (t.joinable()){
            t.join();
        }
    }
    
    close(serverSocket);

    return 0;
}