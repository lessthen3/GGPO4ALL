/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#define BITVECTOR_NIBBLE_SIZE 8

void BitVector_SetBit(uint8_t* vector, int* offset);
void BitVector_ClearBit(uint8_t* vector, int* offset);
void BitVector_WriteNibblet(uint8_t* vector, int nibble, int* offset);
int BitVector_ReadBit(uint8_t* vector, int* offset);
int BitVector_ReadNibblet(uint8_t* vector, int* offset);

