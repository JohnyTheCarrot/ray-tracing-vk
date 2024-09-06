#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include "Singleton.h"

#include <fstream>
#include <string_view>

namespace raytracing {
	enum class LogLevel { Info, Debug, Warning, Error };

	constexpr std::string_view LogLevelToString(LogLevel level) {
		switch (level) {
			case LogLevel::Info:
				return "INFO";
			case LogLevel::Debug:
				return "DEBUG";
			case LogLevel::Warning:
				return "WARNING";
			case LogLevel::Error:
				return "ERROR";
		}

		return "UNKNOWN";
	}

	class Logger final : public engine::Singleton<Logger> {
		std::ofstream m_File{"log.txt"};

	public:
		void log(LogLevel level, std::string_view message);

		void log(std::string_view message);

		Logger();
	};
}// namespace raytracing

#endif//DIAGNOSTICS_H
