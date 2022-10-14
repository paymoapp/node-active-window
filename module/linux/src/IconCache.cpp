#include "IconCache.h"

namespace PaymoActiveWindow {
	IconCache::IconCache(unsigned int size) {
		this->size = size;
	}

	bool IconCache::has(const std::string* key) {
		const std::lock_guard<std::mutex> lock(this->mutex);

		return this->keysRefs.find(*key) != this->keysRefs.end();
	}

	void IconCache::set(const std::string* key, const std::string* value) {
		const std::lock_guard<std::mutex> lock(this->mutex);

		this->refreshKey(key);
		this->data[*key] = *value;
	}

	std::string IconCache::get(const std::string* key) {
		const std::lock_guard<std::mutex> lock(this->mutex);

		this->refreshKey(key);
		return this->data[*key];
	}

	void IconCache::refreshKey(const std::string* key) {
		if (this->keysRefs.find(*key) == this->keysRefs.end()) {
			// key not prezent in cache
			if (this->keys.size() == this->size) {
				// limit reached, delete LRU element
				this->data.erase(this->keys.back());
				this->keysRefs.erase(this->keys.back());
				this->keys.pop_back();
			}
		} else {
			// remove existing reference to item
			this->keys.erase(this->keysRefs[*key]);
		}

		// add item to front of the list
		this->keys.push_front(*key);
		this->keysRefs[*key] = this->keys.begin();
	}
}
