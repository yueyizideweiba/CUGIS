#include "GISlog.h"
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>
#include <string>
#include <iostream>

Logger& Logger::getInstance()
{
	static Logger instance;
	return instance;
}

void Logger::logFileOpenError(const std::string& filename)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.error("Failed to open file: " + filename);
}

void Logger::logUserInputError(const std::string& input)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.warn("User input error: " + input);
}

void Logger::logInput(const std::string& input)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info(input);
}

void Logger::logFileOpenSuccess(const std::string& filename)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("Success to open file: " + filename);
}

void Logger::logUserInputSuccess(const std::string& input)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("User input success: " + input);
}

void Logger::logFunctionSuccess(const std::string& functionName)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("Success to operate: " + functionName);
}

void Logger::logFunctionFail(const std::string& functionName)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.error("Fail to operate: " + functionName);
}


void Logger::logUserInputWarning(const std::string& input)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("User input success: " + input);
}

void Logger::logUserInputWarning(const double input)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("User input success: " + std::to_string(input));
}

void Logger::logFunctionWarning(const std::string& functionName)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("function warning ：");
}


void Logger::logFunctionUndo(const std::string& functionName)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("function undo ：");
}

void Logger::logFunctionRedo(const std::string& functionName)
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.info("function redo ：");
}

Logger::Logger()
{
	log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
	layout->setConversionPattern("%d [%p] %m%n");

	log4cpp::FileAppender* fileAppender = new log4cpp::FileAppender("FileAppender", "log.txt");
	fileAppender->setLayout(layout);

	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.setAppender(fileAppender);
	root.setPriority(log4cpp::Priority::DEBUG);
}