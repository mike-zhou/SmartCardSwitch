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
		ScsNotIntialized,
		InvalidSmartCardNumber,
		SmartCardReaderOccupied,	//a smart card has already been in the smart card reader.
		SmartCardReaderEmpty,		//no smart card is in the smart card reader
		InvalidPedKeyNumber,
		InvalidSoftKeyNumber,
		InvalidAssistKeyNumber,
		InvalidTouchScreenKeyNumber,
		Failure
	};

	virtual ~ScsClient() {}

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
								const unsigned int portNumber) = 0;


	/**
	 * Shutdown ScsClient object
	 */
	virtual void Shutdown() = 0;

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
	virtual ScsResult InsertSmartCard(const unsigned int smartCardNumber) = 0;

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
	virtual ScsResult RemoveSmartCard(const unsigned int smartCardNumber) = 0;

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
	virtual ScsResult SwipeSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs) = 0;


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
	virtual ScsResult TapSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs) = 0;

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
	virtual ScsResult TapBarcode(const unsigned int smartCardNumber, const unsigned int pauseMs) = 0;

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
	virtual ScsResult PressPedKeys(const std::vector<unsigned int> pedNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) = 0;

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
	virtual ScsResult PressSoftKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) = 0;

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
	virtual ScsResult PressAssistKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) = 0;

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
	virtual ScsResult PressTouchScreenKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) = 0;

	/**
	 * Power on/off OPT
	 *
	 * Return value:
	 * 		Success
	 * 		ScsNotConnected
	 */
	virtual ScsResult PowerOnOpt(bool on) = 0;

	/**
	 * Power on/off ethernet switch
	 *
	 * Return value:
	 * 		Success
	 * 		ScsNotConnected
	 */
	virtual ScsResult PowerOnEthernetSwitch(bool on) = 0;

	/**
	 * Move the carriage to Home position.
	 */
	virtual ScsResult BackToHome() = 0;
};

/**
 * Return a singleton instance of ScsClient
 */
ScsClient * GetScsClientInstance();

#endif /* INCLUDE_SCSCLIENT_H_ */
