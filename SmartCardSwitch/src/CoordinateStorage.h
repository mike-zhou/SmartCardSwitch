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
		Home = 0,
		SmartCardGate = 1,
		SmartCard = 2,
		PedKeyGate = 3,
		PedKey = 4,
		PedKeyPressed = 5,
		SoftKeyGate = 6,
		SoftKey = 7,
		SoftKeyPressed = 8,
		AssistKeyGate = 9,
		AssistKey = 10,
		AssistKeyPressed = 11,
		TouchScreenKeyGate  = 12,
		TouchScreenKey = 13,
		TouchScreenKeyPressed = 14,
		SmartCardReaderGate = 15,
		SmartCardReader = 16,
		BarCodeReaderGate = 17,
		BarCodeReader = 18,
		ContactlessReaderGate = 19,
		ContactlessReader = 20
	};

	unsigned int SmartCardsAmount() { return _smartCards.size(); }
	unsigned int PedKeysAmount() { return _pedKeys.size(); }
	unsigned int SoftKeysAmount() { return _softKeys.size(); }
	unsigned int TouchScreenKeysAmount() { return _touchScreenKeys.size(); }
	unsigned int AssistKeysAmount() { return _assistKeys.size(); }

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

	void SetSmartCardPlaceStartZ(long zPosition);
	void SetSmartCardFetchOffset(long offset);
	void SetSmartCardReleaseOffsetZ(long offset);
	bool GetSmartCardPlaceStartZ(long & zPosition);
	bool GetSmartCardFetchOffset(long & offset);
	bool GetSmartCardReleaseOffset(long & offset);

	void SetSmartCardReaderSlowInsertEndY(long yPosition);
	bool GetSmartCardReaderSlowInsertEndY(long & yPosition);

private:
	//constraints
	const unsigned int SMART_CARDS_AMOUNT = 64;
	const unsigned int BAR_CODE_AMOUNT = 16;
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

	//home
	Coordinate _home;

	//smart cards
	Coordinate _smartCardGate;
	std::vector<Coordinate> _smartCards;
	long _smartCardPlaceStart;
	long _smartCardFetchOffset; //shared with card removal from smart card reader
	long _smartCardReleaseOffset;

	//PED keys
	Coordinate _pedKeyGate;
	std::vector<Coordinate> _pedKeys;
	std::vector<Coordinate> _pedKeysPressed;

	//soft keys
	Coordinate _softKeyGate;
	std::vector<Coordinate> _softKeys;
	std::vector<Coordinate> _softKeysPressed;


	//touch screen keys
	Coordinate _touchScreenKeyGate;
	std::vector<Coordinate> _touchScreenKeys;
	std::vector<Coordinate> _touchScreenKeysPressed;

	//assist keys
	Coordinate _assistKeyGate;
	std::vector<Coordinate> _assistKeys;
	std::vector<Coordinate> _assistKeysPressed;

	//smart card reader
	Coordinate _smartCardReaderGate;
	long _smartCardReaderSlowInsertEnd;
	Coordinate _smartCardReader;

	//contactless reader
	Coordinate _contactlessReaderGate;
	Coordinate _contactlessReader;

	//barcode reader
	Coordinate _barCodeReaderGate;
	Coordinate _barCodeReader;
};

#endif /* COORDINATESTORAGE_H_ */
