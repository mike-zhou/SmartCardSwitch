/*
 * CommandRunner.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: user1
 */

#include "stdio.h"
#include <iostream>
#include "CommandRunner.h"
#include "Logger.h"
#include "CommandFactory.h"
#include "ReplyTranslator.h"
#include "Command.h"
#include "CoordinateStorage.h"
#include "MovementConfiguration.h"

extern Logger * pLogger;
extern CoordinateStorage * pCoordinateStorage;
extern MovementConfiguration * pMovementConfiguration;

CommandRunner::CommandRunner(): Task("CommandRunner")
{
	_userCommand.state = UserCommand::CommandState::IDLE;
	//device power status
	_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::UNKNOWN;
	_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::UNKNOWN;
	//BDCs status
	_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::UNKNOWN;
	for(unsigned int i = 0; i < BDC_AMOUNT; i++) {
		_userCommand.resultBdcStatus[i] = IResponseReceiver::BdcStatus::UNKNOWN;
	}
	//steppers
	_userCommand.resultSteppersPowerStatus = UserCommand::PowerStatus::UNKNOWN;
	_userCommand.resultStepperClkResolution = -1;
	for(unsigned int i = 0; i < STEPPER_AMOUNT; i++)
	{
		auto& status = _userCommand.resultStepperStatus[i];

		status.state = std::string();//empty string by default
		status.enabled = UserCommand::StepperEnableStatus::UNKOWN;
		status.forward = UserCommand::StepperDirectionStatus::UNKNOWN;
		status.locatorIndex = -1;
		status.locatorLineNumberStart = -1;
		status.locatorLineNumberTerminal = -1;
		status.homeOffset = -1;
		status.lowClks = -1;
		status.highClks = -1;
		status.accelerationBuffer = -1;
		status.accelerationBufferDecrement = -1;
		status.decelerationBuffer = -1;
		status.decelerationBufferIncrement = -1;
	}
	//locators
	for(unsigned int i = 0; i < LOCATOR_AMOUNT; i++) {
		_userCommand.resultLocatorStatus[i] = -1;
	}

	_pDeviceAccessor = nullptr;
}

void CommandRunner::SetDevice(DeviceAccessor * pDeviceAccessor)
{
	_pDeviceAccessor = pDeviceAccessor;
}

void CommandRunner::OnFeedback(const std::string& feedback)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	//append to end
	_feedbacks.push_back(feedback);
	pLogger->LogDebug("CommandRunner::OnFeedback feedback: " + feedback);
	_event.set();
}

void CommandRunner::runTask()
{
	while(1)
	{
		if(isCancelled())
		{
			break;
		}
		else
		{
			if(_event.tryWait(10) == false) {
				continue;
			}
			else
			{
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);
				processFeedbacks();
			}
		}
	}

	pLogger->LogInfo("CommandRunner::runTask exited");
}

void CommandRunner::saveMovementConfig()
{
	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		auto& stepper = _userCommand.resultStepperStatus[i];

		pMovementConfiguration->SetStepperGeneral(i,
			stepper.lowClks,
			stepper.highClks,
			stepper.accelerationBuffer,
			stepper.accelerationBufferDecrement,
			stepper.decelerationBuffer,
			stepper.decelerationBufferIncrement);

		pMovementConfiguration->SetStepperBoundary(i,
				stepper.locatorIndex,
				stepper.locatorLineNumberStart,
				stepper.locatorLineNumberTerminal);

	}

	if(pMovementConfiguration->PersistToFile()) {
		pLogger->LogInfo("CommandRunner::saveMovementConfig succeeded");
	}
	else {
		pLogger->LogInfo("CommandRunner::saveMovementConfig failed");
	}
}

bool CommandRunner::isCorrespondingReply(const std::string& commandKey, unsigned short commandId)
{
	bool bCorrespondingReply = false;

	if(_userCommand.state != UserCommand::CommandState::COMMAND_SENT) {
		pLogger->LogError("CommandRunner::isCorrespondingReply obsolete reply");
	}
	else if(_userCommand.commandKey != commandKey) {
		pLogger->LogError("CommandRunner::isCorrespondingReply command key doesn't match, original: '" + _userCommand.commandKey + "', key in reply: '" + commandKey + "'");
	}
	else if((_userCommand.commandId & 0xffff) != commandId) {
		pLogger->LogError("CommandRunner::isCorrespondingReply commandId doesn't match, original: " + std::to_string(_userCommand.commandId) + ", cmdId in reply: " + std::to_string(commandId));
	}
	else {
		bCorrespondingReply = true;
	}

	return bCorrespondingReply;
}

void CommandRunner::onFeedbackDevicesGet(std::shared_ptr<ReplyTranslator::ReplyDevicesGet> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	//update device list
	_userCommand.resultDevices.clear();
	if(replyPtr->devices.empty()) {
		pLogger->LogError("CommandRunner::onFeedbackDevicesGet no devices in: " + replyPtr->originalString);
	}
	else {
		int number = 0;
		for(auto it=replyPtr->devices.begin(); it!=replyPtr->devices.end(); it++, number++) {
			_userCommand.resultDevices.push_back(*it);
			pLogger->LogInfo("CommandRunner::onFeedbackDevicesGet device: " + std::to_string(number) + ". " + *it);
		}
	}

	_userCommand.state = UserCommand::CommandState::SUCCEEDED;

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnDevicesGet(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultDevices);
	}
}

void CommandRunner::onFeedbackDeviceConnect(std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->connected) {
		pLogger->LogInfo("CommandRunner::onFeedbackDeviceConnect connected to device: " + replyPtr->deviceName);
		_userCommand.resultConnectedDeviceName = replyPtr->deviceName;
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		pLogger->LogInfo("CommandRunner::onFeedbackDeviceConnect couldn't connect to device: " + replyPtr->deviceName + " reason: " + replyPtr->reason);
		_userCommand.resultConnectedDeviceName.clear();
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnDeviceConnect(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackDeviceQueryPower(std::shared_ptr<ReplyTranslator::ReplyDeviceQueryPower> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->bPoweredOn) {
			pLogger->LogInfo("CommandRunner::onFeedbackDeviceQueryPower device is powered on");
			_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::POWERED_ON;
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackDeviceQueryPower device is powered off");
			_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::POWERED_OFF;
		}

		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		//error happened
		pLogger->LogError("CommandRunner::onFeedbackDeviceQueryPower unknown device power status due to: " + replyPtr->errorInfo);
		_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::UNKNOWN;
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnDeviceQueryPower(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultDevicePowerStatus == UserCommand::PowerStatus::POWERED_ON);
	}
}

void CommandRunner::onFeedbackDeviceQueryFuse(std::shared_ptr<ReplyTranslator::ReplyDeviceQueryFuse> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->bFuseOn) {
			pLogger->LogInfo("CommandRunner::onFeedbackDeviceQueryFuse main fuse is on");
			_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::FUSE_ON;
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackDeviceQueryFuse main fuse is off");
			_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::FUSE_OFF;
		}

		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		//error happened
		pLogger->LogError("CommandRunner::onFeedbackDeviceQueryFuse unknown main fuse status due to: " + replyPtr->errorInfo);
		_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::UNKNOWN;
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnDeviceQueryFuse(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultDeviceFuseStatus == UserCommand::FuseStatus::FUSE_ON);
	}
}

void CommandRunner::onFeedbackBdcsPowerOn(std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		pLogger->LogInfo("CommandRunner::onFeedbackBdcsPowerOn bdcs is powered on");
		_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_ON;

		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcsPowerOn error: " + replyPtr->errorInfo);
		//keep bdcs power status unchanged.
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcsPowerOn(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackBdcsPowerOff(std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		pLogger->LogInfo("CommandRunner::onFeedbackBdcsPowerOff bdcs is powered off");
		_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_OFF;

		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcsPowerOff error: " + replyPtr->errorInfo);
		//keep bdcs power status unchanged.
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcsPowerOff(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackBdcsQueryPower(std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->bPoweredOn == true)
		{
			if(_userCommand.resultBdcsPowerStatus != UserCommand::PowerStatus::POWERED_ON) {
				pLogger->LogError("CommandRunner::onFeedbackBdcsQueryPower status doesn't match: queried state: on; saved state: " +
						std::string((_userCommand.resultSteppersPowerStatus == UserCommand::PowerStatus::UNKNOWN)?"UNKNOWN":"POWERED_OFF"));
			}
			_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_ON;
		}
		else
		{
			if(_userCommand.resultSteppersPowerStatus != UserCommand::PowerStatus::POWERED_OFF) {
				pLogger->LogError("CommandRunner::onFeedbackBdcsQueryPower status doesn't match: queried state: off; saved state: " +
						std::string((_userCommand.resultSteppersPowerStatus == UserCommand::PowerStatus::UNKNOWN)?"UNKNOWN":"POWERED_ON"));
			}
			_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_OFF;
		}

		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcsQueryPower error: " + replyPtr->errorInfo);
		//keep bdcs power status unchanged.
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcsQueryPower(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultBdcsPowerStatus == UserCommand::PowerStatus::POWERED_ON);
	}
}

void CommandRunner::onFeedbackBdcCoast(std::shared_ptr<ReplyTranslator::ReplyBdcCoast> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackBdcCoast index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackBdcCoast index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::COAST;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcCoast error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcCoast(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackBdcReverse(std::shared_ptr<ReplyTranslator::ReplyBdcReverse> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackBdcReverse index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackBdcReverse index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::REVERSE;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcReverse error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcReverse(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackBdcForward(std::shared_ptr<ReplyTranslator::ReplyBdcForward> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackBdcForward index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackBdcForward index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::FORWARD;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcForward error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcForward(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackBdcBreak(std::shared_ptr<ReplyTranslator::ReplyBdcBreak> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackBdcBreak index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackBdcBreak index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::BREAK;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcBreak error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcBreak(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackBdcQuery(std::shared_ptr<ReplyTranslator::ReplyBdcQuery> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackBdcQuery index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackBdcQuery index: " + std::to_string(replyPtr->index) + " mode: " + std::to_string(replyPtr->index));

			switch(replyPtr->mode)
			{
				case ReplyTranslator::ReplyBdcQuery::BdcMode::COAST:
					_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::COAST;
					break;
				case ReplyTranslator::ReplyBdcQuery::BdcMode::REVERSE:
					_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::REVERSE;
					break;
				case ReplyTranslator::ReplyBdcQuery::BdcMode::FORWARD:
					_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::FORWARD;
					break;
				case ReplyTranslator::ReplyBdcQuery::BdcMode::BREAK:
					_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::BREAK;
					break;
				default:
					pLogger->LogError("CommandRunner::onFeedbackBdcQuery unknown bdc status");
					_userCommand.resultBdcStatus[replyPtr->index] = IResponseReceiver::BdcStatus::UNKNOWN;
					break;
			}
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackBdcQuery error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnBdcQuery(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultBdcStatus[replyPtr->index]);
	}
}

void CommandRunner::onFeedbackSteppersPowerOn(std::shared_ptr<ReplyTranslator::ReplySteppersPowerOn> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty()) {
		pLogger->LogInfo("CommandRunner::onFeedbackSteppersPowerOn steppers are powered on");
		_userCommand.resultSteppersPowerStatus = UserCommand::PowerStatus::POWERED_ON;
		success = true;
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackSteppersPowerOn error: " + replyPtr->errorInfo);
		//keep stepper statatus unchanged.
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnSteppersPowerOn(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackSteppersPowerOff(std::shared_ptr<ReplyTranslator::ReplySteppersPowerOff> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty()) {
		pLogger->LogInfo("CommandRunner::onFeedbackSteppersPowerOff steppers are powered off");
		_userCommand.resultSteppersPowerStatus = UserCommand::PowerStatus::POWERED_OFF;
		success = true;
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackSteppersPowerOff error: " + replyPtr->errorInfo);
		//keep stepper statatus unchanged.
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnSteppersPowerOff(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackSteppersQueryPower(std::shared_ptr<ReplyTranslator::ReplySteppersQueryPower> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->bPowered == true)
		{
			if(_userCommand.resultSteppersPowerStatus != UserCommand::PowerStatus::POWERED_ON) {
				pLogger->LogError("CommandRunner::onFeedbackSteppersQueryPower status doesn't match: queried state: on; saved state: " +
						std::string((_userCommand.resultSteppersPowerStatus == UserCommand::PowerStatus::UNKNOWN)?"UNKNOWN":"POWERED_OFF"));
			}
			_userCommand.resultSteppersPowerStatus = UserCommand::PowerStatus::POWERED_ON;
		}
		else
		{
			if(_userCommand.resultSteppersPowerStatus != UserCommand::PowerStatus::POWERED_OFF) {
				pLogger->LogError("CommandRunner::onFeedbackSteppersQueryPower status doesn't match: queried state: off; saved state: " +
						std::string((_userCommand.resultSteppersPowerStatus == UserCommand::PowerStatus::UNKNOWN)?"UNKNOWN":"POWERED_ON"));
			}
			_userCommand.resultSteppersPowerStatus = UserCommand::PowerStatus::POWERED_OFF;
		}
		success = true;
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackSteppersQueryPower error: " + replyPtr->errorInfo);
		//keep stepper statatus unchanged.
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnSteppersQueryPower(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultSteppersPowerStatus == UserCommand::PowerStatus::POWERED_ON);
	}
}

void CommandRunner::onFeedbackStepperQueryResolution(std::shared_ptr<ReplyTranslator::ReplyStepperQueryResolution> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		pLogger->LogInfo("CommandRunner::onFeedbackStepperQueryResolution stepper resolution: " + std::to_string(replyPtr->resolutionUs));
		_userCommand.resultStepperClkResolution = replyPtr->resolutionUs;
		success = true;
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperQueryResolution error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperQueryResolution(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultStepperClkResolution);
	}
}

void CommandRunner::onFeedbackStepperConfigStep(std::shared_ptr<ReplyTranslator::ReplyStepperConfigStep> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperConfigStep index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperConfigStep wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackStepperConfigStep index: " + std::to_string(replyPtr->index) +
					", lowClks: " + std::to_string(_userCommand.lowClks) +
					", highClks: " + std::to_string(_userCommand.highClks));
			_userCommand.resultStepperStatus[replyPtr->index].lowClks = _userCommand.lowClks;
			_userCommand.resultStepperStatus[replyPtr->index].highClks = _userCommand.highClks;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperConfigStep error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperConfigStep(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperAccelerationBuffer(std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBuffer> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperAccelerationBuffer index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperAccelerationBuffer wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackStepperAccelerationBuffer index: " + std::to_string(replyPtr->index) +
					", value: " + std::to_string(_userCommand.accelerationBuffer));
			_userCommand.resultStepperStatus[replyPtr->index].accelerationBuffer = _userCommand.accelerationBuffer;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperAccelerationBuffer error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperAccelerationBuffer(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperAccelerationBufferDecrement(std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBufferDecrement> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperAccelerationBufferDecrement index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperAccelerationBufferDecrement wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackStepperAccelerationBufferDecrement index: " + std::to_string(replyPtr->index) +
					", value: " + std::to_string(_userCommand.accelerationBufferDecrement));
			_userCommand.resultStepperStatus[replyPtr->index].accelerationBufferDecrement = _userCommand.accelerationBufferDecrement;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperAccelerationBufferDecrement error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperAccelerationBufferDecrement(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperDecelerationBuffer(std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBuffer> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperDecelerationBuffer index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperDecelerationBuffer wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackStepperDecelerationBuffer index: " + std::to_string(replyPtr->index) +
					", value: " + std::to_string(_userCommand.decelerationBuffer));
			_userCommand.resultStepperStatus[replyPtr->index].decelerationBuffer = _userCommand.decelerationBuffer;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperDecelerationBuffer error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperDecelerationBuffer(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperDecelerationBufferIncrement(std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBufferIncrement> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperDecelerationBufferIncrement index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperDecelerationBufferIncrement wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackStepperDecelerationBufferIncrement index: " + std::to_string(replyPtr->index) +
					", value: " + std::to_string(_userCommand.decelerationBufferIncrement));
			_userCommand.resultStepperStatus[replyPtr->index].decelerationBufferIncrement = _userCommand.decelerationBufferIncrement;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperDecelerationBufferIncrement error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperDecelerationBufferIncrement(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperEnable(std::shared_ptr<ReplyTranslator::ReplyStepperEnable> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperEnable index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperEnable wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			if(replyPtr->enabled) {
				pLogger->LogInfo("CommandRunner::onFeedbackStepperEnable enabled index: " + std::to_string(replyPtr->index));
				_userCommand.resultStepperStatus[replyPtr->index].enabled = UserCommand::StepperEnableStatus::ENABLED;
			}
			else {
				pLogger->LogInfo("CommandRunner::onFeedbackStepperEnable disabled index: " + std::to_string(replyPtr->index));
				_userCommand.resultStepperStatus[replyPtr->index].enabled = UserCommand::StepperEnableStatus::DISABLED;
			}
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperEnable error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperEnable(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperForward(std::shared_ptr<ReplyTranslator::ReplyStepperForward> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperForward index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperForward wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			if(replyPtr->forward) {
				pLogger->LogInfo("CommandRunner::onFeedbackStepperForward forward index: " + std::to_string(replyPtr->index));
				_userCommand.resultStepperStatus[replyPtr->index].forward = UserCommand::StepperDirectionStatus::FORWORD;
			}
			else {
				pLogger->LogInfo("CommandRunner::onFeedbackStepperForward reverse index: " + std::to_string(replyPtr->index));
				_userCommand.resultStepperStatus[replyPtr->index].forward = UserCommand::StepperDirectionStatus::REVERSE;
			}
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperForward error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperForward(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperSteps(std::shared_ptr<ReplyTranslator::ReplyStepperSteps> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperSteps index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperSteps wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackStepperSteps index: " + std::to_string(replyPtr->index) + ", steps: " + std::to_string(_userCommand.steps));
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperSteps error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperSteps(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperRun(std::shared_ptr<ReplyTranslator::ReplyStepperRun> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperRun index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperRun wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else
		{
			if(replyPtr->position == _userCommand.finalPosition) {
				pLogger->LogInfo("CommandRunner::onFeedbackStepperRun succeed, index: " + std::to_string(replyPtr->index) + ", position: " + std::to_string(replyPtr->position));
				success = true;
			}
			else {
				pLogger->LogInfo("CommandRunner::onFeedbackStepperRun failed, index: " + std::to_string(replyPtr->index) +
						", position: " + std::to_string(replyPtr->position) +
						", expected position: " + std::to_string(_userCommand.finalPosition));
				success = false;
			}
			_userCommand.resultStepperStatus[replyPtr->index].homeOffset = replyPtr->position;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperRun error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperRun(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperConfigHome(std::shared_ptr<ReplyTranslator::ReplyStepperConfigHome> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperConfigHome index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperConfigHome wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else
		{
			pLogger->LogInfo("CommandRunner::onFeedbackStepperConfigHome succeed index: " + std::to_string(replyPtr->index) +
					", locatorIndex: " + std::to_string(_userCommand.locatorIndex) +
					", lineNumberStart: " + std::to_string(_userCommand.locatorLineNumberStart) +
					", lineNumberTerminal: " + std::to_string(_userCommand.locatorLineNumberTerminal));

			_userCommand.resultStepperStatus[replyPtr->index].locatorIndex = _userCommand.locatorIndex;
			_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberStart = _userCommand.locatorLineNumberStart;
			_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberTerminal = _userCommand.locatorLineNumberTerminal;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperConfigHome error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperConfigHome(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackStepperMove(std::shared_ptr<ReplyTranslator::ReplyStepperMove> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	pLogger->LogError("CommandRunner::onFeedbackStepperMove not implemented");
	_userCommand.state = UserCommand::CommandState::FAILED;
}

void CommandRunner::onFeedbackStepperQuery(std::shared_ptr<ReplyTranslator::ReplyStepperQuery> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperQuery index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperQuery wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else
		{
			pLogger->LogInfo("CommandRunner::onFeedbackStepperQuery succeed index: " + std::to_string(replyPtr->index) +
					", state: " + replyPtr->state +
					", enabled: " + std::string((replyPtr->bEnabled)?"true":"false") +
					", forward: " + std::string((replyPtr->bForward)?"true":"false") +
					", locatorIndex: " + std::to_string(replyPtr->locatorIndex) +
					", lineNumberStart: " + std::to_string(replyPtr->locatorLineNumberStart) +
					", lineNumberTerminal: " + std::to_string(replyPtr->locatorLineNumberTerminal) +
					", homeOffset: " + std::to_string(replyPtr->homeOffset) +
					", lowClks: " + std::to_string(replyPtr->lowClks) +
					", highClks: " + std::to_string(replyPtr->highClks) +
					", accelerationBuffer: " + std::to_string(replyPtr->accelerationBuffer) +
					", accelerationBufferDecrement: " + std::to_string(replyPtr->accelerationBufferDecrement) +
					", decelerationBuffer: " + std::to_string(replyPtr->decelerationBuffer) +
					", decelerationBufferIncrement: " + std::to_string(replyPtr->decelerationBufferIncrement));

			//state
			_userCommand.resultStepperStatus[replyPtr->index].state = replyPtr->state;

			//enabled
			if(replyPtr->bEnabled)
			{
				if(_userCommand.resultStepperStatus[replyPtr->index].enabled != UserCommand::StepperEnableStatus::ENABLED) {
					pLogger->LogError("CommandRunner::onFeedbackStepperQuery enabled mismatch: index: " + std::to_string(replyPtr->index) +
							", local value: " + std::string((_userCommand.resultStepperStatus[replyPtr->index].enabled == UserCommand::StepperEnableStatus::UNKOWN)?"UNKNOWN":"DISABLED"));

					_userCommand.resultStepperStatus[replyPtr->index].enabled = UserCommand::StepperEnableStatus::ENABLED;
				}
			}
			else
			{
				if(_userCommand.resultStepperStatus[replyPtr->index].enabled != UserCommand::StepperEnableStatus::DISABLED) {
					pLogger->LogError("CommandRunner::onFeedbackStepperQuery disabled mismatch: index: " + std::to_string(replyPtr->index) +
							", local value: " + std::string((_userCommand.resultStepperStatus[replyPtr->index].enabled == UserCommand::StepperEnableStatus::UNKOWN)?"UNKNOWN":"ENABLED"));

					_userCommand.resultStepperStatus[replyPtr->index].enabled = UserCommand::StepperEnableStatus::DISABLED;
				}
			}

			//forward
			if(replyPtr->bForward)
			{
				if(_userCommand.resultStepperStatus[replyPtr->index].forward != UserCommand::StepperDirectionStatus::FORWORD) {
					pLogger->LogError("CommandRunner::onFeedbackStepperQuery forward mismatch: index: " + std::to_string(replyPtr->index) +
							", local value: " + std::string((_userCommand.resultStepperStatus[replyPtr->index].forward == UserCommand::StepperDirectionStatus::UNKNOWN)?"UNKNOWN":"REVERSE"));

					_userCommand.resultStepperStatus[replyPtr->index].forward = UserCommand::StepperDirectionStatus::FORWORD;
				}
			}
			else
			{
				if(_userCommand.resultStepperStatus[replyPtr->index].forward != UserCommand::StepperDirectionStatus::REVERSE) {
					pLogger->LogError("CommandRunner::onFeedbackStepperQuery reverse mismatch: index: " + std::to_string(replyPtr->index) +
							", local value: " + std::string((_userCommand.resultStepperStatus[replyPtr->index].forward == UserCommand::StepperDirectionStatus::UNKNOWN)?"UNKNOWN":"FORWARD"));

					_userCommand.resultStepperStatus[replyPtr->index].forward = UserCommand::StepperDirectionStatus::REVERSE;
				}
			}

			//locator index
			if(_userCommand.resultStepperStatus[replyPtr->index].locatorIndex != replyPtr->locatorIndex) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery locatorIndex mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->locatorIndex) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].locatorIndex));
			}
			_userCommand.resultStepperStatus[replyPtr->index].locatorIndex = replyPtr->locatorIndex;

			//line number start
			if(_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberStart != replyPtr->locatorLineNumberStart) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery locatorLineNumberStart mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->locatorLineNumberStart) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberStart));

			}
			_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberStart = replyPtr->locatorLineNumberStart;

			//line number terminal
			if(_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberTerminal != replyPtr->locatorLineNumberTerminal) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery locatorLineNumberTerminal mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->locatorLineNumberTerminal) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberTerminal));

			}
			_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberTerminal = replyPtr->locatorLineNumberTerminal;

			//home offset
			if(_userCommand.resultStepperStatus[replyPtr->index].homeOffset != replyPtr->homeOffset) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery homeOffset mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->homeOffset) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].homeOffset));

			}
			_userCommand.resultStepperStatus[replyPtr->index].homeOffset = replyPtr->homeOffset;

			//low clocks
			if(_userCommand.resultStepperStatus[replyPtr->index].lowClks != replyPtr->lowClks) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery lowClks mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->lowClks) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].lowClks));

			}
			_userCommand.resultStepperStatus[replyPtr->index].lowClks = replyPtr->lowClks;

			//high clocks
			if(_userCommand.resultStepperStatus[replyPtr->index].highClks != replyPtr->highClks) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery highClks mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->lowClks) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].highClks));

			}
			_userCommand.resultStepperStatus[replyPtr->index].highClks = replyPtr->highClks;

			//acceleration buffer
			if(_userCommand.resultStepperStatus[replyPtr->index].accelerationBuffer != replyPtr->accelerationBuffer) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery accelerationBuffer mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->lowClks) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].accelerationBuffer));

			}
			_userCommand.resultStepperStatus[replyPtr->index].accelerationBuffer = replyPtr->accelerationBuffer;

			//acceleration buffer decrement
			if(_userCommand.resultStepperStatus[replyPtr->index].accelerationBufferDecrement != replyPtr->accelerationBufferDecrement) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery accelerationBufferDecrement mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->accelerationBufferDecrement) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].accelerationBufferDecrement));

			}
			_userCommand.resultStepperStatus[replyPtr->index].accelerationBufferDecrement = replyPtr->accelerationBufferDecrement;

			//deceleration buffer
			if(_userCommand.resultStepperStatus[replyPtr->index].decelerationBuffer != replyPtr->decelerationBuffer) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery decelerationBuffer mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->decelerationBuffer) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].decelerationBuffer));

			}
			_userCommand.resultStepperStatus[replyPtr->index].decelerationBuffer = replyPtr->decelerationBuffer;

			//deceleration buffer increment
			if(_userCommand.resultStepperStatus[replyPtr->index].decelerationBufferIncrement != replyPtr->decelerationBufferIncrement) {
				pLogger->LogError("CommandRunner::onFeedbackStepperQuery decelerationBufferIncrement mismatch: index: " + std::to_string(replyPtr->index) +
						", value queried: " + std::to_string(replyPtr->decelerationBufferIncrement) +
						", local value: " + std::to_string(_userCommand.resultStepperStatus[replyPtr->index].decelerationBufferIncrement));

			}
			_userCommand.resultStepperStatus[replyPtr->index].decelerationBufferIncrement = replyPtr->decelerationBufferIncrement;

			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperQuery error: " + replyPtr->errorInfo);
	}

	IResponseReceiver::StepperState stepperState;

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;

		if(_userCommand.resultStepperStatus[replyPtr->index].state == StepperStateApproachingHomeLocator) {
			stepperState = IResponseReceiver::StepperState::ApproachingHomeLocator;
		}
		else if(_userCommand.resultStepperStatus[replyPtr->index].state == StepperStateLeavingHomeLocator) {
			stepperState = IResponseReceiver::StepperState::LeavingHomeLocator;
		}
		else if(_userCommand.resultStepperStatus[replyPtr->index].state == StepperStateGoingHome) {
			stepperState = IResponseReceiver::StepperState::GoingHome;
		}
		else if(_userCommand.resultStepperStatus[replyPtr->index].state == StepperStateAccelerating) {
			stepperState = IResponseReceiver::StepperState::Accelerating;
		}
		else if(_userCommand.resultStepperStatus[replyPtr->index].state == StepperStateCruising) {
			stepperState = IResponseReceiver::StepperState::Cruising;
		}
		else if(_userCommand.resultStepperStatus[replyPtr->index].state == StepperStateDecelerating) {
			stepperState = IResponseReceiver::StepperState::Decelerating;
		}
		else {
			stepperState = IResponseReceiver::StepperState::Unknown;
		}
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperQuery(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				stepperState,
				_userCommand.resultStepperStatus[replyPtr->index].enabled == UserCommand::StepperEnableStatus::ENABLED,
				_userCommand.resultStepperStatus[replyPtr->index].forward == UserCommand::StepperDirectionStatus::FORWORD,
				_userCommand.resultStepperStatus[replyPtr->index].locatorIndex,
				_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberStart,
				_userCommand.resultStepperStatus[replyPtr->index].locatorLineNumberTerminal,
				_userCommand.resultStepperStatus[replyPtr->index].homeOffset,
				_userCommand.resultStepperStatus[replyPtr->index].lowClks,
				_userCommand.resultStepperStatus[replyPtr->index].highClks,
				_userCommand.resultStepperStatus[replyPtr->index].accelerationBuffer,
				_userCommand.resultStepperStatus[replyPtr->index].accelerationBufferDecrement,
				_userCommand.resultStepperStatus[replyPtr->index].decelerationBuffer,
				_userCommand.resultStepperStatus[replyPtr->index].decelerationBufferIncrement);
	}
}

void CommandRunner::onFeedbackStepperSetState(std::shared_ptr<ReplyTranslator::ReplyStepperSetState> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackStepperSetState index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.stepperIndex) {
			pLogger->LogError("CommandRunner::onFeedbackStepperSetState wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.stepperIndex));
		}
		else {
			pLogger->LogInfo("CommandRunner::onFeedbackStepperSetState index: " + std::to_string(replyPtr->index) + ", steps: " + std::to_string(_userCommand.steps));
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackStepperSetState error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnStepperSetState(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED);
	}
}

void CommandRunner::onFeedbackLocatorQuery(std::shared_ptr<ReplyTranslator::ReplyLocatorQuery> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	bool success = false;

	if(replyPtr->errorInfo.empty())
	{
		if(replyPtr->index >= LOCATOR_AMOUNT) {
			pLogger->LogError("CommandRunner::onFeedbackLocatorQuery index out of range: " + std::to_string(replyPtr->index));
		}
		else if(replyPtr->index != _userCommand.locatorIndex) {
			pLogger->LogError("CommandRunner::onFeedbackLocatorQuery wrong index: " + std::to_string(replyPtr->index) + "; should be: " + std::to_string(_userCommand.locatorIndex));
		}
		else
		{
			pLogger->LogInfo("CommandRunner::onFeedbackLocatorQuery succeed index: " + std::to_string(replyPtr->index) +
					", locatorIndex: " + std::to_string(_userCommand.locatorIndex) +
					", lowInput: " + std::to_string(replyPtr->lowInput));

			if(replyPtr->lowInput != _userCommand.resultLocatorStatus[replyPtr->index]) {
				pLogger->LogError("CommandRunner::onFeedbackLocatorQuery status doesn't match: status queried: " + std::to_string(replyPtr->lowInput) +
						", local value: " + std::to_string(_userCommand.resultLocatorStatus[replyPtr->index]));
			}
			_userCommand.resultLocatorStatus[replyPtr->index] = replyPtr->lowInput;
			success = true;
		}
	}
	else {
		pLogger->LogError("CommandRunner::onFeedbackLocatorQuery error: " + replyPtr->errorInfo);
	}

	if(success) {
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		_userCommand.state = UserCommand::CommandState::FAILED;
	}

	for(auto it = _cmdResponseReceiverArray.begin(); it != _cmdResponseReceiverArray.end(); it++)
	{
		auto pReceiver = *it;
		pReceiver->OnLocatorQuery(_userCommand.commandId,
				_userCommand.state == UserCommand::CommandState::SUCCEEDED,
				_userCommand.resultLocatorStatus[replyPtr->index]);
	}
}

void CommandRunner::processFeedbacks()
{
	if(_feedbacks.empty()) {
		return;
	}
	for(;;)
	{
		if(_feedbacks.empty()) {
			break;
		}

		//retrieve feed back from beginning of _feedbacks
		std::string feedback = _feedbacks.front();
		_feedbacks.pop_front();
		pLogger->LogInfo("CommandRunner::processFeedbacks dispose feedback: " + feedback);

		//create a ReplyTranslater object.
		ReplyTranslator translator(feedback);
		auto replyType = translator.Type();

		switch(replyType)
		{
			case ReplyTranslator::ReplyType::DevicesGet:
			{
				auto replyPtr = translator.ToDevicesGet();
				onFeedbackDevicesGet(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::DeviceConnect:
			{
				auto replyPtr = translator.ToDeviceConnect();
				onFeedbackDeviceConnect(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::DeviceQueryPower:
			{
				auto replyPtr = translator.ToDeviceQueryPower();
				onFeedbackDeviceQueryPower(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::DeviceQueryFuse:
			{
				auto replyPtr = translator.ToDeviceQueryFuse();
				onFeedbackDeviceQueryFuse(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcsPowerOn:
			{
				auto replyPtr = translator.ToBdcsPowerOn();
				onFeedbackBdcsPowerOn(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcsPowerOff:
			{
				auto replyPtr = translator.ToBdcsPowerOff();
				onFeedbackBdcsPowerOff(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcsQueryPower:
			{
				auto replyPtr = translator.ToBdcsQueryPower();
				onFeedbackBdcsQueryPower(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcCoast:
			{
				auto replyPtr = translator.ToBdcCoast();
				onFeedbackBdcCoast(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcReverse:
			{
				auto replyPtr = translator.ToBdcReverse();
				onFeedbackBdcReverse(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcForward:
			{
				auto replyPtr = translator.ToBdcForward();
				onFeedbackBdcForward(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcBreak:
			{
				auto replyPtr = translator.ToBdcBreak();
				onFeedbackBdcBreak(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcQuery:
			{
				auto replyPtr = translator.ToBdcQuery();
				onFeedbackBdcQuery(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::SteppersPowerOn:
			{
				auto replyPtr = translator.ToSteppersPowerOn();
				onFeedbackSteppersPowerOn(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::SteppersPowerOff:
			{
				auto replyPtr = translator.ToSteppersPowerOff();
				onFeedbackSteppersPowerOff(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::SteppersQueryPower:
			{
				auto replyPtr = translator.ToSteppersQueryPower();
				onFeedbackSteppersQueryPower(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperQueryResolution:
			{
				auto replyPtr = translator.ToStepperQueryResolution();
				onFeedbackStepperQueryResolution(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperConfigStep:
			{
				auto replyPtr = translator.ToStepperConfigStep();
				onFeedbackStepperConfigStep(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperAccelerationBuffer:
			{
				auto replyPtr = translator.ToStepperAccelerationBuffer();
				onFeedbackStepperAccelerationBuffer(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperAccelerationBufferDecrement:
			{
				auto replyPtr = translator.ToStepperAccelerationBufferDecrement();
				onFeedbackStepperAccelerationBufferDecrement(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperDecelerationBuffer:
			{
				auto replyPtr = translator.ToStepperDecelerationBuffer();
				onFeedbackStepperDecelerationBuffer(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperDecelerationBufferIncrement:
			{
				auto replyPtr = translator.ToStepperDecelerationBufferIncrement();
				onFeedbackStepperDecelerationBufferIncrement(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperEnable:
			{
				auto replyPtr = translator.ToStepperEnable();
				onFeedbackStepperEnable(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperForward:
			{
				auto replyPtr = translator.ToStepperForward();
				onFeedbackStepperForward(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperSteps:
			{
				auto replyPtr = translator.ToStepperSteps();
				onFeedbackStepperSteps(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperRun:
			{
				auto replyPtr = translator.ToStepperRun();
				onFeedbackStepperRun(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperConfigHome:
			{
				auto replyPtr = translator.ToStepperConfigHome();
				onFeedbackStepperConfigHome(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperMove:
			{
				auto replyPtr = translator.ToStepperMove();
				onFeedbackStepperMove(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperQuery:
			{
				auto replyPtr = translator.ToStepperQuery();
				onFeedbackStepperQuery(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::StepperSetState:
			{
				auto replyPtr = translator.ToStepperSetState();
				onFeedbackStepperSetState(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::LocatorQuery:
			{
				auto replyPtr = translator.ToLocatorQuery();
				onFeedbackLocatorQuery(replyPtr);
			}
			break;

			default:
			{
				pLogger->LogError("CommandRunner::processFeedbacks unknown feedback: " + feedback);
			}
			break;
		}
	}
}

void CommandRunner::AddResponseReceiver(IResponseReceiver * p)
{
	_cmdResponseReceiverArray.push_back(p);
}

unsigned long CommandRunner::sendCmdToDevice(std::shared_ptr<DeviceCommand>& cmdPtr)
{
	unsigned long cmdId = InvalidCommandId;

	if(cmdPtr != nullptr)
	{
		//set common userCommand attribute
		_userCommand.jsonCommandString = cmdPtr->ToJsonCommandString();
		_userCommand.commandKey = cmdPtr->CommandKey();
		_userCommand.commandId = cmdPtr->CommandId();
		_userCommand.expectedResult = cmdPtr->GetFinalState();

		//send out command
		_pDeviceAccessor->SendCommand(_userCommand.jsonCommandString);
		_userCommand.state = UserCommand::CommandState::COMMAND_SENT;

		cmdId = _userCommand.commandId;
	}

	return cmdId;
}

ICommandReception::CommandId CommandRunner::DevicesGet()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	cmdPtr = CommandFactory::DevicesGet();
	if(cmdPtr == nullptr) {
		pLogger->LogError("CommandRunner::DevicesGet empty ptr returned from CommandFactory::DevicesGet");
	}
	else {
		_userCommand.resultDevices.clear();
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::DeviceConnect(unsigned int index)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	auto& devices = _userCommand.resultDevices;
	if(devices.empty()) {
		pLogger->LogError("CommandRunner::DeviceConnect no device to connect");
	}
	else
	{
		int deviceNumber = index;

		if(deviceNumber >= devices.size()) {
			pLogger->LogError("CommandRunner::DeviceConnect wrong device number in command: " + std::to_string(deviceNumber));
		}
		else
		{
			std::string deviceName = devices[deviceNumber];
			cmdPtr = CommandFactory::DeviceConnect(deviceName);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::DeviceConnect empty ptr returned from CommandFactory::DeviceConnect");
			}
			else {
				_userCommand.resultConnectedDeviceName.clear();
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::DeviceQueryPower()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::DeviceQueryPower hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::DeviceQueryPower();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::DeviceQueryPower empty ptr returned from CommandFactory::DeviceQueryPower");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::DeviceQueryFuse()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::DeviceQueryFuse hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::DeviceQueryFuse();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::DeviceQueryFuse empty ptr returned from CommandFactory::DeviceQueryFuse");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::OptPowerOn()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::OptPowerOff()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::OptQueryPower()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::DcmPowerOn()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::DcmPowerOff()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::DcmQueryPower()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcsPowerOn()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcsPowerOn hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::BdcsPowerOn();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::BdcsPowerOn empty ptr returned from CommandFactory::BdcsPowerOn");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcsPowerOff()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcsPowerOff hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::BdcsPowerOff();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::BdcsPowerOff empty ptr returned from CommandFactory::BdcsPowerOff");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcsQueryPower()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcsQueryPower hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::BdcsQueryPower();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::BdcsQueryPower empty ptr returned from CommandFactory::BdcsQueryPower");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcCoast(unsigned int index)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcCoast hasn't connected to any device");
	}
	else {
		if(index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::BdcCoast invalid BDC index: " + std::to_string(index));
		}
		else
		{
			//set the intial state to BREAK.
			cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::COAST, 0, 0 ,0);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::BdcCoast empty ptr returned from CommandFactory::BdcOperation");
			}
			else {
				_userCommand.bdcIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcReverse(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcReverse hasn't connected to any device");
	}
	else {
		if(index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::BdcReverse invalid BDC index: " + std::to_string(index));
		}
		else
		{
			//set the intial state to BREAK.
			cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::REVERSE,
												lowClks, highClks, cycles);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::BdcReverse empty ptr returned from CommandFactory::BdcOperation");
			}
			else {
				_userCommand.bdcIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcForward(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcForward hasn't connected to any device");
	}
	else {
		if(index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::BdcForward invalid BDC index: " + std::to_string(index));
		}
		else
		{
			//set the intial state to BREAK.
			cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::FORWARD,
													lowClks, highClks, cycles);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::BdcForward empty ptr returned from CommandFactory::BdcOperation");
			}
			else {
				_userCommand.bdcIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcBreak(unsigned int index)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcBreak hasn't connected to any device");
	}
	else {
		if(index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::BdcBreak invalid BDC index: " + std::to_string(index));
		}
		else
		{
			//set the intial state to BREAK.
			cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::BREAK, 0, 0, 0);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::BdcBreak empty ptr returned from CommandFactory::BdcOperation");
			}
			else {
				_userCommand.bdcIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::BdcQuery(unsigned int index)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::BdcQuery hasn't connected to any device");
	}
	else {
		if(index >= BDC_AMOUNT) {
			pLogger->LogError("CommandRunner::BdcQuery invalid BDC index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::BdcQuery(index);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::BdcQuery empty ptr returned from CommandFactory::BdcQuery");
			}
			else {
				_userCommand.bdcIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::SteppersPowerOn()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::SteppersPowerOn hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::SteppersPowerOn();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::SteppersPowerOn empty ptr returned from CommandFactory::SteppersPowerOn");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::SteppersPowerOff()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::SteppersPowerOff hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::SteppersPowerOff();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::SteppersPowerOff empty ptr returned from CommandFactory::SteppersPowerOff");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::SteppersQueryPower()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::SteppersQueryPower hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::SteppersQueryPower();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::SteppersQueryPower empty ptr returned from CommandFactory::SteppersQueryPower");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperQueryResolution()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperQueryResolution hasn't connected to any device");
	}
	else {
		cmdPtr = CommandFactory::StepperQueryResolution();
		if(cmdPtr == nullptr) {
			pLogger->LogError("CommandRunner::StepperQueryResolution empty ptr returned from CommandFactory::StepperQueryResolution");
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperConfigStep(unsigned int index, unsigned short lowClks, unsigned short highClks)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperConfigStep hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperConfigStep invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperConfigStep(index, lowClks, highClks);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperConfigStep empty ptr returned from CommandFactory::StepperConfigStep");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.lowClks = lowClks;
				_userCommand.highClks = highClks;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperAccelerationBuffer(unsigned int index, unsigned short value)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperAccelerationBuffer hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperAccelerationBuffer invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperAccelerationBuffer(index, value);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperAccelerationBuffer empty ptr returned from CommandFactory::StepperAccelerationBuffer");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.accelerationBuffer = value;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperAccelerationBufferDecrement(unsigned int index, unsigned short value)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperAccelerationBufferDecrement hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperAccelerationBufferDecrement invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperAccelerationBufferDecrement(index, value);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperAccelerationBufferDecrement empty ptr returned from CommandFactory::StepperAccelerationBufferDecrement");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.accelerationBufferDecrement = value;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperDecelerationBuffer(unsigned int index, unsigned short value)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperDecelerationBuffer hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperDecelerationBuffer invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperDecelerationBuffer(index, value);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperDecelerationBuffer empty ptr returned from CommandFactory::StepperDecelerationBuffer");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.decelerationBuffer = value;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperDecelerationBufferIncrement(unsigned int index, unsigned short value)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperDecelerationBufferIncrement hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperDecelerationBufferIncrement invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperDecelerationBufferIncrement(index, value);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperDecelerationBufferIncrement empty ptr returned from CommandFactory::StepperDecelerationBufferIncrement");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.decelerationBufferIncrement = value;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperEnable(unsigned int index, bool bEnable)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperEnable hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperEnable invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperEnable(index, bEnable);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperEnable empty ptr returned from CommandFactory::StepperEnable");
			}
			else {
				_userCommand.stepperIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperForward(unsigned int index, bool forward)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperForward hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperForward invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperForward(index, forward);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperForward empty ptr returned from CommandFactory::StepperForward");
			}
			else {
				_userCommand.stepperIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperSteps(unsigned int index, unsigned short value)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperSteps hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperSteps invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperSteps(index, value);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperSteps empty ptr returned from CommandFactory::StepperSteps");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.steps = value;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperRun(unsigned int index, unsigned short initialPos, unsigned short finalPos)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperRun hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperRun invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperRun(index, initialPos, finalPos);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperRun empty ptr returned from CommandFactory::StepperRun");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.initialPosition = initialPos;
				_userCommand.finalPosition = finalPos;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperConfigHome(unsigned int index, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperConfigHome hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperConfigHome invalid stepper index: " + std::to_string(index));
		}
		else if((lineNumberStart < LOCATOR_LINE_NUMBER_MIN) || (lineNumberStart > LOCATOR_LINE_NUMBER_MAX)) {
			pLogger->LogError("CommandRunner::StepperConfigHome lineNumberStart is out of range: " + std::to_string(lineNumberStart));
		}
		else if((lineNumberTerminal < LOCATOR_LINE_NUMBER_MIN) || (lineNumberTerminal > LOCATOR_LINE_NUMBER_MAX)) {
			pLogger->LogError("CommandRunner::StepperConfigHome lineNumberStart is out of range: " + std::to_string(lineNumberTerminal));
		}
		else if(lineNumberTerminal == lineNumberStart) {
			pLogger->LogError("CommandRunner::StepperConfigHome lineNumberStart is same as lineNumberTerminal");
		}
		else
		{
			cmdPtr = CommandFactory::StepperConfigHome(index, locatorIndex, lineNumberStart, lineNumberTerminal);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperConfigHome empty ptr returned from CommandFactory::StepperConfigHome");
			}
			else {
				_userCommand.stepperIndex = index;
				_userCommand.locatorIndex = locatorIndex;
				_userCommand.locatorLineNumberStart = lineNumberStart;
				_userCommand.locatorLineNumberTerminal = lineNumberTerminal;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperMove(unsigned int index, unsigned short steps)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperQuery(unsigned int index)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperQuery hasn't connected to any device");
	}
	else {
		if(index >= STEPPER_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperQuery invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperQuery(index);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperQuery empty ptr returned from CommandFactory::StepperQuery");
			}
			else {
				_userCommand.stepperIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::StepperSetState(unsigned int index, StepperState state)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::StepperSetState hasn't connected to any device");
	}
	else {
		if(index >= LOCATOR_AMOUNT) {
			pLogger->LogError("CommandRunner::StepperSetState invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::StepperSetState(index, (unsigned int)state);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::StepperSetState empty ptr returned from CommandFactory::StepperSetState");
			}
			else {
				_userCommand.stepperIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::LocatorQuery(unsigned int index)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	if(_userCommand.resultConnectedDeviceName.empty()) {
		pLogger->LogError("CommandRunner::LocatorQuery hasn't connected to any device");
	}
	else {
		if(index >= LOCATOR_AMOUNT) {
			pLogger->LogError("CommandRunner::LocatorQuery invalid stepper index: " + std::to_string(index));
		}
		else
		{
			cmdPtr = CommandFactory::LocatorQuery(index);
			if(cmdPtr == nullptr) {
				pLogger->LogError("CommandRunner::LocatorQuery empty ptr returned from CommandFactory::LocatorQuery");
			}
			else {
				_userCommand.locatorIndex = index;
			}
		}
	}

	ICommandReception::CommandId cmdId ;
	cmdId = sendCmdToDevice(cmdPtr);
	return cmdId;
}

ICommandReception::CommandId CommandRunner::SaveMovementConfig()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	saveMovementConfig();
	return 0;
}

