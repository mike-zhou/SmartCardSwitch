/*
 * consoleInput.h
 *
 *  Created on: Dec 20, 2018
 *      Author: mikez
 */

#ifndef CONSOLEINPUT_H_
#define CONSOLEINPUT_H_

#include <deque>
#include <iostream>

#include "Poco/Task.h"

#include "CommandFactory.h"
#include "CommandRunner.h"
#include "Logger.h"

extern Logger * pLogger;

class ConsoleInput: public Poco::Task
{
public:
	ConsoleInput(CommandRunner * pCmdRunner): Task("ConsoleInput") { _pCmdRunner = pCmdRunner; }
	virtual ~ConsoleInput() { }

private:
	CommandRunner * _pCmdRunner;
	std::deque<char> _input;

	std::string getConsoleCommand()
	{
		std::string command;

		if(_input.size() < 1) {
			return command;
		}

		bool bCommandAvailable = false;

		//find '\n' in the input
		for(auto it=_input.begin(); it!=_input.end(); it++)
		{
			if(*it == '\n') {
				bCommandAvailable = true;
				break;
			}
		}
		if(!bCommandAvailable) {
			return command;
		}
		//retrieve command from beginning of _input
		for(;;)
		{
			auto c = _input.front();
			_input.pop_front();
			if(c == '\n') {
				break;
			}
			command.push_back(c);
		}

		//validate command characters
		bool bCmdValid = true;
		for(auto it=command.begin(); it!=command.end(); it++)
		{
			if((*it != ' ') && ((*it < '0') || (*it > '9'))) {
				bCmdValid = false;
				break;
			}
		}
		if(bCmdValid == false) {
			pLogger->LogError("ConsoleInput::processInput invalid command: " + command);
			std::cout << CommandFactory::Help();
			command.clear();
		}

		return command;
	}

	void runTask()
	{
		while(1)
		{
			if(isCancelled())
			{
				break;
			}
			else
			{
				std::string cmd;

				char c = getchar();
				_input.push_back(c);

				cmd = getConsoleCommand();

				if(!cmd.empty())
				{
					//parse command
					int d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
					bool errorOccured = false;
					std::string json;

					d0 = -1;
					d1 = -1;
					d2 = -1;
					d3 = -1;
					d4 = -1;
					d5 = -1;
					d6 = -1;
					d7 = -1;
					d8 = -1;
					d9 = -1;

					try
					{
						sscanf(cmd.data(), "%d %d %d %d %d %d %d %d %d %d\n", &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);
					}
					catch(...)
					{
						errorOccured = true;
						std::string e = "ConsoleInput::runTask exception in parsing input";
						pLogger->LogError(e);
					}
					if(errorOccured) {
						continue;
					}

					switch(d0)
					{
						case 0:
						{
							Poco::Net::SocketAddress address("127.0.0.1:60001");
							if(!_pCmdRunner->Init(address)) {
								pLogger->LogError("ConsoleInput::runTask failed to connect to " + address.toString());
							}
						}
						break;

						case 1:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong smart card number");
							}
							else {
								json = CommandFactory::CmdInsertSmartCard(d1);
							}
						}
						break;

						case 2:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong smart card number");
							}
							else {
								json = CommandFactory::CmdRemoveSmartCard(d1);
							}
						}
						break;

						case 3:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong smart card number");
							}
							else if(d2 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong period value");
							}
							else {
								json = CommandFactory::CmdTapSmartCard(d1, d2);
							}
						}
						break;

						case 4:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong smart card number");
							}
							else if(d2 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong period value");
							}
							else {
								json = CommandFactory::CmdSwipeSmartCard(d1, d2);
							}
						}
						break;

						case 5:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong smart card number");
							}
							else if(d2 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong period value");
							}
							else {
								json = CommandFactory::CmdShowBarCode(d1, d2);
							}
						}
						break;

						case 6:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong down period");
							}
							else if(d2 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong up period");
							}
							else if(d3 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k1");
							}
							else if(d4 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k2");
							}
							else if(d5 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k3");
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								json = CommandFactory::CmdPressPedKey(d1, d2, numbers);
							}
						}
						break;

						case 7:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong down period");
							}
							else if(d2 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong up period");
							}
							else if(d3 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k1");
							}
							else if(d4 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k2");
							}
							else if(d5 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k3");
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								json = CommandFactory::CmdPressSoftKey(d1, d2, numbers);
							}
						}
						break;

						case 8:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong down period");
							}
							else if(d2 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong up period");
							}
							else if(d3 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k1");
							}
							else if(d4 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k2");
							}
							else if(d5 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k3");
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								json = CommandFactory::CmdPressAssistKey(d1, d2, numbers);
							}
						}
						break;

						case 9:
						{
							if(d1 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong down period");
							}
							else if(d2 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong up period");
							}
							else if(d3 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k1");
							}
							else if(d4 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k2");
							}
							else if(d5 == -1) {
								pLogger->LogError("ConsoleInput::runTask wrong k3");
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								json = CommandFactory::CmdTouchScreenKey(d1, d2, numbers);
							}
						}
						break;

						case 10:
						{
							json = CommandFactory::CmdPowerOnOpt();
						}
						break;

						case 11:
						{
							json = CommandFactory::CmdPowerOffOpt();
						}
						break;

						default:
						{
							pLogger->LogError("ConsoleInput::runTask unknown command: " + cmd);
						}
						break;
					}

					if(!json.empty()) {
						_pCmdRunner->RunJsonCommand(json);
					}
				}

				pLogger->LogInfo(CommandFactory::Help());
			}
		}

		pLogger->LogInfo("ConsoleInput::runTask exits");
	}
};


#endif /* CONSOLEINPUT_H_ */
