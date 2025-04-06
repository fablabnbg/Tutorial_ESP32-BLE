/*
 * CommandHandler.h
 *
 *  Created on: 03.02.2025
 *      Author: ian
 */

#pragma once

#include <SimpleCLI.h>
extern SimpleCLI cli;

class CommandHandler {
public:
	CommandHandler();
	void handleCommand(const Command& cmd);
	void setup();
	void loop();

private:
	Command cmdPing;
	Command cmdHelp;
	String inputBuffer;
};

