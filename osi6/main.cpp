#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <optional>

enum {
	SUCCESS,
	ERR,
	EXIT
};

struct Client {
	int socket;
	std::string ip;
	int port;

	Client() {
		ip.resize(INET_ADDRSTRLEN);
	}
};

std::string sum(const std::string& summands) {
	std::istringstream sstream(summands);
	double a, b;
	sstream >> a >> b;
	return std::to_string(a + b);
}

int get_server_ips() {
	std::cout << "Server IPs: \n\n";

	ifaddrs* interface_addrs = nullptr;

	if (getifaddrs(&interface_addrs) == -1) {
		perror("getifaddrs");
		return -1;
	}

	for (ifaddrs* interface_addr = interface_addrs; interface_addr != nullptr;
		interface_addr = interface_addr->ifa_next) {
		if (interface_addr->ifa_addr == nullptr) continue;

		if (interface_addr->ifa_addr->sa_family == AF_INET) { // IPv4
			void* addr = &((sockaddr_in*)interface_addr->ifa_addr)->sin_addr;
			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, addr, ip, sizeof(ip));

			std::cout << interface_addr->ifa_name << " - " << ip << std::endl;
		}
	}

	std::string external_ip = "217.71.129.139";
	std::cout << "extetrnal: " << external_ip;

	freeifaddrs(interface_addrs);
	
	std::cout << "\n\n";

	return 0;
}

int input_port() {
	std::cout << "Port: ";
	int port = 2011;
	std::cin >> port;
	std::cout << "\n";

	return port;
}

std::optional<int> get_server_socket(const int port) {
	int ret_val;

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	std::cout << "Creating the server socket" << std::endl;

	int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket < 0) {
		perror("socket");
		return std::nullopt;
	}

	std::cout << "Binding the server socket" << std::endl;

	ret_val = bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));

	if (ret_val < 0) {
		perror("bind");
		close(server_socket);
		return std::nullopt;
	}

	return server_socket;
}

int serve_client(const Client& client) {
	int ret_val = 0;

	while (true) {
		std::cout << "Receiving client data" << std::endl;

		std::string summands;
		summands.resize(1024);

		ret_val = recv(client.socket, (char*)summands.data(), summands.size(), 0);

		if (ret_val == 0) {
			std::cout << "Client " << client.ip << ":" << client.port << " has disconnected\n";
			close(client.socket);
			return EXIT;
		}

		if (ret_val < 0) {
			perror("recv");
			close(client.socket);
			return ERR;
		}

		summands.resize(ret_val);

		std::cout << "Sending results" << std::endl;

		std::string res;

		res = sum(summands);

		ret_val = send(client.socket, res.data(), res.size(), 0);

		if (ret_val < 0) {
			std::cout << "Sending failed\n";
			close(client.socket);
			return ERR;
		}
	}

	return SUCCESS;
}

std::optional<Client> accept_client(const int server_socket) {
	std::cout << "Accepting client\n";

	Client client;

	sockaddr_in client_sockaddr;
	socklen_t client_addr_size = sizeof(client_sockaddr);

	client.socket = accept(server_socket, (sockaddr*)&client_sockaddr, &client_addr_size);

	if (client.socket < 0) {
		perror("accept");
		close(server_socket);
		return std::nullopt;
	}

	inet_ntop(AF_INET, &client_sockaddr.sin_addr, client.ip.data(), INET_ADDRSTRLEN);
	client.port = ntohs(client_sockaddr.sin_port);

	std::cout << "Client " << client.ip << ":" << client.port << " has connected\n";

	return client;
}

int listen_server_socket(const int server_socket) {
	int ret_val = 0;

	const int clients_count = 1;

	std::cout << "Listening to the server socket" << std::endl;

	ret_val = listen(server_socket, clients_count);

	if (ret_val < 0) {
		perror("listen");
		close(server_socket);
		return ERR;
	}

	return SUCCESS;
}

int server() {
	int port = input_port();

	if (get_server_ips() == ERR)
		return ERR;

	auto server_socket = get_server_socket(port);
	if (!server_socket.has_value())
		return ERR;

	while (true) {
		if (listen_server_socket(*server_socket) == ERR)
			return ERR;

		auto client = accept_client(*server_socket);

		if (!client.has_value())
			continue;

		if (serve_client(*client) == ERR)
			return ERR;
	}

	return SUCCESS;
}

int main() {
	if (server() == ERR)
		return ERR;

	return SUCCESS;
}