//
// Created by Hayden Rivas on 6/2/25.
//
#if defined(SLATE_OS_MACOS)
#include "Slate/Network/Socket.h"
#include "Slate/Common/Logger.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace Slate {

	SocketConnection::SocketConnection(int fd) : _fd(fd) {
		if (_fd < 0) throw std::runtime_error("Invalid socket file descriptor.");
	}
	SocketConnection::~SocketConnection() {
		if (_fd >= 0) close(_fd);
	}

	// generic send and recieve functions
	ssize_t SocketConnection::send(const void* data, size_t size) const {
		ssize_t result = ::send(_fd, data, size, 0);
		if (result == -1) {
			LOG_USER(LogType::Warning, "Send encountered an error: (errno {}) : {}", errno, std::strerror(errno));
		}
		return result;
	}
	ssize_t SocketConnection::recv(void* buffer, size_t size) const {
		ssize_t result = ::recv(_fd, buffer, size, 0);
		if (result == -1) {
			LOG_USER(LogType::Warning, "Recieve encountered an error: (errno {}) : {}", errno, std::strerror(errno));
		}
		return result;
	}

	struct ServerSocket::Impl {
		int _serverFd = -1;
		const char* _path = nullptr;
	};
	ServerSocket::ServerSocket() {
		_impl = CreateUniquePtr<ServerSocket::Impl>();
	}
	ServerSocket::~ServerSocket() {
		if (_impl->_serverFd != -1) {
			close(_impl->_serverFd);
		}
		if (_impl->_path) {
			unlink(_impl->_path);
		}
	}
	bool ServerSocket::bind(const char* path) {
		_impl->_path = strdup(path);
		// adress families, AF_UNIX for local communication as we will use it
		int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		// SOCK_STREAM = TCP
		// SOCK_DGRAM = UDP
		// SOCK_SEQPACKET = SCTP
		if (sockfd == -1) {
			LOG_USER(LogType::Warning, "Failed to create server socket : (errno {}) {}", path, errno, std::strerror(errno));
			return false;
		}
		_impl->_serverFd = sockfd;

		// fill in our unix domain socket info
		sockaddr_un server_addr = {};
		server_addr.sun_family = AF_UNIX;
		std::strncpy(server_addr.sun_path, path, sizeof(server_addr.sun_path) - 1);
		// good practice, removes old socket in prep for bind
		unlink(path);

		// bind and then begin listening
		if (::bind(_impl->_serverFd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
			LOG_USER(LogType::Warning, "Failed to bind to socket path: '{}' : (errno {}) {}", path, errno, std::strerror(errno));
			return false;
		}
		if (listen(_impl->_serverFd, 1) == -1) {
			LOG_USER(LogType::Warning, "Failed to listen on server socket: {}", std::strerror(errno));
			return false;
		}

		LOG_USER(LogType::Info, "Server ready and listening on '{}'", path);
		return true;
	}
	SocketConnection ServerSocket::acceptClient() {
		LOG_USER(LogType::Info, "Waiting for client...");
		int clientFd = accept(_impl->_serverFd, nullptr, nullptr);
		if (clientFd == -1) {
			LOG_USER(LogType::Warning, "Failed to accept client on {}, (errno {}) {}", _impl->_path, errno, std::strerror(errno));
		}
		LOG_USER(LogType::Info, "Client connected!");
		return SocketConnection(clientFd);
	}

	struct ClientSocket::Impl {
		int _clientFd = -1;
	};
	ClientSocket::ClientSocket() {
		_impl = CreateUniquePtr<ClientSocket::Impl>();
	}
	ClientSocket::~ClientSocket() {
		if (_impl->_clientFd == -1) {
			close(_impl->_clientFd);
		}
	}
	bool ClientSocket::connect(const char* path) {
		int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (sockfd == -1) {
			LOG_USER(LogType::Warning, "Failed to create client socket : (errno {}) : {}", errno, std::strerror(errno));
			return false;
		}
		_impl->_clientFd = sockfd;

		sockaddr_un client_addr = {};
		client_addr.sun_family = AF_UNIX;
		std::strncpy(client_addr.sun_path, path, sizeof(client_addr.sun_path) - 1);

		if (::connect(sockfd, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr)) == -1) {
			LOG_USER(LogType::Warning, "Failed to connect to socket path: '{}' : (errno {}) {}", path, errno, std::strerror(errno));
			close(sockfd);
			_impl->_clientFd = -1;
			return false;
		}
		LOG_USER(LogType::Info, "Successfully connected to server socket '{}'", path);
		return true;
	}
	SocketConnection ClientSocket::getConnection() {
		if (_impl->_clientFd == -1) {
			LOG_USER(LogType::Warning, "Client fd invalid, connection will be unstable/invalid!");
		}
		return SocketConnection(_impl->_clientFd);
	}
}

#endif