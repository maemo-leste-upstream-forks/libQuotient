/******************************************************************************
 * Copyright (C) 2017 Marius Gripsgard <marius@ubports.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "callcandidatesevent.h"

/*
m.call.candidates
{
    "age": 242352,
    "content": {
        "call_id": "12345",
        "candidates": [
            {
                "candidate": "candidate:863018703 1 udp 2122260223 10.9.64.156
43670 typ host generation 0", "sdpMLineIndex": 0, "sdpMid": "audio"
            }
        ],
        "version": 0
    },
    "event_id": "$WLGTSEFSEF:localhost",
    "origin_server_ts": 1431961217939,
    "room_id": "!Cuyf34gef24t:localhost",
    "sender": "@example:localhost",
    "type": "m.call.candidates"
}
*/
