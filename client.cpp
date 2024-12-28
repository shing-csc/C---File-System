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
const int PORT = 8080;
const int BUFFER_SIZE = 1024;



int main(){

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;


    // Clientsocket establish connection to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        cerr << "Unable to connect to server" << endl;
        close(clientSocket);
        return -1;
    }

    string filename; 
    cout << "Enter the filename of the file to upload: " << endl;
    cin >> filename;


    if (send(clientSocket, filename.c_str(), filename.size(), 0) == -1) {
        cerr << "Failed to send filename to server." << endl;
        close(clientSocket);
        return -5;
    }

    string directory = "clientfiles/";
    string fullFilePath = directory + filename;

    ifstream inFile(fullFilePath, ios::binary);
    if (!inFile.is_open()){
        cerr << "Failed to open file " << filename << endl;
        close(clientSocket);
        return -2;
    }

    char buffer[BUFFER_SIZE] = {0};

    // Obtain data from ifstream, load it to buffer and send to the client socket 
    while(inFile.read(buffer, sizeof(buffer))){

        if (send(clientSocket, buffer, sizeof(buffer), 0) == -1){
            cerr << "Failed to send file data " << endl;
            close(clientSocket);
            return -3;
        }
    }

    if (inFile.gcount() > 0){ // gcount = number of characters atually read by the ifstream

        if (send(clientSocket, buffer, inFile.gcount(), 0) == -1){
            cerr << "Failed to send last chuck of file data " << endl;
            close(clientSocket);
            return -4;
        }
    }

    cout << "File upload complete!" << endl;
    inFile.close();
    close(clientSocket);
    return 0;
}