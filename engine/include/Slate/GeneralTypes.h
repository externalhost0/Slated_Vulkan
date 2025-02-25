//
// Created by Hayden Rivas on 1/29/25.
//
#pragma once
#include <cstdint>
#include <cstdio>
namespace Slate {
	struct Version {
		uint8_t major = 0;
		uint8_t minor = 0;
		uint8_t patch = 0;

		// comparision operators!
		bool operator==(const Version& other) const {
			return major == other.major && minor == other.minor && patch == other.patch;
		}
		bool operator!=(const Version& other) const {
			return !(*this == other);
		}
		bool operator<(const Version& other) const {
			if (major != other.major) return major < other.major;
			if (minor != other.minor) return minor < other.minor;
			return patch < other.patch;
		}
		bool operator>(const Version& other) const {
			return other < *this;
		}
		bool operator<=(const Version& other) const {
			return !(other < *this);
		}
		bool operator>=(const Version& other) const {
			return !(*this < other);
		}
		// returns the version with . (period) seperating each value
		const char* GetVersionAsString() const {
			static char versionString[12]; // max of 11 char, including null terminator "255.255.255\0"
			int length = snprintf(versionString, sizeof(versionString), "%u.%u.%u", major, minor, patch);
			if (length < 0 || length >= static_cast<int>(sizeof(versionString))) {
				return "Error Getting Version!";
			}
			return versionString;
		};
	};

}