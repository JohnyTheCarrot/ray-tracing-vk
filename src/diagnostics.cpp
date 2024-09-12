#include "diagnostics.h"

#include <format>
#include <iostream>
#include <string_view>

namespace raytracing {
	void Logger::log(LogLevel level, std::string_view message) {
		/*if (level == LogLevel::Debug)*/
		/*	return;*/

		auto const levelAsString{LogLevelToString(level)};
		auto const completeMessage{std::format("[{}] {}", levelAsString, message)};

		log(completeMessage);
	}

	void Logger::log(std::string_view message) {
		m_File << message << '\n';
		std::cout << message << std::endl;
	}

	void Logger::error(std::string_view message) {
		m_File << message << '\n';
		std::cerr << message << std::endl;
	}

	Logger::Logger() = default;
}// namespace raytracing
