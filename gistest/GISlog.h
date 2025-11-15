#ifndef GISLOG_H
#define GISLOG_H

#include <log4cpp/Category.hh>
#include <string>

class Logger
{
public:
	static Logger& getInstance();

	void logFileOpenError(const std::string& filename);
	void logUserInputError(const std::string& input);
	void logInput(const std::string& input);
	void logFileOpenSuccess(const std::string& filename);
	void logUserInputSuccess(const std::string& input);
	void logUserInputWarning(const std::string& input);
	void logUserInputWarning(const double input);
	void logFunctionSuccess(const std::string& functionName);
	void logFunctionFail(const std::string& functionName);
	void logFunctionWarning(const std::string& functionName);
	void logFunctionUndo(const std::string& functionName);
	void logFunctionRedo(const std::string& functionName);

private:
	Logger();
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
};

#endif // GISLOG_H