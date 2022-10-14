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
		bool has(const std::wstring* key);
		void set(const std::wstring* key, const std::string* value);
		std::string get(const std::wstring* key);
	private:
		void refreshKey(const std::wstring* key);
		unsigned int size;
		std::list<std::wstring> keys;
		std::unordered_map<std::wstring, std::list<std::wstring>::iterator> keysRefs;
		std::unordered_map<std::wstring, std::string> data;
		std::mutex mutex;
	};
}

#endif
