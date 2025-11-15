#ifndef COMMAND_H
#define COMMAND_H

#include <QString>

class Command {
public:
	virtual ~Command() = default;
	virtual void execute() = 0;
	virtual void undo() = 0;
};
//设置虚函数execute,undo
#endif // COMMAND_H

