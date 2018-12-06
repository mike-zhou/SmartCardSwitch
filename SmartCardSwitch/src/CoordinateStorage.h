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
		SoftKeyGate = 5,
		SoftKey = 6,
		AssistKeyGate = 7,
		AssistKey = 8,
		TouchScreenKeyGate = 9,
		TouchScreenKey = 10,
		SmartCardReaderGate = 11,
		SmartCardReader = 12,
		BarCodeReaderGate = 13,
		BarCodeReader = 14,
		BarCodeCardGate = 15,
		BarCodeCard = 16,
		ContactlessReaderGate = 17,
		contactlessReader = 18
	};

	unsigned int SmartCardsAmount() { return _smartCards.size(); }
	unsigned int BarCodeCardsAmout() { return _barCodeCards.size(); }
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

	void SetSmartCardFetchStart(long zPosition);
	void SetSmartCardPlaceStart(long zPosition);
	void SetSmartCardAccessEnd(long zPosition);
	bool GetSmartCardFetchStart(long & zPosition);
	bool GetSmartCardPlaceStart(long & zPosition);
	bool GetSmartCardAccessEnd(long & zPosition);

	void SetSmartCardReaderSlowInsertStart(long yPosition);
	void SetSmartCardReaderSlowInsertEnd(long yPosition);
	void SetSmartCardReaderRemovalStart(long yPosition);
	bool GetSmartCardReaderSlowInsertStart(long & yPosition);
	bool GetSmartCardReaderSlowInsertEnd(long & yPosition);
	bool GetSmartCardReaderRemovalStart(long & yPosition);

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
	long _smartCardFetchStart;
	long _smartCardPlaceStart;
	long _smartCardAccessEnd;

	//PED keys
	Coordinate _pedKeyGate;
	std::vector<Coordinate> _pedKeys;

	//soft keys
	Coordinate _softKeyGate;
	std::vector<Coordinate> _softKeys;

	//touch screen keys
	Coordinate _touchScreenKeyGate;
	std::vector<Coordinate> _touchScreenKeys;

	//assist keys
	Coordinate _assistKeyGate;
	std::vector<Coordinate> _assistKeys;

	//smart card reader
	Coordinate _smartCardReaderGate;
	long _smartCardReaderSlowInsertStart;
	long _smartCardReaderSlowInsertEnd;
	long _smartCardReaderRemovalStart;
	Coordinate _smartCardReader;

	//contactless reader
	Coordinate _contactlessReaderGate;
	Coordinate _contactlessReader;

	//barcode reader
	Coordinate _barCodeReaderGate;
	Coordinate _barCodeReader;

	//barcode cards
	Coordinate _barCodeCardGate;
	std::vector<Coordinate> _barCodeCards;
};

#endif /* COORDINATESTORAGE_H_ */
