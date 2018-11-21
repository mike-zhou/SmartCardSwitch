/*
 * IUserCommandRunner.h
 *
 *  Created on: Nov 21, 2018
 *      Author: mikez
 */

#ifndef IUSERCOMMANDRUNNER_H_
#define IUSERCOMMANDRUNNER_H_

#include <string>

/**
 * ==== Examples of user commands ====
 *
 * {
 * 	"userCommand":"reset device"
 * 	"commandId":"uniqueCommandId"
 * }
 *
 * {
 *	"userCommand":"power on opt",
 *	"commandId":"unqueCommandId"
 * }
 *
 * {
 * 	"userCommand":"power off opt",
 * 	"commandId":"uniqueCommandId"
 * }
 *
 * {
 * 	"userCommand":"insert smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"remove smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"swipe smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"tap smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"show bar code",
 * 	"commandId":"uniqueCommandId",
 * 	"barCodeNumber":0
 * }
 *
 * {
 * 	"userCommand":"press PED key",
 * 	"keys":[
 * 		{"index":0, "keyNumber":9},
 * 		{"index":1, "keyNumber":7}
 * 	],
 * 	"downPeriod":1000,
 * 	"commandId":"uniqueCommandId",
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"press soft key",
 * 	"keys":[
 * 		{"index":0, "keyNumber":6},
 * 		{"index":1, "keyNumber":3}
 * 	],
 * 	"downPeriod":1000,
 * 	"commandId":"uniqueCommandId",
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"press assist key",
 * 	"commandId":"uniqueCommandId",
 * 	"keys":[
 * 		{"index":0, "keyNumber":2},
 * 		{"index":1, "keyNumber":9}
 * 	],
 * 	"downPeriod":1000,
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"touch screen",
 * 	"commandId":"uniqueCommandId",
 * 	"areas":[
 * 		{"index":0, "keyNumber":1},
 * 		{"index":1, "keyNumber":5}
 * 	],
 * 	"downPeriod":1000,
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"cancel command",
 * 	"commandId":"uniqueCommandId"
 * }
 *
 * ======== Compound command ====
 * [
 * 	{
 * 		"index"=0,
 * 		"command"={
 * 			"userCommand":"insert smart card",
 * 			"commandId":"uniqueCommandId",
 * 			"smartCardNumber":0
 * 		}
 * 	},
 * 	{
 * 		"index"=1,
 * 		"command"={
 * 			"userCommand":"press PED key",
 * 			"keys":[
 * 				{"index":0, "keyNumber":9},
 * 				{"index":1, "keyNumber":7}
 * 			],
 * 			"downPeriod":1000,
 * 			"commandId":"uniqueCommandId",
 * 			"upPeriod":1000
 * 		}
 * 	},
 * 	{
 * 		"index"=2,
 * 		"command"={
 * 			"userCommand":"remove smart card",
 * 			"commandId":"uniqueCommandId",
 * 			"smartCardNumber":0
 * 		}
 * 	}
 * ]
 *
 * ==== Example of command reply ====
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"ongoing"
 * }
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"failed",
 * 	"errorInfo":"reason to failure"
 * }
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"succeeded"
 * }
 *
 */


class IUserCommandRunnerObserver
{
public:
	virtual ~IUserCommandRunnerObserver() {}

	virtual void OnCommandStatus(const std::string& jsonStatus) = 0;
};

class IUserCommandRunner
{
public:
	virtual ~IUserCommandRunner() { }

	// error is empty if JSON command is accepted.
	virtual void Runcommand(const std::string& jsonCmd, std::string& error) = 0;
};




#endif /* IUSERCOMMANDRUNNER_H_ */
