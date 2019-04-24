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

#include "tinysynth.h"
#include "chips/opn_chip_base.h"
#include <cstdio>
#include <cmath>

static const unsigned g_outputRate = 53267;

struct OPN_Operator
{
    //! Raw register data
    uint8_t     data[7];
};

struct OPN_PatchSetup
{
    //! Operators prepared for sending to OPL chip emulator
    OPN_Operator    OPS[4];
    uint8_t         fbalg;
    uint8_t         lfosens;
    //! Fine tuning
    int8_t          finetune;
    //! Single note (for percussion instruments)
    uint8_t         tone;
};

void TinySynth::resetChip()
{
    m_chip->setRate(g_outputRate, 7670454);

    m_chip->writeReg(0, 0x22, 0x00);   //LFO off
    m_chip->writeReg(0, 0x27, 0x0 );   //Channel 3 mode normal

    //Shut up all channels
    m_chip->writeReg(0, 0x28, 0x00 );   //Note Off 0 channel
    m_chip->writeReg(0, 0x28, 0x01 );   //Note Off 1 channel
    m_chip->writeReg(0, 0x28, 0x02 );   //Note Off 2 channel
    m_chip->writeReg(0, 0x28, 0x04 );   //Note Off 3 channel
    m_chip->writeReg(0, 0x28, 0x05 );   //Note Off 4 channel
    m_chip->writeReg(0, 0x28, 0x06 );   //Note Off 5 channel

    //Disable DAC
    m_chip->writeReg(0, 0x2B, 0x0 );   //DAC off
}

void TinySynth::setInstrument(const FmBank::Instrument &in)
{
    OPN_PatchSetup patch;

    m_notenum = in.percNoteNum >= 128 ? (in.percNoteNum - 128) : in.percNoteNum;
    if(m_notenum == 0)
        m_notenum = 25;
    m_notesNum = 1;
    m_fineTune = 0;
    m_noteOffsets[0] = in.note_offset1;
    //m_noteOffsets[1] = in.note_offset2;

    for(int op = 0; op < 4; op++)
    {
        patch.OPS[op].data[0] = in.getRegDUMUL(op);
        patch.OPS[op].data[1] = in.getRegLevel(op);
        patch.OPS[op].data[2] = in.getRegRSAt(op);
        patch.OPS[op].data[3] = in.getRegAMD1(op);
        patch.OPS[op].data[4] = in.getRegD2(op);
        patch.OPS[op].data[5] = in.getRegSysRel(op);
        patch.OPS[op].data[6] = in.getRegSsgEg(op);
    }
    patch.fbalg    = in.getRegFbAlg();
    patch.lfosens  = in.getRegLfoSens();
    patch.finetune = static_cast<int8_t>(in.note_offset1);
    patch.tone     = 0;

    m_c = 0;
    m_port = (m_c <= 2) ? 0 : 1;
    m_cc   = m_c % 3;

    for(uint8_t op = 0; op < 4; op++)
    {
        m_chip->writeReg(m_port, 0x30 + (op * 4) + m_cc, patch.OPS[op].data[0]);
        m_chip->writeReg(m_port, 0x40 + (op * 4) + m_cc, patch.OPS[op].data[1]);
        m_chip->writeReg(m_port, 0x50 + (op * 4) + m_cc, patch.OPS[op].data[2]);
        m_chip->writeReg(m_port, 0x60 + (op * 4) + m_cc, patch.OPS[op].data[3]);
        m_chip->writeReg(m_port, 0x70 + (op * 4) + m_cc, patch.OPS[op].data[4]);
        m_chip->writeReg(m_port, 0x80 + (op * 4) + m_cc, patch.OPS[op].data[5]);
        m_chip->writeReg(m_port, 0x90 + (op * 4) + m_cc, patch.OPS[op].data[6]);
    }
    m_chip->writeReg(m_port, 0xB0 + m_cc, patch.fbalg);
    m_chip->writeReg(m_port, 0xB4 + m_cc, 0xC0 | patch.lfosens);
}

void TinySynth::noteOn()
{
    #pragma message("XXX Tuning for OPNA clock")

    double hertz = 440.0 * std::exp2((m_notenum - 69) / 12);

    uint32_t octave = 0, ftone = 0;

    if(hertz < 0) // Avoid infinite loop
        return;

    double coef = 321.88557;
    //double coef = 309.12412;
    hertz *= coef;

    //Basic range until max of octaves reaching
    while((hertz >= 1023.75) && (octave < 0x3800))
    {
        hertz /= 2.0;    // Calculate octave
        octave += 0x800;
    }
    ftone = octave + static_cast<uint32_t>(hertz + 0.5);

    m_chip->writeReg(m_port, 0xA4 + m_cc, (ftone >> 8) & 0xFF);//Set frequency and octave
    m_chip->writeReg(m_port, 0xA0 + m_cc,  ftone & 0xFF);
    m_chip->writeReg(0, 0x28, 0xF0 + uint8_t((m_c <= 2) ? m_c : m_c + 1));
}

void TinySynth::noteOff()
{
    // Keyoff the note
    uint8_t cc = static_cast<uint8_t>(m_c % 6);
    m_chip->writeReg(0, 0x28, (m_c <= 2) ? cc : cc + 1);
}

void TinySynth::generate(int16_t *output, size_t frames)
{
    m_chip->generate(output, frames);
}
