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

#ifndef POST_RECORD_DATA_H_
#define POST_RECORD_DATA_H_

/**
 * @brief Called to initialize the AWS SDK.
 *
 * This takes care of initializing a connection to the IoT Core service
 * through MQTT based on the values set in aws_iot_config.h.
 */
bool InitializeAWSSDK();

/**
 * @brief Posts record data to AWS IoT Core service via configured MQTT topic.
 */
void PostRecordData();

#endif /* POST_RECORD_DATA_H_ */
