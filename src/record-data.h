/******************************************************************************
* Copyright 1998-2019 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 16855 W. Bernardo Dr, Ste 260
* San Diego, CA 92127
* www.netburner.com
******************************************************************************/

#ifndef TEMP_DATA_H_
#define TEMP_DATA_H_

#include <json_lexer.h>

/**
 * @brief Defines a struct used to track our postId and time information.
 */
typedef struct PostRecordS
{
	char* postTime;
    int postId = 0;
} PostRecord;

/**
 * @brief Builds our MQTT message using a JSON data set.
 */
void SerializeRecordJson(PostRecord &record, ParsedJsonDataSet &json);

#endif /* TEMP_DATA_H_ */
