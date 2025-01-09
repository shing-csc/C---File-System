# Variables
CXX = g++
CXXFLAGS = -std=c++11 -pthread

SERVER_DIR = server
CLIENT_DIR = client
BUILD_DIR = makeBuild
TARGETS = $(BUILD_DIR)/server $(BUILD_DIR)/client 


# Rules
all: $(TARGETS)

# EXECUTABLES: under /build 
# - server executable
$(BUILD_DIR)/server: $(SERVER_DIR)/server.o $(SERVER_DIR)/server_upload.o $(SERVER_DIR)/server_directory.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# - client executable
$(BUILD_DIR)/client: $(CLIENT_DIR)/client.o $(CLIENT_DIR)/client_upload.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^


# OBJECT FILES: under /respectiveDir
# - server object files
$(SERVER_DIR)/server.o: $(SERVER_DIR)/server.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(SERVER_DIR)/server_upload.o: $(SERVER_DIR)/server_upload.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(SERVER_DIR)/server_directory.o: $(SERVER_DIR)/server_directory.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# - client object files
$(CLIENT_DIR)/client.o: $(CLIENT_DIR)/client.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(CLIENT_DIR)/client_upload.o: $(CLIENT_DIR)/client_upload.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm-f $(BUILD_DIR) $(SERVER_DIR)/*.o $(CLIENT_DIR)/*.o