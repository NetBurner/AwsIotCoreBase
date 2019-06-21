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

#include <predef.h> ////

#include <nbrtos.h>
#include <string.h>

#include "aws-src/include/aws_iot_version.h"
#include "aws_iot_config.h"
#include "aws-src/include/aws_iot_mqtt_client.h"
#include "aws-src/include/aws_iot_mqtt_client_interface.h"

#include "record-data.h"

extern const unsigned char privatekey[];
extern const unsigned char certificate[];

const char* gTopicNameSend = "NBTutorial/ToAws";
const char* gTopicNameRec = "NBTutorial/ToNb";

ParsedJsonDataSet gJsonOut;
char recJson[512] = {0};

AWS_IoT_Client client;

/**
 * @brief Called when a MQTT message is received.
 */
void IoTSubscribeCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData)
{
    iprintf("Received Message:\r\n");
    ParsedJsonDataSet jsonIn = ParsedJsonDataSet((char *) params->payload, (int) params->payloadLen);
    jsonIn.PrintObject(true);
    iprintf("\r\n");

    jsonIn.PrintObjectToBuffer( recJson, 256, true);
}

/**
 * @brief Creates the message
 */
void CreateOutMessage(ParsedJsonDataSet &jsonOutObject)
{
    static uint16_t postId = 0;
    time_t seconds = 0;
    char dateString[50] = { 0 };

    time(&seconds);
    struct tm tm_struct;

    localtime_r( &seconds, &tm_struct);
    sniprintf( dateString, 50, "%d/%d/%d %02d:%02d:%02d",
               tm_struct.tm_mon+1,
               tm_struct.tm_mday,
               tm_struct.tm_year+1900,
               tm_struct.tm_hour,
               tm_struct.tm_min,
               tm_struct.tm_sec );

    PostRecord record = { dateString, postId++ };

    SerializeRecordJson(record, jsonOutObject);
}

bool InitializeAWSSDK()
{
    IoT_Error_t rc = FAILURE;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    iprintf("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR,
    VERSION_PATCH, VERSION_TAG);

    // Set required MQTT parameters
    mqttInitParams.enableAutoReconnect = false;
    mqttInitParams.pHostURL = AWS_IOT_MQTT_HOST;
    mqttInitParams.port = AWS_IOT_MQTT_PORT;
    mqttInitParams.pRootCALocation = "";
    mqttInitParams.pDeviceCertLocation = (char *) certificate;
    mqttInitParams.pDevicePrivateKeyLocation = (char *) privatekey;
    mqttInitParams.mqttCommandTimeout_ms = 30000;
    mqttInitParams.tlsHandshakeTimeout_ms = 10000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = nullptr;
    mqttInitParams.disconnectHandlerData = nullptr;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if (SUCCESS != rc)
    {
        iprintf("aws_iot_mqtt_init returned error : %d ", rc);
        return false;
    }

    connectParams.keepAliveIntervalInSec = 600;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.pClientID = AWS_IOT_MQTT_CLIENT_ID;
    connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
    connectParams.isWillMsgPresent = false;

    iprintf("Connecting...\n");
    rc = aws_iot_mqtt_connect(&client, &connectParams);
    if (SUCCESS != rc)
    {
        iprintf("Error(%d) connecting to %s:%d\n", rc, mqttInitParams.pHostURL, mqttInitParams.port);
        return false;
    }

    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if (SUCCESS != rc)
    {
        iprintf("Unable to set Auto Reconnect to true - %d\n", rc);
        return false;
    }

    iprintf("Subscribing to topic: %s, %d\n", gTopicNameRec, strlen(gTopicNameRec));
    rc = aws_iot_mqtt_subscribe(&client, (char *) gTopicNameRec, strlen(gTopicNameRec), QOS1, IoTSubscribeCallbackHandler, nullptr);
    if (SUCCESS != rc)
    {
        iprintf("Error subscribing : %d\n", rc);
        return false;
    }

    return true;
}

void PostRecordData()
{
    char buffer[200] = { 0 };
    CreateOutMessage(gJsonOut);
    gJsonOut.PrintObjectToBuffer(buffer, 199, false);

    IoT_Publish_Message_Params paramsQOS1;
    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *) buffer;
    paramsQOS1.payloadLen = strlen(buffer);
    paramsQOS1.isRetained = 0;

    IoT_Error_t rc = aws_iot_mqtt_publish(&client, gTopicNameSend, strlen(gTopicNameSend), &paramsQOS1);
    if (SUCCESS != rc)
    {
        iprintf("Error subscribing : %d\n", rc);
    }
    else
    {
        iprintf("Publish success:\r\n");
        gJsonOut.PrintObject(true);
        iprintf("\r\n");
    }
}
