/*
 * CoordinateStorage.cpp
 *
 *  Created on: Nov 6, 2018
 *      Author: mikez
 */
#include <stddef.h>
#include <fcntl.h>
#include <stdio.h>

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
	_wAdjustment = 0;
	ReloadCoordinate();
}

int CoordinateStorage::GetWAdjustment()
{
	return _wAdjustment;
}

void CoordinateStorage::SetWAdjustment(int adjustment)
{
	_wAdjustment = adjustment;
}

void CoordinateStorage::ReloadCoordinate()
{
	_smartCardSlowlyPlaceStart = -1;
	_smartCardSlowlyPlaceEnd = -1;
	_smartCardFetchOffset = -1;
	_smartCardReaderSlowInsertEnd = -1;
	_smartCardReleaseOffset = -1;
	_smartCardInsertExtra = -1;

	_maximumX = -1;
	_maximumY = -1;
	_maximumZ = -1;
	_maximumW = -1;

	_home.x = 0;
	_home.y = 0;
	_home.z = 0;
	_home.w = 0;

	_smartCards.clear();
	_pedKeys.clear();
	_pedKeysPressed.clear();
	_softKeys.clear();
	_softKeysPressed.clear();
	_touchScreenKeys.clear();
	_touchScreenKeysPressed.clear();
	_assistKeys.clear();
	_assistKeysPressed.clear();
	_smartCardOffsets.clear();

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
		FILE * fd = fopen(_filePathName.c_str(), "r");
		if(fd == NULL) {
			pLogger->LogError("CoordinateStorage::CoordinateStorage cannot open file: " + _filePathName);
			return;
		}
		//read out the file content
		for(;;)
		{
			unsigned char c;
			auto amount = fread(&c, 1, 1, fd);
			if(amount < 1) {
				break;
			}
			else {
				json.push_back(c);
			}
		}
		//close file
		fclose(fd);

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
			_smartCardGate.x = ds["smartCards"]["gate"]["x"];
			_smartCardGate.y = ds["smartCards"]["gate"]["y"];
			_smartCardGate.z = ds["smartCards"]["gate"]["z"];
			_smartCardGate.w = ds["smartCards"]["gate"]["w"];
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

			//smart card offset
			auto smartCardOffsetAmount = ds["smartCardOffsets"].size();
			for(unsigned int i=0; i<smartCardOffsetAmount; i++)
			{
				unsigned int index;
				int value;

				index = ds["smartCardOffsets"][i]["index"];
				value = ds["smartCardOffsets"][i]["value"];
				SetSmartCardOffset(index, value);
			}

			//PED keys
			_pedKeyGate.x = ds["pedKeys"]["gate"]["x"];
			_pedKeyGate.y = ds["pedKeys"]["gate"]["y"];
			_pedKeyGate.z = ds["pedKeys"]["gate"]["z"];
			_pedKeyGate.w = ds["pedKeys"]["gate"]["w"];
			auto pedKeysAmount = ds["pedKeys"]["keys"].size();
			for(unsigned int i=0; i<pedKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["pedKeys"]["keys"][i]["index"];
				x = ds["pedKeys"]["keys"][i]["value"]["x"];
				y = ds["pedKeys"]["keys"][i]["value"]["y"];
				z = ds["pedKeys"]["keys"][i]["value"]["z"];
				w = ds["pedKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::PedKey, x, y, z, w, index);
			}
			auto pedKeysPressedAmount = ds["pedKeys"]["keysPressed"].size();
			for(unsigned int i=0; i<pedKeysPressedAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["pedKeys"]["keysPressed"][i]["index"];
				x = ds["pedKeys"]["keysPressed"][i]["value"]["x"];
				y = ds["pedKeys"]["keysPressed"][i]["value"]["y"];
				z = ds["pedKeys"]["keysPressed"][i]["value"]["z"];
				w = ds["pedKeys"]["keysPressed"][i]["value"]["w"];

				SetCoordinate(Type::PedKeyPressed, x, y, z, w, index);
			}

			//soft keys
			_softKeyGate.x = ds["softKeys"]["gate"]["x"];
			_softKeyGate.y = ds["softKeys"]["gate"]["y"];
			_softKeyGate.z = ds["softKeys"]["gate"]["z"];
			_softKeyGate.w = ds["softKeys"]["gate"]["w"];
			auto softKeysAmount = ds["softKeys"]["keys"].size();
			for(unsigned int i=0; i<softKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["softKeys"]["keys"][i]["index"];
				x = ds["softKeys"]["keys"][i]["value"]["x"];
				y = ds["softKeys"]["keys"][i]["value"]["y"];
				z = ds["softKeys"]["keys"][i]["value"]["z"];
				w = ds["softKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::SoftKey, x, y, z, w, index);
			}
			auto softKeysPressedAmount = ds["softKeys"]["keysPressed"].size();
			for(unsigned int i=0; i<softKeysPressedAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["softKeys"]["keysPressed"][i]["index"];
				x = ds["softKeys"]["keysPressed"][i]["value"]["x"];
				y = ds["softKeys"]["keysPressed"][i]["value"]["y"];
				z = ds["softKeys"]["keysPressed"][i]["value"]["z"];
				w = ds["softKeys"]["keysPressed"][i]["value"]["w"];

				SetCoordinate(Type::SoftKeyPressed, x, y, z, w, index);
			}

			//touch screen keys
			_touchScreenKeyGate.x = ds["touchScreenKeys"]["gate"]["x"];
			_touchScreenKeyGate.y = ds["touchScreenKeys"]["gate"]["y"];
			_touchScreenKeyGate.z = ds["touchScreenKeys"]["gate"]["z"];
			_touchScreenKeyGate.w = ds["touchScreenKeys"]["gate"]["w"];
			auto touchScreenKeysAmount = ds["touchScreenKeys"]["keys"].size();
			for(unsigned int i=0; i<touchScreenKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["touchScreenKeys"]["keys"][i]["index"];
				x = ds["touchScreenKeys"]["keys"][i]["value"]["x"];
				y = ds["touchScreenKeys"]["keys"][i]["value"]["y"];
				z = ds["touchScreenKeys"]["keys"][i]["value"]["z"];
				w = ds["touchScreenKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::TouchScreenKey, x, y, z, w, index);
			}
			auto touchScreenKeysPressedAmount = ds["touchScreenKeys"]["keysPressed"].size();
			for(unsigned int i=0; i<touchScreenKeysPressedAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["touchScreenKeys"]["keysPressed"][i]["index"];
				x = ds["touchScreenKeys"]["keysPressed"][i]["value"]["x"];
				y = ds["touchScreenKeys"]["keysPressed"][i]["value"]["y"];
				z = ds["touchScreenKeys"]["keysPressed"][i]["value"]["z"];
				w = ds["touchScreenKeys"]["keysPressed"][i]["value"]["w"];

				SetCoordinate(Type::TouchScreenKeyPressed, x, y, z, w, index);
			}

			//assist keys
			_assistKeyGate.x = ds["assistKeys"]["gate"]["x"];
			_assistKeyGate.y = ds["assistKeys"]["gate"]["y"];
			_assistKeyGate.z = ds["assistKeys"]["gate"]["z"];
			_assistKeyGate.w = ds["assistKeys"]["gate"]["w"];
			auto assistKeysAmount = ds["assistKeys"]["keys"].size();
			for(unsigned int i=0; i<assistKeysAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["assistKeys"]["keys"][i]["index"];
				x = ds["assistKeys"]["keys"][i]["value"]["x"];
				y = ds["assistKeys"]["keys"][i]["value"]["y"];
				z = ds["assistKeys"]["keys"][i]["value"]["z"];
				w = ds["assistKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::AssistKey, x, y, z, w, index);
			}
			auto assistKeysPressedAmount = ds["assistKeys"]["keysPressed"].size();
			for(unsigned int i=0; i<assistKeysPressedAmount; i++)
			{
				long x, y, z, w;
				long index;

				index = ds["assistKeys"]["keysPressed"][i]["index"];
				x = ds["assistKeys"]["keysPressed"][i]["value"]["x"];
				y = ds["assistKeys"]["keysPressed"][i]["value"]["y"];
				z = ds["assistKeys"]["keysPressed"][i]["value"]["z"];
				w = ds["assistKeys"]["keysPressed"][i]["value"]["w"];

				SetCoordinate(Type::AssistKeyPressed, x, y, z, w, index);
			}

			//smart card reader
			_smartCardReaderGate.x = ds["smartCardReader"]["gate"]["x"];
			_smartCardReaderGate.y = ds["smartCardReader"]["gate"]["y"];
			_smartCardReaderGate.z = ds["smartCardReader"]["gate"]["z"];
			_smartCardReaderGate.w = ds["smartCardReader"]["gate"]["w"];
			_smartCardReader.x = ds["smartCardReader"]["reader"]["x"];
			_smartCardReader.y = ds["smartCardReader"]["reader"]["y"];
			_smartCardReader.z = ds["smartCardReader"]["reader"]["z"];
			_smartCardReader.w = ds["smartCardReader"]["reader"]["w"];

			//contactless reader
			_contactlessReaderGate.x = ds["contactlessReader"]["gate"]["x"];
			_contactlessReaderGate.y = ds["contactlessReader"]["gate"]["y"];
			_contactlessReaderGate.z = ds["contactlessReader"]["gate"]["z"];
			_contactlessReaderGate.w = ds["contactlessReader"]["gate"]["w"];
			_contactlessReader.x = ds["contactlessReader"]["reader"]["x"];
			_contactlessReader.y = ds["contactlessReader"]["reader"]["y"];
			_contactlessReader.z = ds["contactlessReader"]["reader"]["z"];
			_contactlessReader.w = ds["contactlessReader"]["reader"]["w"];

			//bar code reader
			_barCodeReaderGate.x = ds["barCodeReader"]["gate"]["x"];
			_barCodeReaderGate.y = ds["barCodeReader"]["gate"]["y"];
			_barCodeReaderGate.z = ds["barCodeReader"]["gate"]["z"];
			_barCodeReaderGate.w = ds["barCodeReader"]["gate"]["w"];
			_barCodeReader.x = ds["barCodeReader"]["reader"]["x"];
			_barCodeReader.y = ds["barCodeReader"]["reader"]["y"];
			_barCodeReader.z = ds["barCodeReader"]["reader"]["z"];
			_barCodeReader.w = ds["barCodeReader"]["reader"]["w"];

			//safe
//			_safe.x = ds["safe"]["x"];
//			_safe.y = ds["safe"]["y"];
//			_safe.z = ds["safe"]["z"];
//			_safe.w = ds["safe"]["w"];

			//offset
			_smartCardSlowlyPlaceStart = ds["smartCardSlowlyPlaceStart"];
			_smartCardSlowlyPlaceEnd = ds["smartCardSlowlyPlaceEnd"];
			_smartCardFetchOffset = ds["smartCardFetchOffset"];
			_smartCardReaderSlowInsertEnd = ds["smartCardReaderSlowInsertEnd"];
			_smartCardReleaseOffset = ds["smartCardReleaseOffset"];
			_smartCardInsertExtra = ds["smartCardInsertExtra"];
//
//			//maximum
//			_maximumX = ds["maximumX"];
//			_maximumY = ds["maximumY"];
//			_maximumZ = ds["maximumZ"];
//			_maximumW = ds["maximumW"];

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
	json = json + "\"gate\":" + _smartCardGate.ToJsonObj() + ",";
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

	//smart card offsets
	json = json + ",\"smartCardOffsets\":[";
	for(unsigned int i=0; i<_smartCardOffsets.size(); i++) {
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + std::to_string(_smartCardOffsets[i]) + "},";
	}
	if(!_smartCardOffsets.empty()) {
		json.pop_back();
	}
	json = json + "]";//end of smart card offset.

	//PED keys
	json = json + ",\"pedKeys\": {";
	json = json + "\"gate\":" + _pedKeyGate.ToJsonObj() + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_pedKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _pedKeys[i].ToJsonObj() + "},";
	}
	if(!_pedKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "],";//end of keys
	json = json + "\"keysPressed\":["; //start of keysOressed
	for(unsigned int i=0; i<_pedKeysPressed.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _pedKeysPressed[i].ToJsonObj() + "},";
	}
	if(!_pedKeysPressed.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys

	json = json + "}";//end of pedKeys.

	//soft keys
	json = json + ",\"softKeys\": {";
	json = json + "\"gate\":" + _softKeyGate.ToJsonObj() + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_softKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _softKeys[i].ToJsonObj() + "},";
	}
	if(!_softKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "],";//end of keys
	json = json + "\"keysPressed\":["; //start of keys
	for(unsigned int i=0; i<_softKeysPressed.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _softKeysPressed[i].ToJsonObj() + "},";
	}
	if(!_softKeysPressed.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys
	json = json + "}";//end of softKeys.

	//touchScreen keys
	json = json + ",\"touchScreenKeys\": {";
	json = json + "\"gate\":" + _touchScreenKeyGate.ToJsonObj() + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_touchScreenKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _touchScreenKeys[i].ToJsonObj() + "},";
	}
	if(!_touchScreenKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "],";//end of keys
	json = json + "\"keysPressed\":["; //start of keys
	for(unsigned int i=0; i<_touchScreenKeysPressed.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _touchScreenKeysPressed[i].ToJsonObj() + "},";
	}
	if(!_touchScreenKeysPressed.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys
	json = json + "}";//end of touchScreenKeys.

	//assist keys
	json = json + ",\"assistKeys\": {";
	json = json + "\"gate\":" + _assistKeyGate.ToJsonObj() + ",";
	json = json + "\"keys\":["; //start of keys
	for(unsigned int i=0; i<_assistKeys.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _assistKeys[i].ToJsonObj() + "},";
	}
	if(!_assistKeys.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "],";//end of keys
	json = json + "\"keysPressed\":["; //start of keys
	for(unsigned int i=0; i<_assistKeysPressed.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _assistKeysPressed[i].ToJsonObj() + "},";
	}
	if(!_assistKeysPressed.empty()) {
		json.pop_back(); //delete the extra ','
	}
	json = json + "]";//end of keys
	json = json + "}";//end of assistKeys.

	//smart card reader
	json = json + ",\"smartCardReader\": {";
	json = json + "\"gate\":" + _smartCardReaderGate.ToJsonObj() + ",";
	json = json + "\"reader\":" + _smartCardReader.ToJsonObj();
	json = json + "}";

	//contactless reader
	json = json + ",\"contactlessReader\": {";
	json = json + "\"gate\":" + _contactlessReaderGate.ToJsonObj() + ",";
	json = json + "\"reader\":" + _contactlessReader.ToJsonObj();
	json = json + "}";

	//bar code reader
	json = json + ",\"barCodeReader\": {";
	json = json + "\"gate\":" + _barCodeReaderGate.ToJsonObj() + ",";
	json = json + "\"reader\":" + _barCodeReader.ToJsonObj();
	json = json + "}";

	//safe
	json = json + ",\"safe\":" + _safe.ToJsonObj();

	//offset
	json = json + ", \"smartCardSlowlyPlaceStart\":" + std::to_string(_smartCardSlowlyPlaceStart);
	json = json + ", \"smartCardSlowlyPlaceEnd\":" + std::to_string(_smartCardSlowlyPlaceEnd);
	json = json + ", \"smartCardFetchOffset\":" + std::to_string(_smartCardFetchOffset);
	json = json + ",\"smartCardReleaseOffset\":" + std::to_string(_smartCardReleaseOffset);
	json = json + ", \"smartCardReaderSlowInsertEnd\":" + std::to_string(_smartCardReaderSlowInsertEnd);
	json = json + ",\"smartCardInsertExtra\":" + std::to_string(_smartCardInsertExtra);

	//maximum
	json = json + ",\"maximumX\":" + std::to_string(_maximumX);
	json = json + ",\"maximumY\":" + std::to_string(_maximumY);
	json = json + ",\"maximumZ\":" + std::to_string(_maximumZ);
	json = json + ",\"maximumW\":" + std::to_string(_maximumW);

	json = json + "}";

	//write json string to file
	try
	{
		FILE * fd;
		Poco::File storageFile(_filePathName);

		if(storageFile.exists()) {
			storageFile.remove(false);
		}

		fd = fopen(_filePathName.c_str(),"w");
		if(fd != NULL)
		{
			pLogger->LogInfo("CoordinateStorage::PersistToFile write " + std::to_string(json.size()) + " bytes to file " + _filePathName);
			auto amount = fwrite(json.c_str(), json.size(), 1, fd);
			if(amount != json.size()) {
				pLogger->LogError("CoordinateStorage::PersistToFile failure in writing: " + std::to_string(amount) + "/" + std::to_string(json.size()));
			}
			else {
				rc = true;
			}
			fclose(fd);
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
		case Type::SmartCardGate:
		{
			_smartCardGate = value;
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

		case Type::PedKeyGate:
		{
			_pedKeyGate = value;
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

		case Type::PedKeyPressed:
		{
			if(index < PED_KEYS_AMOUNT)
			{
				if(index >= _pedKeysPressed.size())
				{
					Coordinate tmp;
					// fill _pedKeys
					for(; index >= _pedKeysPressed.size(); ) {
						_pedKeysPressed.push_back(tmp);
					}
				}
				_pedKeysPressed[index] = value;
				rc = true;
			}
			else {
				pLogger->LogError("CoordinateStorage::SetCoordinate PED key index out of range: " + std::to_string(index));
			}
		}
		break;

		case Type::SoftKeyGate:
		{
			_softKeyGate = value;
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

		case Type::SoftKeyPressed:
		{
			if(index < SOFT_KEYS_AMOUNT)
			{
				if(index >= _softKeysPressed.size())
				{
					Coordinate tmp;
					// fill _softKeys
					for(; index >= _softKeysPressed.size(); ) {
						_softKeysPressed.push_back(tmp);
					}
				}
				_softKeysPressed[index] = value;
				rc = true;
			}
			else {
				pLogger->LogError("CoordinateStorage::SetCoordinate soft key index out of range: " + std::to_string(index));
			}
		}
		break;

		case Type::TouchScreenKeyGate:
		{
			_touchScreenKeyGate = value;
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

		case Type::TouchScreenKeyPressed:
		{
			if(index < TOUCH_SCREEN_KEYS_AMOUNT)
			{
				if(index >= _touchScreenKeysPressed.size())
				{
					Coordinate tmp;
					// fill _touchScreenKeys
					for(; index >= _touchScreenKeysPressed.size(); ) {
						_touchScreenKeysPressed.push_back(tmp);
					}
				}
				_touchScreenKeysPressed[index] = value;
				rc = true;
			}
			else {
				pLogger->LogError("CoordinateStorage::SetCoordinate touch screen key index out of range: " + std::to_string(index));
			}
		}
		break;

		case Type::AssistKeyGate:
		{
			_assistKeyGate = value;
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

		case Type::AssistKeyPressed:
		{
			if(index < ASSIST_KEYS_AMOUNT)
			{
				if(index >= _assistKeysPressed.size())
				{
					Coordinate tmp;
					// fill _assistKeys
					for(; index >= _assistKeysPressed.size(); ) {
						_assistKeysPressed.push_back(tmp);
					}
				}
				_assistKeysPressed[index] = value;
				rc = true;
			}
			else {
				pLogger->LogError("CoordinateStorage::SetCoordinate assist key index out of range: " + std::to_string(index));
			}
		}
		break;

		case Type::SmartCardReaderGate:
		{
			_smartCardReaderGate = value;
			rc = true;
		}
		break;

		case Type::SmartCardReader:
		{
			_smartCardReader = value;
			rc = true;
		}
		break;

		case Type::BarCodeReaderGate:
		{
			_barCodeReaderGate = value;
			rc = true;
		}
		break;

		case Type::BarCodeReader:
		{
			_barCodeReader = value;
			rc = true;
		}
		break;



		case Type::Home:
		{
			_home = value;
			rc = true;
		}
		break;

		case Type::ContactlessReaderGate:
		{
			_contactlessReaderGate = value;
			rc = true;
		}
		break;

		case Type::ContactlessReader:
		{
			_contactlessReader = value;
			rc = true;
		}
		break;

		case Type::Safe:
		{
			_safe = value;
			rc = true;
		}
		break;

		default:
		{
			pLogger->LogError("CoordinateStorage::SetCoordinate unknown type: " + std::to_string(type));
		}
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
	case Type::SmartCardGate:
	{
		value = _smartCardGate;
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

	case Type::PedKeyGate:
	{
		value = _pedKeyGate;
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

	case Type::PedKeyPressed:
	{
		if(index < _pedKeysPressed.size())
		{
			value = _pedKeysPressed[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate PED key pressed index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::SoftKeyGate:
	{
		value = _softKeyGate;
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

	case Type::SoftKeyPressed:
	{
		if(index < _softKeysPressed.size())
		{
			value = _softKeysPressed[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate soft key pressed index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::TouchScreenKeyGate:
	{
		value = _touchScreenKeyGate;
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

	case Type::TouchScreenKeyPressed:
	{
		if(index < _touchScreenKeysPressed.size())
		{
			value = _touchScreenKeysPressed[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate touch screen pressed index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::AssistKeyGate:
	{
		value = _assistKeyGate;
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

	case Type::AssistKeyPressed:
	{
		if(index < _assistKeysPressed.size())
		{
			value = _assistKeysPressed[index];
			rc = true;
		}
		else
		{
			pLogger->LogError("CoordinateStorage::GetCoordinate assist key pressed index out of range: " + std::to_string(index));
		}
	}
	break;

	case Type::SmartCardReaderGate:
	{
		value = _smartCardReaderGate;
		rc = true;
	}
	break;

	case Type::SmartCardReader:
	{
		value = _smartCardReader;
		rc = true;
	}
	break;

	case Type::BarCodeReaderGate:
	{
		value = _barCodeReaderGate;
		rc = true;
	}
	break;

	case Type::BarCodeReader:
	{
		value = _barCodeReader;
		rc = true;
	}
	break;

	case Type::ContactlessReaderGate:
	{
		value = _contactlessReaderGate;
		rc = true;
	}
	break;

	case Type::ContactlessReader:
	{
		value = _contactlessReader;
		rc = true;
	}
	break;

	case Type::Home:
	{
		value = _home;
		rc = true;
	}
	break;

	case Type::Safe:
	{
		value = _safe;
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
		w = value.w + _wAdjustment;
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

void CoordinateStorage::SetSmartCardSlowlyPlaceStartZ(long zPosition)
{
	_smartCardSlowlyPlaceStart = zPosition;
}

void CoordinateStorage::SetSmartCardSlowlyPlaceEndZ(long zPosition)
{
	_smartCardSlowlyPlaceEnd = zPosition;
}

void CoordinateStorage::SetSmartCardFetchOffset(long offset)
{
	_smartCardFetchOffset = offset;
}

void CoordinateStorage::SetSmartCardReleaseOffsetZ(long offset)
{
	_smartCardReleaseOffset = offset;
}

void CoordinateStorage::SetSmartCardInsertExtra(long offset)
{
	_smartCardInsertExtra = offset;
}

bool CoordinateStorage::GetSmartCardSlowlyPlaceStartZ(long & zPosition)
{
	if(_smartCardSlowlyPlaceStart < 0) {
		return false;
	}

	zPosition = _smartCardSlowlyPlaceStart;
	return true;
}

bool CoordinateStorage::GetSmartCardSlowlyPlaceEndZ(long & zPosition)
{
	if(_smartCardSlowlyPlaceEnd < 0) {
		return false;
	}

	zPosition = _smartCardSlowlyPlaceEnd;
	return true;
}

bool CoordinateStorage::GetSmartCardFetchOffset(long & offset)
{
	if(_smartCardFetchOffset < 0) {
		return false;
	}

	offset = _smartCardFetchOffset;
	return true;
}

bool CoordinateStorage::GetSmartCardReleaseOffset(long & offset)
{
	if(_smartCardReleaseOffset < 0) {
		return false;
	}

	offset = _smartCardReleaseOffset;
	return true;
}

bool CoordinateStorage::GetSmartCardInsertExtra(long & offset)
{
	if(_smartCardInsertExtra < 0) {
		return false;
	}

	offset = _smartCardInsertExtra;
	return true;
}

void CoordinateStorage::SetSmartCardReaderSlowInsertEndY(long yPosition)
{
	_smartCardReaderSlowInsertEnd = yPosition;
}

bool CoordinateStorage::GetSmartCardReaderSlowInsertEndY(long & yPosition)
{
	if(_smartCardReaderSlowInsertEnd < 0) {
		return false;
	}

	yPosition = _smartCardReaderSlowInsertEnd;
	return true;
}

void CoordinateStorage::SetMaximumX(long value)
{
	_maximumX = value;
}

void CoordinateStorage::SetMaximumY(long value)
{
	_maximumY = value;
}

void CoordinateStorage::SetMaximumZ(long value)
{
	_maximumZ = value;
}

void CoordinateStorage::SetMaximumW(long value)
{
	_maximumW = value;
}

bool CoordinateStorage::GetMaximumX(long & value)
{
	if(_maximumX == -1) {
		return false;
	}

	value = _maximumX;
	return true;
}

bool CoordinateStorage::GetMaximumY(long & value)
{
	if(_maximumY == -1) {
		return false;
	}

	value = _maximumY;
	return true;
}

bool CoordinateStorage::GetMaximumZ(long & value)
{
	if(_maximumZ == -1) {
		return false;
	}

	value = _maximumZ;
	return true;
}

bool CoordinateStorage::GetMaximumW(long & value)
{
	if(_maximumW == -1) {
		return false;
	}

	value = _maximumW;
	return true;
}

bool CoordinateStorage::SetSmartCardOffset(unsigned int index, int offset)
{
	for(;;) {
		if(_smartCardOffsets.size() <= index) {
			_smartCardOffsets.push_back(-1);
		}
		else {
			break;
		}
	}

	_smartCardOffsets[index] = offset;
	return true;
}

bool CoordinateStorage::GetSmartCardOffset(unsigned int index, int& offset)
{
	if(index >= _smartCardOffsets.size()) {
		return false;
	}

	offset = _smartCardOffsets[index];
	return true;
}

