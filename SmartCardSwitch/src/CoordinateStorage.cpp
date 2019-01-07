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

	_smartCardPlaceStart = -1;
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

				index = ds["pedKeys"]["cards"][i]["index"];
				x = ds["pedKeys"]["keys"][i]["value"]["x"];
				y = ds["pedKeys"]["keys"][i]["value"]["y"];
				z = ds["pedKeys"]["keys"][i]["value"]["z"];
				w = ds["pedKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::PedKey, x, y, z, w, index);
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

				index = ds["softKeys"]["cards"][i]["index"];
				x = ds["softKeys"]["keys"][i]["value"]["x"];
				y = ds["softKeys"]["keys"][i]["value"]["y"];
				z = ds["softKeys"]["keys"][i]["value"]["z"];
				w = ds["softKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::SoftKey, x, y, z, w, index);
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

				index = ds["touchScreenKeys"]["cards"][i]["index"];
				x = ds["touchScreenKeys"]["keys"][i]["value"]["x"];
				y = ds["touchScreenKeys"]["keys"][i]["value"]["y"];
				z = ds["touchScreenKeys"]["keys"][i]["value"]["z"];
				w = ds["touchScreenKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::TouchScreenKey, x, y, z, w, index);
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

				index = ds["assistKeys"]["cards"][i]["index"];
				x = ds["assistKeys"]["keys"][i]["value"]["x"];
				y = ds["assistKeys"]["keys"][i]["value"]["y"];
				z = ds["assistKeys"]["keys"][i]["value"]["z"];
				w = ds["assistKeys"]["keys"][i]["value"]["w"];

				SetCoordinate(Type::AssistKey, x, y, z, w, index);
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

			//offset
			_smartCardPlaceStart = ds["smartCardPlaceStart"];
			_smartCardFetchOffset = ds["smartCardFetchOffset"];
			_smartCardReaderSlowInsertEnd = ds["smartCardReaderSlowInsertEnd"];
			_smartCardReleaseOffset = ds["smartCardReleaseOffset"];
			_smartCardInsertExtra = ds["smartCardInsertExtra"];

			//maximum
			_maximumX = ds["maximumX"];
			_maximumY = ds["maximumY"];
			_maximumZ = ds["maximumZ"];
			_maximumW = ds["maximumW"];

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

	//offset
	json = json + ", \"smartCardPlaceStart\":" + std::to_string(_smartCardPlaceStart);
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
		int fd;
		Poco::File storageFile(_filePathName);

		if(storageFile.exists()) {
			storageFile.remove(false);
		}

		fd = open(_filePathName.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
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

	case Type::Home:
	{
		value = _home;
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

void CoordinateStorage::SetSmartCardPlaceStartZ(long zPosition)
{
	_smartCardPlaceStart = zPosition;
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

bool CoordinateStorage::GetSmartCardPlaceStartZ(long & zPosition)
{
	if(_smartCardPlaceStart < 0) {
		return false;
	}

	zPosition = _smartCardPlaceStart;
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

void CoordinateStorage::setMaximumY(long value)
{
	_maximumY = value;
}

void CoordinateStorage::SetMaximumZ(long value)
{
	_maximumZ = value;
}

void CoordinateStorage::setMaximumW(long value)
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

