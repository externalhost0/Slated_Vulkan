//
// Created by Hayden Rivas on 6/2/25.
//

#pragma once

#include "Slate/SmartPointers.h"
#include "Slate/Common/Logger.h"

#include <sys/socket.h>
#include <vector>


namespace google::protobuf {
	class Message;
}

namespace Slate {
	class SocketConnection {
	public:
		explicit SocketConnection(int fd);
		~SocketConnection();

		SocketConnection(const SocketConnection&) = delete;
		SocketConnection& operator=(const SocketConnection&) = delete;

		ssize_t send(const void* data, size_t size) const;
		ssize_t recv(void* buffer, size_t size) const;

		template<class ProtoStruct>
		ssize_t send(const ProtoStruct& type) const {
			return send(&type, sizeof(ProtoStruct));
		}
		template<class ProtoStruct>
		ssize_t recv(const ProtoStruct& type) const {
			return recv(&type, sizeof(ProtoStruct));
		}
		template<class ProtoStruct>
		void fullProtoSend(const ProtoStruct& type) const {
			static_assert(std::is_base_of_v<::google::protobuf::Message, ProtoStruct>, "T must inherit from IResource");
			size_t size = type.ByteSizeLong();
			std::vector<std::byte> buffer(size);

			type.SerializeToArray(buffer.data(), (int)size);
			uint32_t len = htonl(size);

			send(&len, sizeof(uint32_t));
			send(buffer.data(), buffer.size());
		}
	private:
		int _fd;
	};

	class ServerSocket {
	public:
		ServerSocket();
		~ServerSocket();

		bool bind(const char* path);
		SocketConnection acceptClient();
	private:
		struct Impl;
		UniquePtr<Impl> _impl;
	};
	class ClientSocket {
	public:
		ClientSocket();
		~ClientSocket();

		bool connect(const char* path);
		SocketConnection getConnection();
	private:
		struct Impl;
		UniquePtr<Impl> _impl;
	};
}