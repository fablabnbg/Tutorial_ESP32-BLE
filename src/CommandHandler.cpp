/*
 * CommandHandler.cpp
 *
 *  Created on: 03.02.2025
 *      Author: ian
 */

#include <CommandHandler.h>

SimpleCLI cli;
CommandHandler cmdHandler;

void cmdCB(cmd *c) {
	const Command cmd(c); // Create wrapper object
	Serial.printf("CMD: %s\n", cmd.getName());
	cmdHandler.handleCommand(cmd);
}

CommandHandler::CommandHandler() {

}

void CommandHandler::setup() {
	cli.setErrorCallback([](cmd_error* e) {
		CommandError cmdError(e);
	    if (cmdError.hasCommand()) {
	        Serial.print("Wrong parameters. Help: ");
	        Serial.println(cmdError.getCommand().toString());
	    }});
    cmdPing = cli.addCmd("ping", [](cmd* c) {Serial.println("Pong!");});
    cmdPing.setDescription("Ping. Antwortet mit Pong!");

    cmdHelp = cli.addCmd("help", cmdCB);


}

void CommandHandler::handleCommand(const Command &cmd) {
//	if (cmd.equals(logShow)) {
//		printLoglevels();
//		return;
//	}
	cli.toString(false);
	cli.toString(true);
	Serial.println("End handleCommand");

}

void CommandHandler::loop() {
	if (Serial.available()) {
		bool lineComplete = false;
		while (Serial.available()) {
			char inChar = Serial.read();
			if (inChar == 255) continue;
			if (inChar == '\n') lineComplete = true;
			inputBuffer += inChar;
			Serial.print(inChar);		// Echo
		}
		if (lineComplete) {
			cli.parse(inputBuffer);
			inputBuffer.clear();
		}
	}
}
