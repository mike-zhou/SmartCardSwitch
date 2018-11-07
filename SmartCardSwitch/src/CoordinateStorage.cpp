/*
 * CoordinateStorage.cpp
 *
 *  Created on: Nov 6, 2018
 *      Author: mikez
 */
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>

#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Exception.h"
#include "Poco/Format.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Logger.h"
#include "CoordinateStorage.h"

extern Logger * pLogger;

CoordinateStorage::CoordinateStorage(std::string filePathName)
{
	_filePathName = filePathName;

	_smartCardFectchingYOffset = DEFAULT_OFFSET;
	_smartCardAccessingZOffset = DEFAULT_OFFSET;
	_pedKeyPressingZOffset = DEFAULT_OFFSET;
	_softKeyPressingZOffset = DEFAULT_OFFSET;
	_touchScreenKeyPressingZOffset = DEFAULT_OFFSET;
	_assistKeyPressingZOffset = DEFAULT_OFFSET;

	if(_filePathName.empty()) {
		pLogger->LogError("CoordinateStorage::CoordinateStorage empty file path & name");
		return;
	}

	Poco::File storageFile(_filePathName);
	if(storageFile.exists() == false) {
		pLogger->LogError("CoordinateStorage::CoordinateStorage file not exist: " + _filePathName);
		return;
	}

	try
	{
		std::string json;

		//open storage file
		int fd = open(_filePathName.c_str(), O_RDONLY);
		if(fd < 0) {
			pLogger->LogError("CoordinateStorage::CoordinateStorage cannot open file: " + _filePathName);
			return;
		}
		//read out the file content
		for(;;)
		{
			unsigned char c;
			auto amount = read(fd, &c, 1);
			if(amount < 1) {
				break;
			}
			else {
				json.push_back(c);
			}
		}
		//close file
		close(fd);

		if(json.empty()) {
			pLogger->LogError("CoordinateStorage::CoordinateStorage nothing read from: " + _filePathName);
		}
		else
		{
			//parse file content
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(json);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			//smart cards
			_smartCardsEntry.x = ds["smartCards"]["entry"]["x"];
			_smartCardsEntry.y = ds["smartCards"]["entry"]["y"];
			_smartCardsEntry.z = ds["smartCards"]["entry"]["z"];
			_smartCardsEntry.w = ds["smartCards"]["entry"]["w"];
			_smartCardsExit.x = ds["smartCards"]["exit"]["x"];
			_smartCardsExit.y = ds["smartCards"]["exit"]["y"];
			_smartCardsExit.z = ds["smartCards"]["exit"]["z"];
			_smartCardsExit.w = ds["smartCards"]["exit"]["w"];
			_smartCardFectchingYOffset = ds["smartCards"]["YOffset"];
			_smartCardAccessingZOffset = ds["smartCards"]["ZOffset"];
			auto smartCardsAmount = ds["smartCards"]["cards"].size();
			for(unsigned int i=0; i<smartCardsAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["smartCards"]["cards"][i]["index"];
				x = ds["smartCards"]["cards"][i]["value"]["x"];
				y = ds["smartCards"]["cards"][i]["value"]["y"];
				z = ds["smartCards"]["cards"][i]["value"]["z"];
				w = ds["smartCards"]["cards"][i]["value"]["w"];

				SetCoordinate(Type::SmartCard, x, y, z, w, index);
			}

			//PED keys
			_pedKeysEntry.x = ds["pedKeys"]["entry"]["x"];
			_pedKeysEntry.y = ds["pedKeys"]["entry"]["y"];
			_pedKeysEntry.z = ds["pedKeys"]["entry"]["z"];
			_pedKeysEntry.w = ds["pedKeys"]["entry"]["w"];
			_pedKeysExit.x = ds["pedKeys"]["exit"]["x"];
			_pedKeysExit.y = ds["pedKeys"]["exit"]["y"];
			_pedKeysExit.z = ds["pedKeys"]["exit"]["z"];
			_pedKeysExit.w = ds["pedKeys"]["exit"]["w"];
			_pedKeyPressingZOffset = ds["pedKeys"]["ZOffset"];
			auto pedKeysAmount = ds["pedKeys"]["keys"].size();
			for(unsigned int i=0; i<pedKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["pedKeys"]["cards"][i]["index"];
				x = ds["pedKeys"]["keys"][i]["value"]["x"];
				y = ds["pedKeys"]["keys"][i]["value"]["y"];
				z = ds["pedKeys"]["keys"][i]["value"]["z"];
				w = ds["pedKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::PedKey, x, y, z, w, index);
			}

			//soft keys
			_softKeysEntry.x = ds["softKeys"]["entry"]["x"];
			_softKeysEntry.y = ds["softKeys"]["entry"]["y"];
			_softKeysEntry.z = ds["softKeys"]["entry"]["z"];
			_softKeysEntry.w = ds["softKeys"]["entry"]["w"];
			_softKeysExit.x = ds["softKeys"]["exit"]["x"];
			_softKeysExit.y = ds["softKeys"]["exit"]["y"];
			_softKeysExit.z = ds["softKeys"]["exit"]["z"];
			_softKeysExit.w = ds["softKeys"]["exit"]["w"];
			_softKeyPressingZOffset = ds["softKeys"]["ZOffset"];
			auto softKeysAmount = ds["softKeys"]["keys"].size();
			for(unsigned int i=0; i<softKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["softKeys"]["cards"][i]["index"];
				x = ds["softKeys"]["keys"][i]["value"]["x"];
				y = ds["softKeys"]["keys"][i]["value"]["y"];
				z = ds["softKeys"]["keys"][i]["value"]["z"];
				w = ds["softKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::SoftKey, x, y, z, w, index);
			}

			//touch screen keys
			_touchScreenKeysEntry.x = ds["touchScreenKeys"]["entry"]["x"];
			_touchScreenKeysEntry.y = ds["touchScreenKeys"]["entry"]["y"];
			_touchScreenKeysEntry.z = ds["touchScreenKeys"]["entry"]["z"];
			_touchScreenKeysEntry.w = ds["touchScreenKeys"]["entry"]["w"];
			_touchScreenKeysExit.x = ds["touchScreenKeys"]["exit"]["x"];
			_touchScreenKeysExit.y = ds["touchScreenKeys"]["exit"]["y"];
			_touchScreenKeysExit.z = ds["touchScreenKeys"]["exit"]["z"];
			_touchScreenKeysExit.w = ds["touchScreenKeys"]["exit"]["w"];
			_touchScreenKeyPressingZOffset = ds["touchScreenKeys"]["ZOffset"];
			auto touchScreenKeysAmount = ds["touchScreenKeys"]["keys"].size();
			for(unsigned int i=0; i<touchScreenKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["touchScreenKeys"]["cards"][i]["index"];
				x = ds["touchScreenKeys"]["keys"][i]["value"]["x"];
				y = ds["touchScreenKeys"]["keys"][i]["value"]["y"];
				z = ds["touchScreenKeys"]["keys"][i]["value"]["z"];
				w = ds["touchScreenKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::TouchScreenKey, x, y, z, w, index);
			}

			//assist keys
			_assistKeysEntry.x = ds["assistKeys"]["entry"]["x"];
			_assistKeysEntry.y = ds["assistKeys"]["entry"]["y"];
			_assistKeysEntry.z = ds["assistKeys"]["entry"]["z"];
			_assistKeysEntry.w = ds["assistKeys"]["entry"]["w"];
			_assistKeysExit.x = ds["assistKeys"]["exit"]["x"];
			_assistKeysExit.y = ds["assistKeys"]["exit"]["y"];
			_assistKeysExit.z = ds["assistKeys"]["exit"]["z"];
			_assistKeysExit.w = ds["assistKeys"]["exit"]["w"];
			_assistKeyPressingZOffset = ds["assistKeys"]["ZOffset"];
			auto assistKeysAmount = ds["assistKeys"]["keys"].size();
			for(unsigned int i=0; i<assistKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["assistKeys"]["cards"][i]["index"];
				x = ds["assistKeys"]["keys"][i]["value"]["x"];
				y = ds["assistKeys"]["keys"][i]["value"]["y"];
				z = ds["assistKeys"]["keys"][i]["value"]["z"];
				w = ds["assistKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::AssistKey, x, y, z, w, index);
			}

			//smart card slot
			_smartCardSlotEntry.x = ds["smartCardSlot"]["entry"]["x"];
			_smartCardSlotEntry.y = ds["smartCardSlot"]["entry"]["y"];
			_smartCardSlotEntry.z = ds["smartCardSlot"]["entry"]["z"];
			_smartCardSlotEntry.w = ds["smartCardSlot"]["entry"]["w"];
			_smartCardSlotExit.x = ds["smartCardSlot"]["exit"]["x"];
			_smartCardSlotExit.y = ds["smartCardSlot"]["exit"]["y"];
			_smartCardSlotExit.z = ds["smartCardSlot"]["exit"]["z"];
			_smartCardSlotExit.w = ds["smartCardSlot"]["exit"]["w"];
			_smartCardSlot.x = ds["smartCardSlot"]["slot"]["x"];
			_smartCardSlot.y = ds["smartCardSlot"]["slot"]["y"];
			_smartCardSlot.z = ds["smartCardSlot"]["slot"]["z"];
			_smartCardSlot.w = ds["smartCardSlot"]["slot"]["w"];

			//contactless reader
			_contactlessReaderEntry.x = ds["contactlessReader"]["entry"]["x"];
			_contactlessReaderEntry.y = ds["contactlessReader"]["entry"]["y"];
			_contactlessReaderEntry.z = ds["contactlessReader"]["entry"]["z"];
			_contactlessReaderEntry.w = ds["contactlessReader"]["entry"]["w"];
			_contactlessReaderExit.x = ds["contactlessReader"]["exit"]["x"];
			_contactlessReaderExit.y = ds["contactlessReader"]["exit"]["y"];
			_contactlessReaderExit.z = ds["contactlessReader"]["exit"]["z"];
			_contactlessReaderExit.w = ds["contactlessReader"]["exit"]["w"];
			_contactlessReader.x = ds["contactlessReader"]["slot"]["x"];
			_contactlessReader.y = ds["contactlessReader"]["slot"]["y"];
			_contactlessReader.z = ds["contactlessReader"]["slot"]["z"];
			_contactlessReader.w = ds["contactlessReader"]["slot"]["w"];

			//bar code reader
			_barCodeReaderEntry.x = ds["barCodeReader"]["entry"]["x"];
			_barCodeReaderEntry.y = ds["barCodeReader"]["entry"]["y"];
			_barCodeReaderEntry.z = ds["barCodeReader"]["entry"]["z"];
			_barCodeReaderEntry.w = ds["barCodeReader"]["entry"]["w"];
			_barCodeReaderExit.x = ds["barCodeReader"]["exit"]["x"];
			_barCodeReaderExit.y = ds["barCodeReader"]["exit"]["y"];
			_barCodeReaderExit.z = ds["barCodeReader"]["exit"]["z"];
			_barCodeReaderExit.w = ds["barCodeReader"]["exit"]["w"];
			_barCodeReader.x = ds["barCodeReader"]["slot"]["x"];
			_barCodeReader.y = ds["barCodeReader"]["slot"]["y"];
			_barCodeReader.z = ds["barCodeReader"]["slot"]["z"];
			_barCodeReader.w = ds["barCodeReader"]["slot"]["w"];

			pLogger->LogInfo("CoordinateStorage::CoordinateStorage storage file is parsed successfully");
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CoordinateStorage::CoordinateStorage exception: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("CoordinateStorage::CoordinateStorage unknown exception");
	}
}

bool CoordinateStorage::PersistToFile()
{
	std::string json;
	bool rc = false;

	//create the json string
	json = "{";

	//smart cards
	json = json + "\"smartCards\": {";
	json = json + "\"entry\":" + _smartCardsEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _smartCardsExit.ToJsonObj() + ",";
	json = json + "\"YOffset\":" + std::to_string(_smartCardFectchingYOffset) + ",";
	json = json + "\"ZOffset\":" + std::to_string(_smartCardAccessingZOffset) + ",";
	json = json + "\"cards\":["; //start of cards
	for(unsigned int i=0; i<_smartCards.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _smartCards[i].ToJsonObj() + "},";
	}
	if(!_smartCards.empty()) {
		json.pop_back();//delete the extra ','
	}
	json = json + "]";//end of cards
	json = json + "}";//end of smartCards

	//PED keys
	json = json + ",\"pedKeys\": {";
	json = json + "\"entry\":" + _pedKeysEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _pedKeysExit.ToJsonObj() + ",";
	json = json + "\"ZOffset\":" + std::to_string(_pedKeyPressingZOffset) + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_pedKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _pedKeys[i].ToJsonObj() + "},";
	}
	if(!_pedKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys
	json = json + "}";//end of pedKeys.

	//soft keys
	json = json + ",\"softKeys\": {";
	json = json + "\"entry\":" + _softKeysEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _softKeysExit.ToJsonObj() + ",";
	json = json + "\"ZOffset\":" + std::to_string(_softKeyPressingZOffset) + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_softKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _softKeys[i].ToJsonObj() + "},";
	}
	if(!_softKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys
	json = json + "}";//end of softKeys.

	//touchScreen keys
	json = json + ",\"touchScreenKeys\": {";
	json = json + "\"entry\":" + _touchScreenKeysEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _touchScreenKeysExit.ToJsonObj() + ",";
	json = json + "\"ZOffset\":" + std::to_string(_touchScreenKeyPressingZOffset) + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_touchScreenKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _touchScreenKeys[i].ToJsonObj() + "},";
	}
	if(!_touchScreenKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys
	json = json + "}";//end of touchScreenKeys.

	//assist keys
	json = json + ",\"assistKeys\": {";
	json = json + "\"entry\":" + _assistKeysEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _assistKeysExit.ToJsonObj() + ",";
	json = json + "\"ZOffset\":" + std::to_string(_assistKeyPressingZOffset) + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_assistKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _assistKeys[i].ToJsonObj() + "},";
	}
	if(!_assistKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys
	json = json + "}";//end of assistKeys.

	//smart card slot
	json = json + ",\"smartCardSlot\": {";
	json = json + "\"entry\":" + _smartCardSlotEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _smartCardSlotExit.ToJsonObj() + ",";
	json = json + "\"slot\":" + _smartCardSlot.ToJsonObj();
	json = json + "}";

	//contactless reader
	json = json + ",\"contactlessReader\": {";
	json = json + "\"entry\":" + _contactlessReaderEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _contactlessReaderExit.ToJsonObj() + ",";
	json = json + "\"slot\":" + _contactlessReader.ToJsonObj();
	json = json + "}";

	//bar code reader
	json = json + ",\"barCodeReader\": {";
	json = json + "\"entry\":" + _barCodeReaderEntry.ToJsonObj() + ",";
	json = json + "\"exit\":" + _barCodeReaderExit.ToJsonObj() + ",";
	json = json + "\"slot\":" + _barCodeReader.ToJsonObj();
	json = json + "}";

	json = json + "}";

	//write json string to file
	try
	{
		int fd;
		Poco::File storageFile(_filePathName);

		if(!storageFile.exists()) {
			storageFile.createDirectories();
			storageFile.createFile();
		}

		fd = open(_filePathName.c_str(), O_CREAT | O_WRONLY);
		if(fd >= 0)
		{
			pLogger->LogInfo("CoordinateStorage::PersistToFile write " + std::to_string(json.size()) + " bytes to file " + _filePathName);
			auto amount = write(fd, json.c_str(), json.size());
			if(amount != json.size()) {
				pLogger->LogError("CoordinateStorage::PersistToFile failure in writing: " + std::to_string(amount) + "/" + std::to_string(json.size()));
			}
			else {
				rc = true;
			}
			close(fd);
		}
		else
		{
			pLogger->LogError("CoordinateStorage::PersistToFile cannot open " + _filePathName);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CoordinateStorage::PersistToFile exception: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("CoordinateStorage::PersistToFile unknown exception");
	}

	return rc;
}

bool CoordinateStorage::SetCoordinate(Type type,
				unsigned int x,
				unsigned int y,
				unsigned int z,
				unsigned int w,
				unsigned int index)
{
	bool rc = false;

	Coordinate value;
	value.x = x;
	value.y = y;
	value.z = z;
	value.w = w;

	switch(type)
	{
	case Type::SmartCardEntry:
	{
		_smartCardsEntry = value;
		rc = true;
	}
	break;

	case Type::SmartCard:
	{
		if(index < SMART_CARDS_AMOUNT)
		{
			if(index >= _smartCards.size())
			{
				Coordinate tmp;
				// fill smartCards
				for(; index >= _smartCards.size(); ) {
					_smartCards.push_back(tmp);
				}
			}
			_smartCards[index] = value;
			rc = true;
		}
		else {
			pLogger->LogError("CoordinateStorage::SetCoordinate smart card index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::SmartCardExit:
	{
		_smartCardsExit = value;
		rc = true;
	}
	break;

	case Type::PedKeysEntry:
	{
		_pedKeysEntry = value;
		rc = true;
	}
	break;

	case Type::PedKey:
	{
		if(index < PED_KEYS_AMOUNT)
		{
			if(index >= _pedKeys.size())
			{
				Coordinate tmp;
				// fill _pedKeys
				for(; index >= _pedKeys.size(); ) {
					_pedKeys.push_back(tmp);
				}
			}
			_pedKeys[index] = value;
			rc = true;
		}
		else {
			pLogger->LogError("CoordinateStorage::SetCoordinate PED key index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::PedKeysExit:
	{
		_pedKeysExit = value;
		rc = true;
	}
	break;

	case Type::SoftKeysEntry:
	{
		_softKeysEntry = value;
		rc = true;
	}
	break;

	case Type::SoftKey:
	{
		if(index < SOFT_KEYS_AMOUNT)
		{
			if(index >= _softKeys.size())
			{
				Coordinate tmp;
				// fill _softKeys
				for(; index >= _softKeys.size(); ) {
					_softKeys.push_back(tmp);
				}
			}
			_softKeys[index] = value;
			rc = true;
		}
		else {
			pLogger->LogError("CoordinateStorage::SetCoordinate soft key index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::SoftKeysExit:
	{
		_softKeysExit = value;
		rc = true;
	}
	break;

	case Type::TouchScreenKeysEntry:
	{
		_touchScreenKeysEntry = value;
		rc = true;
	}
	break;

	case Type::TouchScreenKey:
	{
		if(index < TOUCH_SCREEN_KEYS_AMOUNT)
		{
			if(index >= _touchScreenKeys.size())
			{
				Coordinate tmp;
				// fill _touchScreenKeys
				for(; index >= _touchScreenKeys.size(); ) {
					_touchScreenKeys.push_back(tmp);
				}
			}
			_touchScreenKeys[index] = value;
			rc = true;
		}
		else {
			pLogger->LogError("CoordinateStorage::SetCoordinate touch screen key index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::TouchScreenKeysExit:
	{
		_assistKeysExit = value;
		rc = true;
	}
	break;

	case Type::AssistKeysEntry:
	{
		_assistKeysEntry = value;
		rc = true;
	}
	break;

	case Type::AssistKey:
	{
		if(index < ASSIST_KEYS_AMOUNT)
		{
			if(index >= _assistKeys.size())
			{
				Coordinate tmp;
				// fill _assistKeys
				for(; index >= _assistKeys.size(); ) {
					_assistKeys.push_back(tmp);
				}
			}
			_assistKeys[index] = value;
			rc = true;
		}
		else {
			pLogger->LogError("CoordinateStorage::SetCoordinate assist key index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::AssistKeysExit:
	{
		_assistKeysExit = value;
		rc = true;
	}
	break;

	case Type::SmartCardSlotEntry:
	{
		_smartCardSlotEntry = value;
		rc = true;
	}
	break;

	case Type::SmartCardSlot:
	{
		_smartCardSlot = value;
		rc = true;
	}
	break;

	case Type::SmartCardSlotExit:
	{
		_smartCardSlotExit = value;
		rc = true;
	}
	break;

	case Type::BarCodeReaderEntry:
	{
		_barCodeReaderEntry = value;
		rc = true;
	}
	break;

	case Type::BarCodeReader:
	{
		_barCodeReader = value;
		rc = true;
	}
	break;

	case Type::BarCodeReaderExit:
	{
		_barCodeReaderExit = value;
		rc = true;
	}
	break;

	default:
		pLogger->LogError("CoordinateStorage::SetCoordinate unknown type: " + std::to_string(type));
		break;
	}

	return rc;
}


bool CoordinateStorage::GetCoordinate(Type type,
				int& x,
				int& y,
				int& z,
				int& w,
				unsigned int index)
{
	bool rc = false;

	Coordinate value;

	switch(type)
	{
	case Type::SmartCardEntry:
	{
		value = _smartCardsEntry;
		rc = true;
	}
	break;

	case Type::SmartCard:
	{
		if(index < _smartCards.size())
		{
			value = _smartCards[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate smart card index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::SmartCardExit:
	{
		value = _smartCardsExit;
		rc = true;
	}
	break;

	case Type::PedKeysEntry:
	{
		value = _pedKeysEntry;
		rc = true;
	}
	break;

	case Type::PedKey:
	{
		if(index < _pedKeys.size())
		{
			value = _pedKeys[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate PED key index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::PedKeysExit:
	{
		value = _pedKeysExit;
		rc = true;
	}
	break;

	case Type::SoftKeysEntry:
	{
		value = _softKeysEntry;
		rc = true;
	}
	break;

	case Type::SoftKey:
	{
		if(index < _softKeys.size())
		{
			value = _softKeys[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate soft key index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::SoftKeysExit:
	{
		value = _softKeysExit;
		rc = true;
	}
	break;

	case Type::TouchScreenKeysEntry:
	{
		value = _touchScreenKeysEntry;
		rc = true;
	}
	break;

	case Type::TouchScreenKey:
	{
		if(index < _touchScreenKeys.size())
		{
			value = _touchScreenKeys[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate touch screen index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::TouchScreenKeysExit:
	{
		value = _assistKeysExit;
		rc = true;
	}
	break;

	case Type::AssistKeysEntry:
	{
		value = _assistKeysEntry;
		rc = true;
	}
	break;

	case Type::AssistKey:
	{
		if(index < _assistKeys.size())
		{
			value = _assistKeys[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate assist key index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::AssistKeysExit:
	{
		value = _assistKeysExit;
		rc = true;
	}
	break;

	case Type::SmartCardSlotEntry:
	{
		value = _smartCardSlotEntry;
		rc = true;
	}
	break;

	case Type::SmartCardSlot:
	{
		value = _smartCardSlot;
		rc = true;
	}
	break;

	case Type::SmartCardSlotExit:
	{
		value = _smartCardSlotExit;
		rc = true;
	}
	break;

	case Type::BarCodeReaderEntry:
	{
		value = _barCodeReaderEntry;
		rc = true;
	}
	break;

	case Type::BarCodeReader:
	{
		value = _barCodeReader;
		rc = true;
	}
	break;

	case Type::BarCodeReaderExit:
	{
		value = _barCodeReaderExit;
		rc = true;
	}
	break;

	default:
		pLogger->LogError("CoordinateStorage::GetCoordinate unknown type: " + std::to_string(type));
		break;
	}

	if(rc)
	{
		x = value.x;
		y = value.y;
		z = value.z;
		w = value.w;
	}

	return rc;
}

CoordinateStorage::Coordinate::Coordinate()
{
	x = y = z = w = -1;
}

std::string CoordinateStorage::Coordinate::ToJsonObj()
{
	std::string json;

	json = "{";
	json = json + "\"x\":" + std::to_string(x) + ",";
	json = json + "\"y\":" + std::to_string(y) + ",";
	json = json + "\"z\":" + std::to_string(z) + ",";
	json = json + "\"w\":" + std::to_string(w);
	json = json + "}";

	return json;
}
