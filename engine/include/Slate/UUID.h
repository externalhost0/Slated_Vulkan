//
// Created by Hayden Rivas on 3/30/25.
//

#pragma once
#include <cstdint>
#include <string>
#include <random>

namespace Slate {
	static uint64_t generate() {
		static std::mt19937_64 rng(std::random_device{}());
		static std::uniform_int_distribution<uint64_t> dist;
		return dist(rng);
	}

	class UUID {
	public:
		UUID() : _id(generate()) {}
		explicit UUID(uint64_t val) : _id(val) {}

		uint64_t toValue() const { return _id; }
		std::string toString() const { return std::to_string(_id); }
		bool operator==(const UUID& other) const { return _id == other._id; }
		bool operator!=(const UUID& other) const { return _id != other._id; }
		bool operator<(const UUID& other) const { return _id < other._id; }
	private:
		uint64_t _id;
	};
}
namespace std {
	template<>
	struct hash<Slate::UUID> {
		size_t operator()(const Slate::UUID& uuid) const {
			return std::hash<uint64_t>{}(uuid.toValue());
		}
	};
}