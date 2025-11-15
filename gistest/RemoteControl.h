#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <memory>
#include <stack>
#include "Command.h"

class RemoteControl {
public:
	void setCommand(std::unique_ptr<Command> command) {
		// 清空重做栈，因为新的命令的执行打断了重做的历史
		while (!redoStack_.empty()) {
			redoStack_.pop();
		}
		command_ = std::move(command);
	}

	void pressExecute() {
		if (command_) {
			command_->execute();
			commandHistory_.push(std::move(command_));
			command_ = nullptr;  // 执行后重置当前命令
		}
	}

	void pressUndo() {
		if (!commandHistory_.empty()) {
			std::unique_ptr<Command> lastCommand = std::move(commandHistory_.top());
			commandHistory_.pop();
			lastCommand->undo();
			redoStack_.push(std::move(lastCommand));
		}
	}

	void pressRedo() {
		if (!redoStack_.empty()) {
			std::unique_ptr<Command> redoCommand = std::move(redoStack_.top());
			redoStack_.pop();
			redoCommand->execute();
			commandHistory_.push(std::move(redoCommand));
		}
	}

private:
	std::unique_ptr<Command> command_;
	std::stack<std::unique_ptr<Command>> commandHistory_;
	std::stack<std::unique_ptr<Command>> redoStack_;
};

#endif