//
// Created by Hayden Rivas on 5/27/25.
//

#pragma once

#include <iostream>
#include <string>

namespace Slate {
	class ISerializble {
	public:
		virtual ~ISerializble() = default;

		virtual void Serialize(std::ostream& out) const = 0;
		virtual void Deserialize(std::istream& in) const = 0;
	};
}