/* 
 * h264bitstream - a library for reading and writing H.264 video
 * Copyright (C) 2005-2007 Auroras Entertainment, LLC
 * Copyright (C) 2008-2011 Avail-TVN
 * 
 * Written by Alex Izvorski <aizvorski@gmail.com> and Alex Giladi <alex.giladi@gmail.com>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "bs.h"
#include "h264_stream.h"
#include "h264_sei.h"

#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset

sei_t* sei_new()
{
    sei_t* s = (sei_t*)malloc(sizeof(sei_t));
    memset(s, 0, sizeof(sei_t));
    s->payload = NULL;
    return s;
}

void sei_free(sei_t* s)
{
    if ( s->payload != NULL ) free(s->payload);
    free(s);
}

void read_sei_end_bits(h264_stream_t* h, bs_t* b )
{
    // if the message doesn't end at a byte border
    if ( !bs_byte_aligned( b ) )
    {
        if ( !bs_read_u1( b ) ) fprintf(stderr, "WARNING: bit_equal_to_one is 0!!!!\n");
        while ( ! bs_byte_aligned( b ) )
        {
            if ( bs_read_u1( b ) ) fprintf(stderr, "WARNING: bit_equal_to_zero is 1!!!!\n");
        }
    }

    read_rbsp_trailing_bits(h, b);
}

// D.1 SEI payload syntax
void read_sei_payload(h264_stream_t* h, bs_t* b, int payloadType, int payloadSize)
{
    sei_t* s = h->sei;

    s->payload = (uint8_t*)malloc(payloadSize);

    int i;

    for ( i = 0; i < payloadSize; i++ )
        s->payload[i] = bs_read_u(b, 8);
        
    //read_sei_end_bits(h, b);
}

// D.1 SEI payload syntax
void write_sei_payload(h264_stream_t* h, bs_t* b, int payloadType, int payloadSize)
{
    sei_t* s = h->sei;

    int i;
    for ( i = 0; i < s->payloadSize; i++ )
        bs_write_u(b, 8, s->payload[i]);
}



