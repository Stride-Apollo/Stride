#pragma once

#include <future>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace stride {
namespace util {

using namespace std;

// partly based on http://stackoverflow.com/a/14921115/2678118
// Implements a simple boolean flag
class Flag {
public:
	inline bool isSet() const {
		lock_guard<mutex> lock(m_mut);
		return m_flag;
	}

	inline void set() {
		{
			lock_guard<mutex> lock(m_mut);
			m_flag = true;
		}
		m_cv.notify_all();
	}

	inline void reset() {
		{
			lock_guard<mutex> lock(m_mut);
			m_flag = false;
		}
		m_cv.notify_all();
	}

	inline void wait() const {
		unique_lock<mutex> lock(m_mut);
		if (m_flag) return;
		m_cv.wait(lock, [this]{ return m_flag; });
	}

private:
	mutable mutex m_mut;
	mutable condition_variable m_cv;
	bool m_flag = false;
};


template<typename T>
vector<T> future_pool(vector<future<T>>& futures) {
	vector<T> results;
	mutex results_lock;
	Flag all_done;

	for (auto& f: futures) {
		async(std::launch::async, [&]() {
			T res = f.get();
			{
				lock_guard<mutex> lock(results_lock);
				results.push_back(res);
				if (results.size() == futures.size())
					all_done.set();
			}
		});
	}

	all_done.wait();
	return results;
}

}
}
