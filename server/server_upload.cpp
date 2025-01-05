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
#include "server_directory.h"

using namespace std;

void handleUpload(int clientSocket, int bufferSize){

    // Buffer setup: Handles file to be upload
    int BUFFER_SIZE = bufferSize;
    char buffer_fileName[BUFFER_SIZE];
    char buffer_targetFileName[BUFFER_SIZE];
    char buffer_fileValid[BUFFER_SIZE];
    char buffer_data[BUFFER_SIZE];

    // Obtain diretory
    string target_directory = handleDirectory();


    // SERVER ACTION 1: Obtain filename from CLIENT
    ssize_t nameLength = recv(
        clientSocket, 
        buffer_fileName, 
        BUFFER_SIZE, 
        0
    );
    buffer_fileName[nameLength] = '\0';
    cout << "LOG: Filename Received::" << buffer_fileName << endl;


    // SERVER ACTION 2: Obtain file validity from CLIENT 
    ssize_t validLength = recv(
        clientSocket,
        buffer_fileValid,
        BUFFER_SIZE,
        0
    );
    buffer_fileValid[validLength] = '\0';

    if (strcmp(buffer_fileValid, "VALID") == 0){
        cout << "LOG: Valid Filename" << endl;
    }
    else if (strcmp(buffer_fileValid, "INVALID") == 0){
        cout << "LOG: Invalid Filename" << endl;
        return;
    }


    // SERVER ACTION 3: Obtain target filename from CLIENT 
    ssize_t targetFileNameLength = recv(
        clientSocket, 
        buffer_targetFileName, 
        BUFFER_SIZE, 
        0
    );
    buffer_targetFileName[targetFileNameLength] = '\0';
    string fullFilePath = target_directory + buffer_targetFileName; 
    ofstream outFile(fullFilePath, ios::binary); // Create the file if the file with this filename not exist

    cout << "LOG: Target Filename Received:: " << fullFilePath << endl;

    // SERVER ACTION 4: Obtain data from target file from CLIENT
    ssize_t bytesRead;

    while((bytesRead = recv(clientSocket, buffer_data, BUFFER_SIZE, 0)) > 0){
        
        // data:      buffer_data
        // data size: bytesRead, not necessary equal to buffersize because of the LAST packet
        cout << buffer_data << endl;
        outFile.write(buffer_data, bytesRead);
    }

    if (bytesRead == 0){
        cout << "File upload to " << fullFilePath << " complete!" << endl;
    }
    else{
        cout << "File upload failed" << endl;
    }

    outFile.close();
}