// CMakeProject1.cpp: 定义应用程序的入口点。
//

#include "CMakeProject1.h"

template<typename T>
std::string to_string_helper(T&& arg) {
	std::ostringstream oss;
	// 万能引用 接原样转发
	oss << std::forward<T>(args);
	return oss.str();
}

class Logque {
public:
	void push();
	bool pop();
	void shutdown();
private:
	std::queue<std::string> queue_;
	std::mutex mutex_;
	std::condition_variable cond_var_;
	bool is_shutdown_ = false;

};

int main()
{
	std::cout << "Hello CMake." << std::endl;
	return 0;
}
