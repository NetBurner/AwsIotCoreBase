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

#include <stdio.h>
#include <nbrtos.h>

#include "aws-src/include/timer_interface.h"

const int ms_per_tick = 1000 / TICKS_PER_SECOND;

/**
 * @brief The following functions define a timer interface that is used by the AWS IoT Core MQTT client.
 */

/**
 * @brief How much time is left on the provided timer.
 */
uint32_t left_ms(Timer *timer)
{
    if (timer == nullptr)
    {
        iprintf("Timer left_ms: passed a null value.\n");
        return 0;
    }

    tick_t currentTick = TimeTick;
    int32_t seconds = (currentTick / TICKS_PER_SECOND);
    int32_t useconds = (currentTick % TICKS_PER_SECOND) * ms_per_tick;

    if (seconds > timer->end_time.tv_sec ||
            (seconds == timer->end_time.tv_sec && useconds >= timer->end_time.tv_usec))
    {
        return 0;
    } else {
        return (timer->end_time.tv_sec * 1000 + timer->end_time.tv_usec) - (seconds * 1000 + useconds);
    }
}

/**
 * @brief Checks to see if a timer has expired.
 */
bool has_timer_expired(Timer *timer) {
    if (timer == nullptr)
    {
        iprintf("Timer has_timer_expired: passed a null value.\n");
        return 0;
    }

    if (left_ms(timer) > 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * @brief Create a timer that will expire in the provided number of milliseconds.
 */
void countdown_ms(Timer *timer, uint32_t timeout) {
    if (timer == nullptr)
    {
        iprintf("Timer countdown_ms: Invalid value: timer is null.\n");
        return;
    }

    tick_t currentTick = TimeTick;
    timer->end_time.tv_sec = (currentTick / TICKS_PER_SECOND) + (timeout / 1000);
    timer->end_time.tv_usec = (currentTick % TICKS_PER_SECOND) * ms_per_tick + (timeout % 1000);
    if (timer->end_time.tv_usec >= 1000) {
        timer->end_time.tv_sec += 1;
        timer->end_time.tv_usec -= 1000;
    }
}

/**
 * @brief Create a timer that will expire in the provided number of seconds.
 */
void countdown_sec(Timer *timer, uint32_t timeout)
{
    if (timer == nullptr)
    {
        iprintf("Timer countdown_sec: Invalid value: timer is null.\n");
        return;
    }

    countdown_ms(timer, timeout * 1000);
}

/**
 * @brief Performs any initialization required on a giver Timer object.
 */
void init_timer(Timer *timer)
{
    if (timer == nullptr)
    {
        iprintf("Timer init_timer: passed a null value.\n");
        return;
    }

    timer->end_time.tv_sec = 0;
    timer->end_time.tv_usec = 0;
}
