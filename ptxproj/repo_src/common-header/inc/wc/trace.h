//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project common-header.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Tracing functions
///
///  \author   Mahe : WAGO GmbH & Co. KG
///  \author   PEn  : WAGO GmbH & Co. KG
///  \author   Röh  : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WC_TRACE_H_
#define INC_WC_TRACE_H_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/assertion.h>
#include <wc/compiler.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

/// \def WC_TRACE_POINT_MAX_LENGTH
#define WC_TRACE_POINT_MAX_LENGTH                                           1024

/// \def WC_TRACE_ALL_CHANNEL_NUMBER
#define WC_TRACE_ALL_CHANNEL_NUMBER                                           99

/// Defines available channels.
typedef enum wc_trace_channels
{
    /// Marker for an invalid value
    invalid    = 0U,

    /// Channel 1
    channel_1  = 1U,

    /// Channel 2
    channel_2  = (1U<<1),

    /// Channel 3
    channel_3  = (1U<<2),

    /// Channel 4
    channel_4  = (1U<<3),

    /// Channel 5
    channel_5  = (1U<<4),

    /// Channel 6
    channel_6  = (1U<<5),

    /// Channel 7
    channel_7  = (1U<<6),

    /// Channel 8
    channel_8  = (1U<<7),

    /// Channel 9
    channel_9  = (1U<<8),

    /// Channel 10
    channel_10 = (1U<<9),

    /// Channel 11
    channel_11 = (1U<<10),

    /// Channel 12
    channel_12 = (1U<<11),

    /// Channel 13
    channel_13 = (1U<<12),

    /// Channel 14
    channel_14 = (1U<<13),

    /// Channel 15
    channel_15 = (1U<<14),

    /// Channel 16
    channel_16 = (1U<<15),

    /// Channel 17
    channel_17 = (1U<<16),

    /// Channel 18
    channel_18 = (1U<<17),

    /// Channel 19
    channel_19 = (1U<<18),

    /// Channel 20
    channel_20 = (1U<<19),

    /// Channel 21
    channel_21 = (1U<<20),

    /// Channel 22
    channel_22 = (1U<<21),

    /// Channel 23
    channel_23 = (1U<<22),

    /// Channel 24
    channel_24 = (1U<<23),

    /// Channel 25
    channel_25 = (1U<<24),

    /// Channel 26
    channel_26 = (1U<<25),

    /// Channel 27
    channel_27 = (1U<<26),

    /// Channel 28
    channel_28 = (1U<<27),

    /// Channel 29
    channel_29 = (1U<<28),

    /// Channel 30
    channel_30 = (1U<<29),

    /// Channel 31
    channel_31 = (1U<<30),

    /// Channel 32
    channel_32 = (1U<<31),

    /// All
    /// For trace points not associated with a particular channel.
    all        = UINT32_MAX,
} wc_trace_channels;
WC_STATIC_ASSERT(sizeof(wc_trace_channels) >= sizeof(uint32_t));

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    //------------------------------------------------------------------------------
    /// Activates a trace channel.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \param channel
    ///   Trace channel to activate.
    //------------------------------------------------------------------------------
    void wc_trace_activate_channel(wc_trace_channels const channel);

    //------------------------------------------------------------------------------
    /// Deactivates a trace channel.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \param channel
    ///   Trace channel to deactivate.
    //------------------------------------------------------------------------------
    void wc_trace_deactivate_channel(wc_trace_channels const channel);

    //------------------------------------------------------------------------------
    /// Checks if a trace channel is active.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \param channel
    ///   Trace channel to check.
    /// \return 
    ///   True if channel is active, false otherwise.
    //------------------------------------------------------------------------------
    bool wc_trace_is_active_channel(wc_trace_channels const channel);

    //------------------------------------------------------------------------------
    /// Starts recording for a specific trace channel,
    /// if channel was activated previously.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \param channel
    ///   Trace channel to start.
    //------------------------------------------------------------------------------
    void wc_trace_start_channel(wc_trace_channels const channel);

    //------------------------------------------------------------------------------
    /// Stops recording for a specific trace channel.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \param channel
    ///   Trace channel to stop.
    //------------------------------------------------------------------------------
    void wc_trace_stop_channel(wc_trace_channels const channel);

    //------------------------------------------------------------------------------
    /// Checks if a trace channel is started.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \param channel
    ///   Trace channel to check.
    /// \return 
    ///   True if channel is started, false otherwise.
    //------------------------------------------------------------------------------
    bool wc_trace_is_started_channel(wc_trace_channels const channel);

    //------------------------------------------------------------------------------
    /// Setter function for a trace point.
    ///
    /// \note A trace point will only be recorded if corresponding channel(s) is activated and started.
    ///
    /// \note Multiple channels may be combined with binary OR operation.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \param channel
    ///   Trace channel to associate trace point with.
    /// \param trace_point_name
    ///   Trace point name to associate with trace point.
    /// \param name_length
    ///   Length of trace point name. Must not exceed \ref WC_TRACE_POINT_MAX_LENGTH.
    //------------------------------------------------------------------------------
    void wc_trace_set_marker(wc_trace_channels const channel,
                             char const *      const trace_point_name,
                             size_t            const name_length);

    //------------------------------------------------------------------------------
    /// Get channel number as unsigned integer value.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \note The function returns the highest channel number in case of multi-channel input.
    ///       In special case "all channels" the value 99 is returned.
    ///
    /// \param channel
    ///   Channel to get number for.
    ///
    /// \return
    ///   Channel number as unsigned integer value.
    //------------------------------------------------------------------------------
    inline unsigned wc_trace_get_channel_number(wc_trace_channels const channel)
    {
        WC_ASSERT(channel != invalid);
        WC_STATIC_ASSERT(sizeof(uint32_t) >= sizeof(all));
        uint32_t channel_bits = (uint32_t)channel;
        unsigned channel_num  = 1;
        if(channel == all)
        {
            channel_num = WC_TRACE_ALL_CHANNEL_NUMBER;
        }
        else
        {
            while(channel_bits > 1u)
            {
                channel_bits = channel_bits >> 1;
                ++channel_num;
            }
        }
        return channel_num;
    }

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

//------------------------------------------------------------------------------
/// \def WC_TRACE_SET_MARKER
/// Setter function for a trace point.
///
/// \note A trace point will only be recorded if corresponding channel(s) is activated and started.
///
/// \note Multiple channels may be combined with binary OR operation.
///
/// \note The function may be implemented/provided application specific.
///
/// \param channel
///   Trace channel to associate trace point with.
/// \param const_trace_point_name
///   Trace point name to associate with trace point. Must be char-array.
//lint -estring(961, WC_TRACE_SET_MARKER) to disable Rule 19.7 it is necessary to disable all 961 messages,
//                                        function-like macro defined
//lint -estring(960, WC_TRACE_SET_MARKER) to disable Rule 19.4 it is necessary to disable all 960 messages,
//                                        disallowed definition for macro
//------------------------------------------------------------------------------
#define WC_TRACE_SET_MARKER(channel, const_trace_point_name) \
    wc_trace_set_marker(channel, const_trace_point_name, sizeof(const_trace_point_name) - 1)

#endif // INC_WC_TRACE_H_
//---- End of source file ------------------------------------------------------
