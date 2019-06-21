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

/**
 * This example shows how to use a NetBurner device with the AWS IoT Core service
 * via MQTT by subscribing to two different topics, and sending messages on one,
 * and receiving them on the other. Please check out the article at www.netburner.com
 * for more details on how to set this up and test it.
 *
 * Please note that this example was written for NNDK 3.0. It should work with 2.8.x,
 * but will require some modifications to do so.
 */

#include <predef.h>

#include <dns.h>
#include <http.h>
#include <init.h>
#include <nbtime.h>
#include <webclient/http_funcs.h>
#include <tcp.h>

#include "post-record-data.h"

extern "C"
{
    void UserMain(void *pd);
}

const char *AppName = "AWS IoT Core Base";

/**
 * @brief Entry point of our program.
 *
 * Here we set up our stack, start the web server, set the system time, initialize
 * the AWS SDK, and finally, enter the loop that will post a message to a specified topic
 * every 5 seconds.
 */
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5); // Wait up to 5 seconds for active network activity
    StartHttp();

    SetHttpDiag(true);

    IPADDR ntpServer;
    bool foundNtp = true;
    int dnsSuccess = 0;

    // Setup NTP time
    while (!foundNtp)
    {
        dnsSuccess = GetHostByName("pool.ntp.org", &ntpServer, INADDR_ANY, TICKS_PER_SECOND * 10);
        iprintf("DnsSuccess: %d\r\n", dnsSuccess);
        iprintf("IP Address: %I\r\n", ntpServer);
        if(dnsSuccess != 0 || !SetNTPTime(ntpServer))
        {
            iprintf("Failed to set time.\n");
            OSTimeDly(TICKS_PER_SECOND * 5);
        }
        else
        {
            foundNtp = true;
        }
    }

    // Initialize the AWS SDK
    if (!InitializeAWSSDK())
    {
        iprintf("Exception initializing AWS SDK.");
        while (1)
        {
            OSTimeDly(TICKS_PER_SECOND);
        }
    }

    iprintf("Application started\n");

    while (1)
    {
        // Post our record data
        PostRecordData();
        OSTimeDly(TICKS_PER_SECOND * 5);
    }
}
