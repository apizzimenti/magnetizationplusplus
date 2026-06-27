
#ifndef EXPERIMENT_METADATA
#define EXPERIMENT_METADATA

#include <string>
#include <cstdio>
#include <format>
#include <fstream>
#include <chrono>
#include <map>
#include <filesystem>

// Source - https://stackoverflow.com/a/78276118
// Posted by idbrii
// Retrieved 2026-06-24, License - CC BY-SA 4.0

#include <cstdio>

#if defined(__GNUC__)
	#if defined(__clang__)
		#define COMPILER_FMT "Clang"
	#else
		#define COMPILER_FMT "GNUC"
	#endif
	#define COMPILER_VERSION __VERSION__
#elif defined(_MSC_VER)
	#define COMPILER_FMT "MSVC"
	#define COMPILER_VERSION _MSC_FULL_VER
#else
	#error Please add your compiler here.
#endif

// Source - https://stackoverflow.com/a/15580211
// Posted by masoud, modified by community. See post 'Timeline' for change history
// Retrieved 2026-06-24, License - CC BY-SA 4.0

std::string getOsName()
{
	#ifdef _WIN32
		return "Windows 32-bit";
	#elif _WIN64
		return "Windows 64-bit";
	#elif __APPLE__ || __MACH__
		return "macOS";
	#elif __linux__
		return "Linux";
	#elif __FreeBSD__
		return "FreeBSD";
	#elif __unix || __unix__
		return "Unix";
	#else
		return "Other";
	#endif
}


auto ttcDays(std::chrono::duration<double> duration) {
	const auto days = std::chrono::duration_cast<std::chrono::days>(duration);
	const auto hours = std::chrono::duration_cast<std::chrono::hours>(duration-days);
	const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration-days-hours);
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration-days-hours-minutes);

	return std::make_tuple(days, hours, minutes, seconds);
}


std::string estimatedTTC(std::chrono::time_point<std::chrono::system_clock> start, int t, int N) {
	auto now = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<double> _elapsed{now-start};
	const auto [elapseddays, elapsedhours, elapsedminutes, elapsedseconds] = ttcDays(_elapsed);

	const std::chrono::duration<double> _estimated{((now-start)/t)*N};
	const auto [estimateddays, estimatedhours, estimatedminutes, estimatedseconds] = ttcDays(_estimated);

	std::string elapsed = std::format("{}d:{:02}h:{:02}m:{:02}s", elapseddays.count(), elapsedhours.count(), elapsedminutes.count(), elapsedseconds.count());
	std::string estimated = std::format("{}d:{:02}h:{:02}m:{:02}s", estimateddays.count(), estimatedhours.count(), estimatedminutes.count(), estimatedseconds.count());	

	return std::format("{}/{}", t, N) + " ———— " + elapsed + " ———— " + estimated + "\r";
}


struct METADATA {
	std::string TIMESTAMP;
	std::string LOCALIZED;
	std::string TTC;
	std::string BUILD = std::format("{} {}, C++{}, {}", COMPILER_FMT, COMPILER_VERSION, __cplusplus, getOsName());
	std::string MODEL;
	std::map<std::string,double> PARAMETERS;


	inline void ttc(auto start, auto end) {
		const std::chrono::duration<double> elapsed{end-start};
		TTC = std::format("{}d:{:%Hh:%Mm:%Ss}", std::chrono::duration_cast<std::chrono::days>(elapsed).count(), elapsed);
	}

	inline void localize(std::string TEST) {
		std::cerr << "Testing; marking at system clock time, saving to TEST." << std::endl;

		auto stamp = std::chrono::high_resolution_clock::now();
		LOCALIZED = std::format(
			"{:%Y/%m/%d %H:%M}",
			std::chrono::current_zone()->to_local(stamp)
		);
	}

	inline void localize(int epochtime) {
		try {
			const std::chrono::system_clock::time_point stamp(std::chrono::seconds{epochtime});
			LOCALIZED = std::format(
				"{:%Y/%m/%d %H:%M}",
				std::chrono::current_zone()->to_local(stamp)
			);
		} catch (...) {
			std::cerr << "Couldn't interpret given timestamp; reverting to system clock time." << std::endl;
			auto stamp = std::chrono::high_resolution_clock::now();
			LOCALIZED = std::format(
				"{:%Y/%m/%d %H:%M}",
				std::chrono::current_zone()->to_local(stamp)
			);
		}
	}

	inline std::string parametrize() {
		std::string parameters = "\t\"PARAMETERS\": {\n";
		std::string prefix = "\t\t";
		int k = 0;

		for (auto [parameter,value] : PARAMETERS) {
			parameters += prefix + std::format("\"{}\": \"{}\"{}", parameter, value, (k==PARAMETERS.size()-1 ? "" : ",")) + "\n";
			k++;
		}

		parameters += "\t}";

		return parameters;
	}

	inline void write(std::string destination) {
		std::ofstream out;
		std::filesystem::path writable(destination);

		if (!std::filesystem::exists(writable.parent_path())) {
			std::cerr << std::format("Couldn't find filepath {}, forcibly writing...", writable.parent_path().string()) << std::endl;
			std::filesystem::create_directory(writable.parent_path());
		}

		out.open(destination);
		std::string quote = "\"";

		out << "{" << std::endl;
		out << "\t\"TIMESTAMP\": " << quote << TIMESTAMP << quote << "," << std::endl;
		out << "\t\"LOCALIZED\": " << quote << LOCALIZED << quote << "," << std::endl;
		out << "\t\"TTC\": " << quote << TTC << quote << "," << std::endl;
		out << "\t\"BUILD\": " << quote << BUILD << quote << "," << std::endl;
		out << "\t\"MODEL\": " << quote << MODEL << quote << "," << std::endl;
		out << parametrize() << std::endl;
		out << "}" << std::endl;

		out.close();
	}
};


#endif
