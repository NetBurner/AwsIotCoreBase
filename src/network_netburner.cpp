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

#include <predef.h>

#include <iosys.h>
#include <crypto/ssl.h>
#include <string.h>
#include <tcp.h>

#include "aws-src/include/aws_iot_error.h"
#include "aws-src/include/network_interface.h"

extern tick_t TimeTick;

extern const int ms_per_tick = 1000 / TICKS_PER_SECOND;

/**
 * The following functions are required by the AWS IoT Core SDK, and are used to tie the SDK to the networking layer
 * of the NetBurner device.
 */

/**
 * @brief Initializes the the values required for a TLS connection, and sets the function pointers for the required
 * network functionality.
 */
IoT_Error_t iot_tls_init(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation, char *pDevicePrivateKeyLocation, char *pDestinationUrl, uint16_t DestinationPort, uint32_t timeout_ms, bool ServerVerificationFlag)
{
    // In other reference implementations, this set up all the pointers and Network function pointers.
    pNetwork->tlsConnectParams.DestinationPort = DestinationPort;
    pNetwork->tlsConnectParams.pDestinationURL = pDestinationUrl;
    pNetwork->tlsConnectParams.pDevicePublicKey = pDeviceCertLocation;
    pNetwork->tlsConnectParams.pDevicePrivateKey = pDevicePrivateKeyLocation;
    //pNetwork->tlsConnectParams.pRootCALocation = pRootCALocation;
    pNetwork->tlsConnectParams.timeout_ms = timeout_ms;

    pNetwork->connect = iot_tls_connect;
    pNetwork->read = iot_tls_read;
    pNetwork->write = iot_tls_write;
    pNetwork->disconnect = iot_tls_disconnect;
    pNetwork->isConnected = iot_tls_is_connected;
    pNetwork->destroy = iot_tls_destroy;

    return SUCCESS;
}

/**
 * @brief Starts the TLS connection.
 */
IoT_Error_t iot_tls_connect(Network *pNetwork, TLSConnectParams *TLSParams)
{
    iprintf("Public Key: %s\n", pNetwork->tlsConnectParams.pDevicePublicKey);
    SSL_ClientReadyCert((const unsigned char *)pNetwork->tlsConnectParams.pDevicePublicKey, (const unsigned char *)pNetwork->tlsConnectParams.pDevicePrivateKey);

    char pDestinationURL[500];
    uint16_t port = pNetwork->tlsConnectParams.DestinationPort;
    uint32_t timeout = pNetwork->tlsConnectParams.timeout_ms;
    memset(pDestinationURL, 0, 500);
    snprintf(pDestinationURL, 500, "https://%s",
             pNetwork->tlsConnectParams.pDestinationURL);

    iprintf("Check\n");
    ParsedURI uri = ParsedURI(pDestinationURL);
    IPADDR addr = uri.GetAddr();
    iprintf("Hello: %d\n", port);
    pNetwork->tlsDataParams.fds = SSL_connect(addr, 0, port, timeout / ms_per_tick, uri.GetHost());

    iprintf("FDS: %d\n", pNetwork->tlsDataParams.fds);
    // TODO: Probably not enough detail here.
    switch (pNetwork->tlsDataParams.fds) {
    case TCP_ERR_TIMEOUT:
    case TCP_ERR_NOCON:
    case TCP_ERR_CLOSING:
    case TCP_ERR_NOSUCH_SOCKET:
    case TCP_ERR_NONE_AVAIL:
    case TCP_ERR_CON_RESET:
    case TCP_ERR_CON_ABORT:
        return TCP_CONNECTION_ERROR;
    case SSL_ERROR_FAILED_NEGOTIATION:
    case SSL_ERROR_CERTIFICATE_UNKNOWN:
    case SSL_ERROR_CERTIFICATE_NAME_FAILED:
    case SSL_ERROR_CERTIFICATE_VERIFY_FAILED:
    case SSL_ERROR_HANDSHAKE_INCOMPLETE:
    case SSL_ERROR_UNABLE_TO_LOAD_CIPHERS:
        return SSL_CONNECTION_ERROR;
    default:
        return SUCCESS;
    }
}

/**
 * @brief Writes the data passed in to our secure connection.
 */
IoT_Error_t iot_tls_write(Network *pNetwork, unsigned char *data, size_t size, Timer *pTimer, size_t *pWrittenBytes)
{
    int writtenBytes = write(pNetwork->tlsDataParams.fds, (const char *)data, size);
    if(writtenBytes >= 0)
    {
        *pWrittenBytes = writtenBytes;
        return SUCCESS;
    }
    else
    {
        return NETWORK_SSL_WRITE_ERROR;
    }
}

/**
 * @brief Reads data from our secure connection.
 */
IoT_Error_t iot_tls_read(Network *pNetwork, unsigned char *buffer, size_t size, Timer *pTimer, size_t *pReadBytes)
{
	tick_t current_time = TimeTick;
	tick_t end_tick = ( (pTimer->end_time.tv_sec * 1000) + pTimer->end_time.tv_usec) / ms_per_tick;
	unsigned long timeout_tick = end_tick - current_time;
    int readBytes = ReadWithTimeout(pNetwork->tlsDataParams.fds, (char *)buffer, size, timeout_tick);

    if(readBytes > 0)
    {
        *pReadBytes = readBytes;
        return SUCCESS;
    }
    else if (readBytes == 0)
    {
        // TODO: This is a timeout, but maybe it's not an error? Maybe there's nothing to read?
    	return NETWORK_SSL_NOTHING_TO_READ;
    }
    else
    {
        return NETWORK_SSL_READ_ERROR;
    }
}

/**
 * @brief Closes our secure connection.
 */
IoT_Error_t iot_tls_disconnect(Network *pNetwork)
{
	iprintf("Disconnecting...\n");
    if (close(pNetwork->tlsDataParams.fds) == 0)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

/**
 * @brief Called to clean up any resources, but is unnecessary for NetBurner platforms as the cleanup
 * is taken care of at the network layer.
 */
IoT_Error_t iot_tls_destroy(Network *pNetwork)
{
	iprintf("Destroy called\n");
    return SUCCESS;
}

/**
 * @brief Used to check if there is a current active TLS connection.
 */
IoT_Error_t iot_tls_is_connected(Network *pNetwork)
{
	return IsSSLfd( pNetwork->tlsDataParams.fds ) ? SUCCESS : FAILURE;
}
