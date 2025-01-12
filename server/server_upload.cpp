#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include <string>
#include <cstring>

#include <filesystem>

#include "server_upload.h"

using namespace std;
namespace fs = std::filesystem;

void handleUpload(int clientSocket, int bufferSize){

    // Buffer setup: Handles file to be upload
    int BUFFER_SIZE = bufferSize;

    char buffer_orgFileNameSize[sizeof(uint32_t)];
    char buffer_outFileNameSize[sizeof(uint32_t)];

    char buffer_orgFileName[BUFFER_SIZE];
    char buffer_outFileName[BUFFER_SIZE];

    char buffer_fileValid[2];
    char buffer_data[BUFFER_SIZE];

    // SERVER ACTION 1: Obtain filename size from CLIENT

    ssize_t orgNameSizeLength = recv(
        clientSocket,
        buffer_orgFileNameSize,
        sizeof(uint32_t),
        0
    );

    uint32_t orgFileNameSizeNetwork;
    std::memcpy(&orgFileNameSizeNetwork, buffer_orgFileNameSize, sizeof(uint32_t));
    uint32_t orgFileNameSize = ntohl(orgFileNameSizeNetwork);
    
    ssize_t nameLength = recv(
        clientSocket, 
        buffer_orgFileName, 
        orgFileNameSize, 
        0
    );
    buffer_orgFileName[nameLength] = '\0';
    cout << "LOG: Filename Received:" << buffer_orgFileName << endl;

    // SERVER ACTION 2: Obtain file validity from CLIENT 
    ssize_t validLength = recv(
        clientSocket,
        buffer_fileValid,
        1,
        0
    );
    buffer_fileValid[validLength] = '\0';

    if (strcmp(buffer_fileValid, "V") == 0){
        cout << "LOG: Valid Filename" << endl;
    }
    else if (strcmp(buffer_fileValid, "I") == 0){
        cout << "LOG: Invalid Filename" << endl;
        return;
    }

    ssize_t outNameSizeLength = recv(
        clientSocket,
        buffer_outFileNameSize,
        sizeof(uint32_t),
        0
    );

    uint32_t outFileNameSizeNetwork;
    std::memcpy(&outFileNameSizeNetwork, buffer_outFileNameSize, sizeof(uint32_t));
    uint32_t outFileNameSize = ntohl(outFileNameSizeNetwork);

    cout <<"FUD"<<endl;
    // SERVER ACTION 3: Obtain target filename from CLIENT 
    ssize_t targetFileNameLength = recv(
        clientSocket, 
        buffer_outFileName, 
        outFileNameSize, 
        0
    );

    /**
     *  PROBLEM RN: 
     *      - cannot create file from server
     *      - ./server can print out "LOG" while handleUpload in "server_upload" cannot -> will it becasue they dont have access to the variable? -> PUBLIC? NO
     *      
     *      
     *  Possible cause: 
     *      - Wrong file route
     *      - Cannot accept the update filename
     * 
     *  Possible solution
     *      - Try to see any methods to print things out in terminal when executing ./server in another thread
     * 
     *  */ 

    buffer_outFileName[targetFileNameLength] = '\0';
    std::string targetFileName(buffer_outFileName, targetFileNameLength);

    string fullFilePath = fs::path(SERVER_DB_PATH) / targetFileName; 

    ofstream outFile(fullFilePath, std::ios::binary); // Create the file if the file with this filename not exist

    cout << "LOG: Target Filename Received: " << fullFilePath << endl;

    // SERVER ACTION 4: Obtain data from target file from CLIENT
    ssize_t bytesRead;

    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << fullFilePath << std::endl;
    }
    while((bytesRead = recv(clientSocket, buffer_data, BUFFER_SIZE, 0)) > 0){
        
        // SERVER ACTION 5: Receiving end of transmission message to end the blocking function caused by recv()
        if (strncmp(buffer_data, "EOF", bytesRead) == 0){
            outFile.flush();
            cout << "End of transmission" << endl;
            break;
        }
        
        // Write the exact data read to the file
        outFile.write(buffer_data, bytesRead);
        outFile.flush();

        if (!outFile) {
            std::cerr << "Error writing to file: " << fullFilePath << std::endl;
            break;
        }
        
    }

    if (bytesRead == 0){
        cout << "File upload to " << fullFilePath << " complete!" << endl;
    }
    else{
        cout << "File upload failed" << endl;
    }

    outFile.close();
    return;
}