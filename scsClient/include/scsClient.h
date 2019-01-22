/*
 * scsClient.h
 *
 *  Created on: Jan 22, 2019
 *      Author: mikez
 */

#ifndef INCLUDE_SCSCLIENT_H_
#define INCLUDE_SCSCLIENT_H_

#include <string>
#include <vector>

/**
 * Smart Card Switch Client
 */
class ScsClient
{
public:

	enum class ScsResult
	{
		Succeess = 0,
		InstanceWasInitialized, //SCS device has already been initialized.
		InvalidIpAddress,
		ScsNotAvailable, 	//no SCS device is listening to IP:port
		ScsOccupied,		//another client has already connected to this device.
		ScsNotConnected,	//no SCS device has been connected to.
		InvalidSmartCardNumber,
		SmartCardReaderOccupied,	//a smart card has already been in the smart card reader.
		SmartCardReaderEmpty,		//no smart card is in the smart card reader
		InvalidPedKeyNumber,
		InvalidSoftKeyNumber,
		InvalidAssistKeyNumber,
		InvalidTouchScreenKeyNumber,
		Failure
	};

	/**
	 * Initialize ScsClient object with path of log folder
	 *
	 * Parameter:
	 * 		logFolder: a string representing log folder
	 * 		fileSizeMB: size in MB of log file
	 * 		fileAmount: file amount of ScsClient logs
	 * 		ip: ip address of SCS device
	 * 		portNumber: port number the SCS device is listening to
	 *
	 * Return value:
	 * 		Succeess
	 * 		InvalidIpAddress
	 * 		InstanceWasInitialized
	 */
	virtual ScsResult Initialize(const std::string& logFolder,
								const unsigned int fileSizeMB,
								const unsigned int fileAmount,
								const std::string & ipAddr,
								const unsigned int portNumber);


	/**
	 * Shutdown ScsClient object
	 */
	virtual void Shutdown();

	/**
	 * Insert the designated smart card
	 *
	 * Parameter:
	 * 		smartCardNumber
	 *
	 * Return value:
	 * 		Success
	 * 		ScsNotConnected
	 * 		InvalidSmartCardNumber
	 * 		SmartCardReaderOccupied
	 */
	virtual ScsResult InsertSmartCard(const unsigned int smartCardNumber);

	/**
	 * Remove smart card from smart card reader and return it to smart card bay
	 *
	 * Parameter:
	 * 		smartCardNumber
	 *
	 * Return value:
	 * 		Success
	 * 		ScsNotConnected
	 * 		InvalidSmartCardNumber
	 * 		SmartCardReaderEmpty
	 */
	virtual ScsResult RemoveSmartCard(const unsigned int smartCardNumber);

	/**
	 * Swipe the designated smart card (insert and extract the smart card)
	 *
	 * Parameters:
	 * 		smartCardNumber
	 * 		pauseMs: the period of milliseconds before smart card is extracted from the smart card reader
	 *
	 * Return value:
	 * 		Success
	 * 		ScsNotConnected
	 * 		InvalidSmartCardNumber
	 * 		SmartCardReaderOccupied
	 */
	virtual ScsResult SwipeSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs);


	/**
	 * Tap the designated contactless smart card
	 *
	 * Parameters:
	 * 		smartCardNumber
	 * 		pauseMs: the period of milliseconds before smart card is removed from the contactless reader
	 *
	 * Return value:
	 * 		Success
	 * 		ScsNotConnected
	 * 		InvalidSmartCardNumber
	 */
	virtual ScsResult TapSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs);

	/**
	 * Tap the designated smart card with barcode
	 *
	 * Parameters:
	 * 		smartCardNumber
	 * 		pauseMs: the period of milliseconds before smart card is removed from the barcode reader
	 *
	 * Return value:
	 * 		Success
	 * 		ScsNotConnected
	 * 		InvalidSmartCardNumber
	 */
	virtual ScsResult TapBarcode(const unsigned int smartCardNumber, const unsigned int pauseMs);

	/**
	 * Press PED keys
	 *
	 * Parameters:
	 * 		pedNumbers: array of PED keys to be pressed
	 * 		upPeriodMs: interval in millisecond between 2 key pressing
	 * 		downPeriodMs: interval in millisecond at which the key should be kept pressed
	 *
	 * Return value:
	 * 		Success
	 * 		InvalidPedKeyNumber
	 */
	virtual ScsResult PressPedKeys(const std::vector<unsigned int> pedNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs);

	/**
	 * Press soft keys
	 *
	 * Parameters:
	 * 		keyNumbers: array of soft keys to be pressed
	 * 		upPeriodMs: interval in millisecond between 2 key pressing
	 * 		downPeriodMs: interval in millisecond at which the key should be kept pressed
	 *
	 * Return value:
	 * 		Success
	 * 		InvalidSoftKeyNumber
	 */
	virtual ScsResult PressSoftKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs);

	/**
	 * Press Assist keys
	 *
	 * Parameters:
	 * 		keyNumbers: array of assist keys to be pressed
	 * 		upPeriodMs: interval in millisecond between 2 key pressing
	 * 		downPeriodMs: interval in millisecond at which the key should be kept pressed
	 *
	 * Return value:
	 * 		Success
	 * 		InvalidAssistKeyNumber
	 */
	virtual ScsResult PressAssistKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs);

	/**
	 * Press touch screen keys
	 *
	 * Parameters:
	 * 		keyNumbers: array of touch screen keys to be pressed
	 * 		upPeriodMs: interval in millisecond between 2 key pressing
	 * 		downPeriodMs: interval in millisecond at which the key should be kept pressed
	 *
	 * Return value:
	 * 		Success
	 * 		InvalidTouchScreenKeyNumber
	 */
	virtual ScsResult PressTouchScreenKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs);

protected:
	ScsClient() {}
	~ScsClient() {}
};

ScsClient * GetScsClientInstance();

#endif /* INCLUDE_SCSCLIENT_H_ */
