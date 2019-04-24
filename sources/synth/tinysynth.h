/*
 * OPN2 Bank Editor by Wohlstand, a free tool for music bank editing
 * Copyright (c) 2017-2019 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "instrument/bank.h"
#include <cstddef>
#include <cstdint>

class OPNChipBase;

struct TinySynth
{
    //! Context of the chip emulator
    OPNChipBase *m_chip;
    //! Count of playing notes
    unsigned m_notesNum;
    //! MIDI note to play
    int m_notenum;
    //! Centy detune
    int8_t  m_fineTune;
    //! Half-tone offset
    int16_t m_noteOffsets[2];

    //! Absolute channel
    uint32_t    m_c;
    //! Port of OPN2 chip
    uint8_t     m_port;
    //! Relative channel
    uint8_t     m_cc;

    void resetChip();
    void setInstrument(const FmBank::Instrument &in);
    void noteOn();
    void noteOff();
    void generate(int16_t *output, size_t frames);
};
