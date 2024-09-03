#include "diagnostics.h"

#include <format>
#include <fstream>
#include <iostream>

namespace raytracing {
	void Logger::Log(LogLevel level, std::string_view message) {
		const auto completeMessage{std::format("[{}] {}", LogLevelToString(level), message)};

		m_File << completeMessage << '\n';

		if (level == LogLevel::Error) {
			std::cerr << completeMessage << std::endl;
		} else {
			std::cout << completeMessage << std::endl;
		}
	}

	Logger::Logger() = default;
}// namespace raytracing
