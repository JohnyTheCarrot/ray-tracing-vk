#include "diagnostics.h"

#include <format>
#include <iostream>
#include <string_view>

namespace raytracing {
	void Logger::Log(LogLevel level, std::string_view message) {
		auto const levelAsString{LogLevelToString(level)};
		auto const completeMessage{std::format("[{}] {}", levelAsString, message)};

		Log(message);
	}

	void Logger::Log(std::string_view message) {
		m_File << message << '\n';
		std::cout << message << std::endl;
	}

	Logger::Logger() = default;
}// namespace raytracing
