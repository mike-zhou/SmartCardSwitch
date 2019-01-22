/*
 * CommandFactory.h
 *
 *  Created on: Dec 20, 2018
 *      Author: mikez
 */

#ifndef COMMANDFACTORY_H_
#define COMMANDFACTORY_H_

#include <string>
#include <vector>

class CommandFactory
{
public:
	static std::string CmdInsertSmartCard(unsigned int cardNumber)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"insert smart card\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json +=  	"\"smartCardNumber\":" + std::to_string(cardNumber);
		json += "}";

		return json;
	}

	static std::string CmdRemoveSmartCard(unsigned int cardNumber)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"remove smart card\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json +=  	"\"smartCardNumber\":" + std::to_string(cardNumber);
		json += "}";

		return json;
	}

	static std::string CmdTapSmartCard(unsigned int cardNumber, unsigned int period)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"tap smart card\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json +=  	"\"smartCardNumber\":" + std::to_string(cardNumber) + ",";
		json += 	"\"downPeriod\":" + std::to_string(period);
		json += "}";

		return json;
	}

	static std::string CmdSwipeSmartCard(unsigned int cardNumber, unsigned int period)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"swipe smart card\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json +=  	"\"smartCardNumber\":" + std::to_string(cardNumber) + ",";
		json += 	"\"downPeriod\":" + std::to_string(period);
		json += "}";

		return json;
	}

	static std::string CmdShowBarCode(unsigned int cardNumber, unsigned int period)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"show bar code\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json +=  	"\"smartCardNumber\":" + std::to_string(cardNumber) + ",";
		json += 	"\"downPeriod\":" + std::to_string(period);
		json += "}";

		return json;
	}

	static std::string CmdPressPedKey(unsigned int downPeriod, unsigned int upPeriod, const std::vector<unsigned int>& keyNumbers)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"press PED key\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json += 	"\"downPeriod\":" + std::to_string(downPeriod) + ",";
		json += 	"\"upPeriod\":" + std::to_string(upPeriod) + ",";
		json += 	"\"keys\": [";
		if(keyNumbers.size() > 0) {
			json += 	"{\"keyIndex\":0,\"keyNumber\":" + std::to_string(keyNumbers[0]) + "}";
		}
		for(unsigned int i=1; i<keyNumbers.size(); i++)
		{
			json += 	",{\"keyIndex\":" + std::to_string(i) + ",\"keyNumber\":" + std::to_string(keyNumbers[i]) + "}";
		}
		json += 	"]";
		json += "}";

		return json;
	}

	static std::string CmdPressSoftKey(unsigned int downPeriod, unsigned int upPeriod, const std::vector<unsigned int>& keyNumbers)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"press soft key\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json += 	"\"downPeriod\":" + std::to_string(downPeriod) + ",";
		json += 	"\"upPeriod\":" + std::to_string(upPeriod) + ",";
		json += 	"\"keys\": [";
		if(keyNumbers.size() > 0) {
			json += 	"{\"keyIndex\":0,\"keyNumber\":" + std::to_string(keyNumbers[0]) + "}";
		}
		for(unsigned int i=1; i<keyNumbers.size(); i++)
		{
			json += 	",{\"keyIndex\":" + std::to_string(i) + ",\"keyNumber\":" + std::to_string(keyNumbers[i]) + "}";
		}
		json += 	"]";
		json += "}";

		return json;
	}

	static std::string CmdPressAssistKey(unsigned int downPeriod, unsigned int upPeriod, const std::vector<unsigned int>& keyNumbers)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"press assist key\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json += 	"\"downPeriod\":" + std::to_string(downPeriod) + ",";
		json += 	"\"upPeriod\":" + std::to_string(upPeriod) + ",";
		json += 	"\"keys\": [";
		if(keyNumbers.size() > 0) {
			json += 	"{\"keyIndex\":0,\"keyNumber\":" + std::to_string(keyNumbers[0]) + "}";
		}
		for(unsigned int i=1; i<keyNumbers.size(); i++)
		{
			json += 	",{\"keyIndex\":" + std::to_string(i) + ",\"keyNumber\":" + std::to_string(keyNumbers[i]) + "}";
		}
		json += 	"]";
		json += "}";

		return json;
	}

	static std::string CmdTouchScreenKey(unsigned int downPeriod, unsigned int upPeriod, const std::vector<unsigned int>& keyNumbers)
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"touch screen\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\",";
		json += 	"\"downPeriod\":" + std::to_string(downPeriod) + ",";
		json += 	"\"upPeriod\":" + std::to_string(upPeriod) + ",";
		json += 	"\"keys\": [";
		if(keyNumbers.size() > 0) {
			json += 	"{\"keyIndex\":0,\"keyNumber\":" + std::to_string(keyNumbers[0]) + "}";
		}
		for(unsigned int i=1; i<keyNumbers.size(); i++)
		{
			json += 	",{\"keyIndex\":" + std::to_string(i) + ",\"keyNumber\":" + std::to_string(keyNumbers[i]) + "}";
		}
		json += 	"]";
		json += "}";

		return json;
	}

	static std::string CmdPowerOnOpt()
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"power on opt\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\"";
		json += "}";

		return json;
	}

	static std::string CmdPowerOffOpt()
	{
		std::string json;

		json += "{";
		json +=  	"\"userCommand\":\"power off opt\",";
		json +=  	"\"commandId\":\"" + getUniqueId() + "\"";
		json += "}";

		return json;
	}

	static std::string Help()
	{
		std::string str;

		str += "\r\n";
		str += "------------- HELP -------------\r\n";
		str += "ConnectToSmartCardSwitch: 0\r\n";
		str += "InsertSmartCard --------: 1 smartCardNumber\r\n";
		str += "RemoveSmartCard --------: 2 smartCardNumber\r\n";
		str += "TapSmartCard -----------: 3 smartCardNumber\r\n";
		str += "SwipeSmartCard ---------: 4 smartCardNumber\r\n";
		str += "ShowBarCode ------------: 5 smartCardNumber\r\n";
		str += "PressPedKey ------------: 6 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PressSoftKey -----------: 7 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PressAssistKey ---------: 8 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PressScreenKey ---------: 9 downPeriiod upPeriod key1 key2 key3\r\n";
		str += "PowerOnOPT -------------: 10\r\n";
		str += "PowerOffOPT ------------: 11\r\n";
		str += "--------------------------------\r\n";

		return str;
	}

private:
	static unsigned long _uniqueId;

	static std::string getUniqueId()
	{
		_uniqueId++;
		return std::to_string(_uniqueId);
	}

};

#endif /* COMMANDFACTORY_H_ */
