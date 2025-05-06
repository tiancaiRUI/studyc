// CMakeProject1.cpp: 定义应用程序的入口点。
//

#include "CMakeProject1.h"

template<typename T>
std::string to_string_helper(T&& arg) {
	std::ostringstream oss;
	// 万能引用 接原样转发
	oss << std::forward<T>(arg);
	return oss.str();
}

class Logque {
public:
	void push(const std::string &msg) {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(msg);
		cond_var_.notify_one();
	}
	bool pop(std::string& msg) {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_var_.wait(lock, [this]() {
			return !queue_.empty() || is_shutdown_;});
		if (is_shutdown_ && queue_.empty()) {
			return false;
		}
		msg = queue_.front();
		queue_.pop();
		return true;
	}
	void shutdown() {
		std::lock_guard<std::mutex> lock(mutex_);
		is_shutdown_ = true;
		cond_var_.notify_all();
	}
private:
	std::queue<std::string> queue_;
	std::mutex mutex_;
	std::condition_variable cond_var_;
	bool is_shutdown_ = false;

	
};


class Logger {
public:
	Logger(const std::string& filename):log_file(filename,std::ios::out|std::ios::app),exit_flag_(false) {
		if (!log_file.is_open()) {
			throw std::runtime_error("can not open");
		}
		work_thread = std::thread(&Logger::processQueue, this);

	};
	~Logger() {
		log_queue_.shutdown();
		if (work_thread.joinable()) {
			work_thread.join();
		}
		if (log_file.is_open()) {
			log_file.close();
		}
	};
	template<typename... Args>
	void log(const std::string& format, Args&& ...args) {
		log_queue_.push(formatMessage(format, std::forward<Args>(args)...));
	}
private:
	Logque log_queue_;
	std::thread work_thread;
	std::ofstream log_file;
	std::atomic<bool> exit_flag_;

	void processQueue() {
		std::string msg;
		while (log_queue_.pop(msg)) {
			log_file << msg << std::endl;
		}
	}

	template<typename... Args>
	std::string formatMessage(const std::string& format, Args&&... args) {
		std::vector<std::string> arg_strings = { to_string_helper(std::forward<Args>(args))... };
		std::ostringstream oss;
		size_t arg_index = 0;
		size_t pos = 0;
		size_t placeholder = format.find("{}", pos);
		while (placeholder != std::string::npos) {
			oss << format.substr(pos, placeholder - pos);
			if (arg_index < arg_strings.size())
			{
				oss << arg_strings[arg_index++];
			}
			else {
				oss << "{}";
			}
			pos = placeholder + 2;
			placeholder = format.find("{}", pos);
		}
		oss << format.substr(pos);
		while (arg_index < arg_strings.size()) {
			oss << arg_strings[arg_index++];

		}
		return oss.str();
	}
};
int main()
{
	std::cout << "Hello CMake." << std::endl;
	Logger logger("log.txt");
	std::cout << "open" << std::endl;
	logger.log("Starting application.");

	int user_id = 42;
	std::string action = "login";
	double duration = 3.5;
	std::string world = "World";

	logger.log("User {} performed {} in {} seconds.", user_id, action, duration);
	logger.log("Hello {}", world);
	logger.log("This is a message without placeholders.");
	logger.log("Multiple placeholders: {}, {}, {}.", 1, 2, 3);
	return 0;
}
