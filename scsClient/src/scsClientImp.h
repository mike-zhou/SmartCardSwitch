/*
 * scsClientImp.h
 *
 *  Created on: Jan 22, 2019
 *      Author: mikez
 */

#ifndef SRC_SCSCLIENTIMP_H_
#define SRC_SCSCLIENTIMP_H_

#include "Poco/Mutex.h"
#include "Poco/TaskManager.h"

#include "Logger.h"
#include "scsClient.h"

class ScsClientImp: public ScsClient
{
public:
	static ScsClient * GetInstance();

	virtual ~ScsClientImp();

private:
	ScsClientImp();

	virtual ScsResult Initialize(const std::string& logFolder,
			const unsigned int fileSizeMB,
			const unsigned int fileAmount,
			const std::string & ipAddr,
			const unsigned int portNumber) override;

	virtual void Shutdown() override;

	virtual ScsResult InsertSmartCard(const unsigned int smartCardNumber) override;
	virtual ScsResult RemoveSmartCard(const unsigned int smartCardNumber) override;
	virtual ScsResult SwipeSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs) override;
	virtual ScsResult TapSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs) override;
	virtual ScsResult TapBarcode(const unsigned int smartCardNumber, const unsigned int pauseMs) override;
	virtual ScsResult PressPedKeys(const std::vector<unsigned int> pedNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) override;
	virtual ScsResult PressSoftKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) override;
	virtual ScsResult PressAssistKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) override;
	virtual ScsResult PressTouchScreenKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs) override;

private:
	static ScsClientImp * _pInstance;
	static Poco::Mutex _mutex;

	Logger * _pLogger;
	Poco::TaskManager _taskManager;

	std::string _ipAddr;
	unsigned int _portNumber;

	std::string sendCommand(const std::string & command);
};

#endif /* SRC_SCSCLIENTIMP_H_ */