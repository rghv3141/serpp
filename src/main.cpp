#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <unistd.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;

int main() {
	int serverSoc = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSoc == -1) {
		std::cerr << "Failed to create a socket" << '\n';
		return 1;
	}

	sockaddr_in serverAddress;
	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080); 
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if(bind(serverSoc, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		std::cerr << "failed to bind socket" << '\n';
		return 1;
	}

	if(listen(serverSoc, 5) == -1) {
		std::cerr << "failed to listen on socket" << '\n';
		return 1;
	}

	while(true) {
		sockaddr_in clientAddress;
		socklen_t client_add_len = sizeof(clientAddress);
		int clientSoc = accept(serverSoc, (struct sockaddr*)&clientAddress, &client_add_len);
		if(clientSoc == -1) {
			std::cerr << "failed to accept connection" << '\n';
			return 1;
		}

		std::cout << "accepted connection from" << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << '\n';

		std::array<char, BUFFER_SIZE> buffer;
		buffer.fill(0);
		ssize_t bytes_recv = recv(clientSoc, buffer.data(), buffer.size() - 1, 0);
		if (bytes_recv == -1) {
            std::cerr << "Failed to receive request" << std::endl;
            close(clientSoc);
            continue;
        }

		//parse the request
		std::string request(buffer.data(), bytes_recv);
		std::size_t first_space = request.find(' ');
		std::string method = request.substr(0, first_space);
		std::size_t sec_space = request.find(' ', first_space + 1);
		std::string path = request.substr(first_space + 1, sec_space - first_space - 1);
		
		std::cout << method << path << '\n';

		if(method == "GET") {
			std::string file_path = "." + path;
			std::cout << path << '\n'; 
			std::ifstream file(file_path + std::string("txt.html"), std::ios::binary);
			if(file) {
				if (file) {
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                response += std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                send(clientSoc, response.c_str(), response.length(), 0);
				} else {
					std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
					send(clientSoc, response.c_str(), response.length(), 0);
				}
			}	
		} else {
				std::string response = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>";
				send(clientSoc, response.c_str(), response.length(), 0);
		}

		close(clientSoc);

	}
	close(serverSoc);

	return 0;
}
