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
	void ReloadCoordinate();

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
		ContactlessReader = 20,
		Safe = 21,
		BarCodeReaderExtraPosition = 22,
		MobileBarcodeGate = 23,
		MobileBarcodePosition = 24
	};

	unsigned int SmartCardsAmount() { return _smartCards.size(); }
	unsigned int PedKeysAmount() { return _pedKeys.size(); }
	unsigned int SoftKeysAmount() { return _softKeys.size(); }
	unsigned int TouchScreenKeysAmount() { return _touchScreenKeys.size(); }
	unsigned int AssistKeysAmount() { return _assistKeys.size(); }
	unsigned int BarcodeReaderExtraPositionsAmount() { return _barCodeReaderExtraPositions.size(); }

	int GetWAdjustment();
	void SetWAdjustment(int adjustment);

	bool SetCoordinate(Type type,
					unsigned int x,
					unsigned int y,
					unsigned int z,
					unsigned int w,
					unsigned int index = 0);

	bool SetCoordinateEx(Type type,
					unsigned int x,
					unsigned int y,
					unsigned int z,
					unsigned int w,
					unsigned int u,
					unsigned int index = 0);

	bool GetCoordinate(Type type,
					int& x,
					int& y,
					int& z,
					int& w,
					unsigned int index = 0);

	bool GetCoordinateEx(Type type,
					int& x,
					int& y,
					int& z,
					int& w,
					int& u,
					unsigned int index = 0);

	void SetSmartCardSlowlyPlaceStartZ(long zPosition);
	void SetSmartCardSlowlyPlaceEndZ(long zPosition);
	void SetSmartCardFetchOffset(long offset);
	void SetSmartCardReleaseOffsetZ(long offset);
	void SetSmartCardInsertExtra(long offset);
	bool GetSmartCardSlowlyPlaceStartZ(long & zPosition);
	bool GetSmartCardSlowlyPlaceEndZ(long & zPosition);
	bool GetSmartCardFetchOffset(long & offset);
	bool GetSmartCardReleaseOffset(long & offset);
	bool GetSmartCardInsertExtra(long & offset);

	void SetSmartCardReaderSlowInsertEndY(long yPosition);
	bool GetSmartCardReaderSlowInsertEndY(long & yPosition);

	void SetMaximumX(long value);
	void SetMaximumY(long value);
	void SetMaximumZ(long value);
	void SetMaximumW(long value);
	bool GetMaximumX(long & value);
	bool GetMaximumY(long & value);
	bool GetMaximumZ(long & value);
	bool GetMaximumW(long & value);

	bool SetSmartCardOffset(unsigned int index, int offset);
	bool GetSmartCardOffset(unsigned int index, int& offset);

private:
	//constraints
	const unsigned int SMART_CARDS_AMOUNT = 128;
	const unsigned int BAR_CODE_READER_EXTRA_POSITION_AMOUNT = 128;
	const unsigned int PED_KEYS_AMOUNT = 15;
	const unsigned int SOFT_KEYS_AMOUNT = 8;
	const unsigned int TOUCH_SCREEN_KEYS_AMOUNT = 64;
	const unsigned int ASSIST_KEYS_AMOUNT = 9;
	const unsigned int MOBILE_BARCODE_POSITION_AMOUNT = 128;

	int _wAdjustment;

	std::string _filePathName;

	struct Coordinate
	{
		long x, y, z, w, u;

		Coordinate();
		std::string ToJsonObj(); //return a json object standing for this struct.
	};

	long _maximumX, _maximumY, _maximumZ, _maximumW;

	//home
	Coordinate _home;

	//smart cards
	Coordinate _smartCardGate;
	std::vector<Coordinate> _smartCards;
	long _smartCardSlowlyPlaceStart;
	long _smartCardSlowlyPlaceEnd;
	long _smartCardFetchOffset; //shared with card removal from smart card reader
	long _smartCardReleaseOffset;
	long _smartCardInsertExtra;

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
	std::vector<Coordinate> _barCodeReaderExtraPositions;

	//mobile barcode
	Coordinate _mobileBarcodeGate;
	std::vector<Coordinate> _mobileBarcodePositions;

	//safe
	Coordinate _safe;

	//smart card offset
	std::vector<int> _smartCardOffsets;
};

#endif /* COORDINATESTORAGE_H_ */
