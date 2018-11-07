/*
 * CoordinateStorage.h
 *
 *  Created on: Nov 5, 2018
 *      Author: mikez
 */

#ifndef COORDINATESTORAGE_H_
#define COORDINATESTORAGE_H_
#include <string>
#include <vector>

class CoordinateStorage
{
public:
	CoordinateStorage(std::string filePathName);
	bool PersistToFile();

	enum Type
	{
		SmartCardEntry = 0,
		SmartCard,
		SmartCardExit,
		PedKeysEntry,
		PedKey,
		PedKeysExit,
		SoftKeysEntry,
		SoftKey,
		SoftKeysExit,
		AssistKeysEntry,
		AssistKey,
		AssistKeysExit,
		TouchScreenKeysEntry,
		TouchScreenKey,
		TouchScreenKeysExit,
		SmartCardSlotEntry,
		SmartCardSlot,
		SmartCardSlotExit,
		BarCodeReaderEntry,
		BarCodeReader,
		BarCodeReaderExit
	};

	unsigned int SmartCardsAmount() { return _smartCards.size(); }
	void SetSmartCardYOffset(long offset) { _smartCardFectchingYOffset = offset; }
	void SetSmartCardZoffset(long offset) { _smartCardAccessingZOffset = offset; }
	long SmartCardYOffset() { return _smartCardFectchingYOffset; }
	long SmartCardZOffset() { return _smartCardAccessingZOffset; }

	unsigned int PedKeysAmount() { return _pedKeys.size(); }
	void SetPedKeyPressingZOffset(long offset) { _pedKeyPressingZOffset = offset; }
	long PedKeyPressingZOffset() { return _pedKeyPressingZOffset; }

	unsigned int SoftKeysAmount() { return _softKeys.size(); }
	void SetSoftKeyPressingZOffset(long offset) { _softKeyPressingZOffset = offset; }
	long SoftKeyPressingZOffset() { return _softKeyPressingZOffset; }

	unsigned int TouchScreenKeysAmount() { return _touchScreenKeys.size(); }
	void SetTouchScreenKeyPressingZOffset(long offset) { _touchScreenKeyPressingZOffset = offset; }
	long TouchScreenKeyPressingZOffset() { return _touchScreenKeyPressingZOffset; }

	unsigned int AssistKeysAmount() { return _assistKeys.size(); }
	void SetAssistKeyPressingZOffset(long offset) { _assistKeyPressingZOffset = offset; }
	long AssistKeyPressingZOffset() { return _assistKeyPressingZOffset; }

	bool SetCoordinate(Type type,
					unsigned int x,
					unsigned int y,
					unsigned int z,
					unsigned int w,
					unsigned int index = 0);

	bool GetCoordinate(Type type,
					int& x,
					int& y,
					int& z,
					int& w,
					unsigned int index = 0);

private:
	const int DEFAULT_OFFSET = 0;
	//constraints
	const unsigned int SMART_CARDS_AMOUNT = 64;
	const unsigned int PED_KEYS_AMOUNT = 15;
	const unsigned int SOFT_KEYS_AMOUNT = 8;
	const unsigned int TOUCH_SCREEN_KEYS_AMOUNT = 8;
	const unsigned int ASSIST_KEYS_AMOUNT = 9;

	std::string _filePathName;

	struct Coordinate
	{
		long x, y, z, w;

		Coordinate();
		std::string ToJsonObj(); //return a json object standing for this struct.
	};

	//smart cards
	Coordinate _smartCardsEntry;
	std::vector<Coordinate> _smartCards;
	Coordinate _smartCardsExit;
	long _smartCardFectchingYOffset;
	long _smartCardAccessingZOffset;

	//PED keys
	Coordinate _pedKeysEntry;
	std::vector<Coordinate> _pedKeys;
	Coordinate _pedKeysExit;
	long _pedKeyPressingZOffset;

	//soft keys
	Coordinate _softKeysEntry;
	std::vector<Coordinate> _softKeys;
	Coordinate _softKeysExit;
	long _softKeyPressingZOffset;

	//touch screen keys
	Coordinate _touchScreenKeysEntry;
	std::vector<Coordinate> _touchScreenKeys;
	Coordinate _touchScreenKeysExit;
	long _touchScreenKeyPressingZOffset;

	//assist keys
	Coordinate _assistKeysEntry;
	std::vector<Coordinate> _assistKeys;
	Coordinate _assistKeysExit;
	long _assistKeyPressingZOffset;

	//smart card slot
	Coordinate _smartCardSlotEntry;
	Coordinate _smartCardSlot;
	Coordinate _smartCardSlotExit;

	//contactless reader
	Coordinate _contactlessReaderEntry;
	Coordinate _contactlessReader;
	Coordinate _contactlessReaderExit;

	//barcode reader
	Coordinate _barCodeReaderEntry;
	Coordinate _barCodeReader;
	Coordinate _barCodeReaderExit;
};

#endif /* COORDINATESTORAGE_H_ */
