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
#include "scsClient.h"

class ConsoleInput: public Poco::Task
{
public:
	ConsoleInput(): Task("ConsoleInput") { }
	virtual ~ConsoleInput() { }

private:
	std::deque<char> _input;

	std::string help()
	{
		std::string str;

		str += "\r\n";
		str += "------------- HELP -------------\r\n";
		str += "InsertSmartCard --------: 1 smartCardNumber\r\n";
		str += "RemoveSmartCard --------: 2 smartCardNumber\r\n";
		str += "SwipeSmartCard ---------: 3 smartCardNumber period\r\n";
		str += "TapSmartCard -----------: 4 smartCardNumber period\r\n";
		str += "ShowBarCode ------------: 5 smartCardNumber period\r\n";
		str += "PressPedKey ------------: 6 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PressSoftKey -----------: 7 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PressAssistKey ---------: 8 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PressScreenKey ---------: 9 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PowerOnOPT -------------: 10\r\n";
		str += "PowerOffOPT ------------: 11\r\n";
		str += "--------------------------------\r\n";

		return str;
	}

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

			std::cout << ("ConsoleInput::processInput invalid command: " + command) << "\r\n";
			command.clear();
		}

		return command;
	}

	void runTask()
	{
		auto pScsClient = GetScsClientInstance();

		if(ScsClient::ScsResult::Succeess != pScsClient->Initialize("./logs/clientConsole/", 1, 10, "127.0.0.1", 60001))
		{
			std::cout << "failed to connect to Smart Card Switch" << "\r\n";
		}

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
						std::cout << e << "\r\n";
					}
					if(errorOccured) {
						continue;
					}

					switch(d0)
					{
						case 1:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong smart card number") << "\r\n";
							}
							else {
								pScsClient->InsertSmartCard(d1);
							}
						}
						break;

						case 2:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong smart card number") << "\r\n";
							}
							else {
								pScsClient->RemoveSmartCard(d1);
							}
						}
						break;

						case 3:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong smart card number") << "\r\n";
							}
							else if(d2 == -1) {
								std::cout << ("ConsoleInput::runTask wrong period value") << "\r\n";
							}
							else {
								pScsClient->SwipeSmartCard(d1, d2);
							}
						}
						break;

						case 4:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong smart card number") << "\r\n";
							}
							else if(d2 == -1) {
								std::cout << ("ConsoleInput::runTask wrong period value") << "\r\n";
							}
							else {
								pScsClient->TapSmartCard(d1, d2);
							}
						}
						break;

						case 5:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong smart card number") << "\r\n";
							}
							else if(d2 == -1) {
								std::cout << ("ConsoleInput::runTask wrong period value") << "\r\n";
							}
							else {
								pScsClient->TapBarcode(d1, d2);
							}
						}
						break;

						case 6:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong down period") << "\r\n";
							}
							else if(d2 == -1) {
								std::cout << ("ConsoleInput::runTask wrong up period") << "\r\n";
							}
							else if(d3 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k1") << "\r\n";
							}
							else if(d4 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k2") << "\r\n";
							}
							else if(d5 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k3") << "\r\n";
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								pScsClient->PressPedKeys(numbers, d2, d1);
							}
						}
						break;

						case 7:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong down period") << "\r\n";
							}
							else if(d2 == -1) {
								std::cout << ("ConsoleInput::runTask wrong up period") << "\r\n";
							}
							else if(d3 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k1") << "\r\n";
							}
							else if(d4 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k2") << "\r\n";
							}
							else if(d5 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k3") << "\r\n";
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								pScsClient->PressSoftKeys(numbers, d2, d1);
							}
						}
						break;

						case 8:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong down period") << "\r\n";
							}
							else if(d2 == -1) {
								std::cout << ("ConsoleInput::runTask wrong up period") << "\r\n";
							}
							else if(d3 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k1") << "\r\n";
							}
							else if(d4 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k2") << "\r\n";
							}
							else if(d5 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k3") << "\r\n";
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								pScsClient->PressAssistKeys(numbers, d2, d1);
							}
						}
						break;

						case 9:
						{
							if(d1 == -1) {
								std::cout << ("ConsoleInput::runTask wrong down period") << "\r\n";
							}
							else if(d2 == -1) {
								std::cout << ("ConsoleInput::runTask wrong up period") << "\r\n";
							}
							else if(d3 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k1") << "\r\n";
							}
							else if(d4 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k2") << "\r\n";
							}
							else if(d5 == -1) {
								std::cout << ("ConsoleInput::runTask wrong k3") << "\r\n";
							}
							else {
								std::vector<unsigned int> numbers;

								numbers.push_back(d3);
								numbers.push_back(d4);
								numbers.push_back(d5);
								pScsClient->PressTouchScreenKeys(numbers, d2, d1);
							}
						}
						break;

//						case 10:
//						{
//							json = CommandFactory::CmdPowerOnOpt();
//						}
//						break;
//
//						case 11:
//						{
//							json = CommandFactory::CmdPowerOffOpt();
//						}
//						break;
//
						default:
						{
							std::cout << ("ConsoleInput::runTask unknown command: " + cmd) << "\r\n";
						}
						break;
					}
				}

				std::cout << help();
			}
		}

		std::cout << ("ConsoleInput::runTask exits") << "\r\n";
	}
};


#endif /* CONSOLEINPUT_H_ */
