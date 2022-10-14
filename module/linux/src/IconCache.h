#include <unordered_map>
#include <list>
#include <mutex>
#include <string>

#ifndef _PAYMO_ICONCACHE_H
#define _PAYMO_ICONCACHE_H

namespace PaymoActiveWindow {
	class IconCache {
	public:
		IconCache(unsigned int size = 0);
		bool has(const std::string* key);
		void set(const std::string* key, const std::string* value);
		std::string get(const std::string* key);
	private:
		void refreshKey(const std::string* key);
		unsigned int size;
		std::list<std::string> keys;
		std::unordered_map<std::string, std::list<std::string>::iterator> keysRefs;
		std::unordered_map<std::string, std::string> data;
		std::mutex mutex;
	};
}

#endif
