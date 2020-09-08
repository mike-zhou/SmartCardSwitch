/*
 * IDataExchange.h
 *
 *  Created on: 8/09/2020
 *      Author: user1
 */

#ifndef IDATAEXCHANGE_H_
#define IDATAEXCHANGE_H_

class IDataExchange
{
public:
	virtual void Send(const unsigned char * pData, const unsigned int amount) = 0;
	virtual void Connect (IDataExchange * pInterface) = 0;
	virtual ~IDataExchange() {}
};

#endif /* IDATAEXCHANGE_H_ */
