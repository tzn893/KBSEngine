#pragma once
#include <string>
#include <map>
#include <fstream>

class Config {
public:
	bool Open() {
		std::ifstream fs(config);
		if (!fs.is_open()) return (valid = false);
		std::string line("");
		while (std::getline(fs,line)) {
			size_t ip = line.find_first_of(':');
			values[line.substr(0, ip)] = line.substr(ip + 1);
		}
		fs.close();
		return (valid = true);
	}

	template<typename T>
	T GetValue(const char* name) {
		if (auto res = values.find(name);res != values.end()) {
			try {
				if constexpr (std::is_same<T, std::string>::value) {
					return res->second;
				}
				else if constexpr (std::is_same<T, float>::value) {
					return std::stof(res->second);
				}
				else if constexpr (std::is_same<T, int>::value) {
					return std::stoi(res->second);
				}
				else {
					return T(res->second);
				}
			}
			catch (...) {
				OUTPUT_DEBUG_STRING("invalid config string\n");
				return T();
			}
		}
		else {
			return T();
		}
	}
private:
	const char* config = "config.init";
	std::map<std::string, std::string> values;
	bool valid;
};

inline Config gConfig;