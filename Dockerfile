# Use Ubuntu as the base image
FROM ubuntu:20.04

# Set non-interactive mode for apt (to avoid prompts during installation)
ENV DEBIAN_FRONTEND=noninteractive

# Install required dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    git \
    && apt-get clean

# Set the working directory inside the container
WORKDIR /app

# Copy the project files into the container
COPY . .

# Configure the project with CMake
RUN rm -rf build && mkdir build && cd build && cmake ..

# Build the project
RUN cd build && cmake --build .

# Expose the port for the server (adjust this if your server uses a different port)
EXPOSE 8080

# Default command to run a shell (so you can interact with the container)
CMD ["bash"]