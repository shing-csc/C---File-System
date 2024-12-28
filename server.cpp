#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include <string>
#include <cstring>


using namespace std;
constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;


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
    listen(serverSocket, 5); 
    cout << "Server is listening on port " << PORT << endl;


    // Accepts the CONNECTION REQUEST from client
    int clientSocket = accept(serverSocket, nullptr, nullptr);


    // Setting the buffer size of each buffer: temporary storage of data
    char buffer_fileName[1024];

    ssize_t nameLength = recv(
        clientSocket, 
        buffer_fileName, 
        BUFFER_SIZE, 
        0);
    
    buffer_fileName[nameLength] = '\0';
    cout << "Receiving filename " << buffer_fileName << endl;

    // Creation of output file
    ofstream outFile(buffer_fileName, ios::binary); // Create the file if the file with this filename not exist


    // Handle the file data
    char buffer_data[BUFFER_SIZE];
    ssize_t bytesRead;

    while((bytesRead = recv(clientSocket, buffer_data, BUFFER_SIZE, 0)) > 0){
        
        // data:      buffer_data
        // data size: bytesRead, not necessary equal to buffersize because of the LAST packet
        outFile.write(buffer_data, bytesRead);
    }

    if (bytesRead == 0){
        cout << "File upload complete!" << endl;
    }
    else{
        cout << "File upload failed" << endl;
    }

    outFile.close();
    close(clientSocket);
    close(serverSocket);

    return 0;
}