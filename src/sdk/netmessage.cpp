/*
 * netmessage.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#include <core/logging.hpp>
#include <sdk/netmessage.hpp>
#include "common.hpp"

bf_write::bf_write()
{
    m_pData             = NULL;
    m_nDataBytes        = 0;
    m_nDataBits         = -1; // set to -1 so we generate overflow on any operation
    m_iCurBit           = 0;
    m_bOverflow         = false;
    m_bAssertOnOverflow = true;
    m_pDebugName        = NULL;
}

unsigned long g_LittleBits[32];

// Precalculated bit masks for WriteUBitLong. Using these tables instead of
// doing the calculations gives a 33% speedup in WriteUBitLong.
unsigned long g_BitWriteMasks[32][33];

// (1 << i) - 1
unsigned long g_ExtraMasks[33];

#include "bitvec.h"

inline int BitForBitnum(int bitnum)
{
    return GetBitForBitnum(bitnum);
}

class CBitWriteMasksInit
{
public:
    CBitWriteMasksInit()
    {
        for (unsigned int startbit = 0; startbit < 32; startbit++)
        {
            for (unsigned int nBitsLeft = 0; nBitsLeft < 33; nBitsLeft++)
            {
                unsigned int endbit                  = startbit + nBitsLeft;
                g_BitWriteMasks[startbit][nBitsLeft] = BitForBitnum(startbit) - 1;
                if (endbit < 32)
                    g_BitWriteMasks[startbit][nBitsLeft] |= ~(BitForBitnum(endbit) - 1);
            }
        }

        for (unsigned int maskBit = 0; maskBit < 32; maskBit++)
            g_ExtraMasks[maskBit] = BitForBitnum(maskBit) - 1;
        g_ExtraMasks[32] = ~0ul;

        for (unsigned int littleBit = 0; littleBit < 32; littleBit++)
            StoreLittleDWord(&g_LittleBits[littleBit], 0, 1u << littleBit);
    }
};
static CBitWriteMasksInit g_BitWriteMasksInit;

bf_write::bf_write(const char *pDebugName, void *pData, int nBytes, int nBits)
{
    m_bAssertOnOverflow = true;
    m_pDebugName        = pDebugName;
    StartWriting(pData, nBytes, 0, nBits);
}

bf_write::bf_write(void *pData, int nBytes, int nBits)
{
    m_bAssertOnOverflow = true;
    m_pDebugName        = NULL;
    StartWriting(pData, nBytes, 0, nBits);
}

bool bf_write::WriteBytes(const void *pBuf, int nBytes)
{
    return WriteBits(pBuf, nBytes << 3);
}

bool bf_write::WriteBits(const void *pInData, int nBits)
{
#if defined(BB_PROFILING)
    VPROF("bf_write::WriteBits");
#endif

    unsigned char *pOut = (unsigned char *) pInData;
    int nBitsLeft       = nBits;

    // Bounds checking..
    if ((m_iCurBit + nBits) > m_nDataBits)
    {
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
        return false;
    }

    // Align output to dword boundary
    while (((unsigned long) pOut & 3) != 0 && nBitsLeft >= 8)
    {

        WriteUBitLong(*pOut, 8, false);
        ++pOut;
        nBitsLeft -= 8;
    }

    if (IsPC() && (nBitsLeft >= 32) && (m_iCurBit & 7) == 0)
    {
        // current bit is byte aligned, do block copy
        int numbytes = nBitsLeft >> 3;
        int numbits  = numbytes << 3;

        Q_memcpy((char *) m_pData + (m_iCurBit >> 3), pOut, numbytes);
        pOut += numbytes;
        nBitsLeft -= numbits;
        m_iCurBit += numbits;
    }

    // X360TBD: Can't write dwords in WriteBits because they'll get swapped
    if (IsPC() && nBitsLeft >= 32)
    {
        unsigned long iBitsRight   = (m_iCurBit & 31);
        unsigned long iBitsLeft    = 32 - iBitsRight;
        unsigned long bitMaskLeft  = g_BitWriteMasks[iBitsRight][32];
        unsigned long bitMaskRight = g_BitWriteMasks[0][iBitsRight];

        unsigned long *pData = &m_pData[m_iCurBit >> 5];

        // Read dwords.
        while (nBitsLeft >= 32)
        {
            unsigned long curData = *(unsigned long *) pOut;
            pOut += sizeof(unsigned long);

            *pData &= bitMaskLeft;
            *pData |= curData << iBitsRight;

            pData++;

            if (iBitsLeft < 32)
            {
                curData >>= iBitsLeft;
                *pData &= bitMaskRight;
                *pData |= curData;
            }

            nBitsLeft -= 32;
            m_iCurBit += 32;
        }
    }

    // write remaining bytes
    while (nBitsLeft >= 8)
    {
        WriteUBitLong(*pOut, 8, false);
        ++pOut;
        nBitsLeft -= 8;
    }

    // write remaining bits
    if (nBitsLeft)
    {
        WriteUBitLong(*pOut, nBitsLeft, false);
    }

    return !IsOverflowed();
}

void bf_write::StartWriting(void *pData, int nBytes, int iStartBit, int nBits)
{
    // Make sure it's dword aligned and padded.
    Assert((nBytes % 4) == 0);
    Assert(((unsigned long) pData & 3) == 0);

    // The writing code will overrun the end of the buffer if it isn't dword
    // aligned, so truncate to force alignment
    nBytes &= ~3;

    m_pData      = (unsigned long *) pData;
    m_nDataBytes = nBytes;

    if (nBits == -1)
    {
        m_nDataBits = nBytes << 3;
    }
    else
    {
        Assert(nBits <= nBytes * 8);
        m_nDataBits = nBits;
    }

    m_iCurBit   = iStartBit;
    m_bOverflow = false;
}

void bf_write::Reset()
{
    m_iCurBit   = 0;
    m_bOverflow = false;
}

bool bf_write::WriteString(const char *pStr)
{
    if (pStr)
    {
        do
        {
            WriteChar(*pStr);
            ++pStr;
        } while (*(pStr - 1) != 0);
    }
    else
    {
        WriteChar(0);
    }

    return !IsOverflowed();
}

bf_read::bf_read()
{
    m_pData             = NULL;
    m_nDataBytes        = 0;
    m_nDataBits         = -1; // set to -1 so we overflow on any operation
    m_iCurBit           = 0;
    m_bOverflow         = false;
    m_bAssertOnOverflow = true;
    m_pDebugName        = NULL;
}

bf_read::bf_read(const void *pData, int nBytes, int nBits)
{
    m_bAssertOnOverflow = true;
    StartReading(pData, nBytes, 0, nBits);
}

bf_read::bf_read(const char *pDebugName, const void *pData, int nBytes, int nBits)
{
    m_bAssertOnOverflow = true;
    m_pDebugName        = pDebugName;
    StartReading(pData, nBytes, 0, nBits);
}

void bf_read::StartReading(const void *pData, int nBytes, int iStartBit, int nBits)
{
    // Make sure we're dword aligned.
    Assert(((unsigned long) pData & 3) == 0);

    m_pData      = (unsigned char *) pData;
    m_nDataBytes = nBytes;

    if (nBits == -1)
    {
        m_nDataBits = m_nDataBytes << 3;
    }
    else
    {
        Assert(nBits <= nBytes * 8);
        m_nDataBits = nBits;
    }

    m_iCurBit   = iStartBit;
    m_bOverflow = false;
}

bool bf_read::ReadString(char *pStr, int maxLen, bool bLine, int *pOutNumChars)
{
    Assert(maxLen != 0);

    bool bTooSmall = false;
    int iChar      = 0;
    while (1)
    {
        char val = ReadChar();
        if (val == 0)
            break;
        else if (bLine && val == '\n')
            break;

        if (iChar < (maxLen - 1))
        {
            pStr[iChar] = val;
            ++iChar;
        }
        else
        {
            bTooSmall = true;
        }
    }

    // Make sure it's null-terminated.
    Assert(iChar < maxLen);
    pStr[iChar] = 0;

    if (pOutNumChars)
        *pOutNumChars = iChar;

    return !IsOverflowed() && !bTooSmall;
}
void bf_write::WriteSBitLong(int data, int numbits)
{
    // Do we have a valid # of bits to encode with?

    // Note: it does this wierdness here so it's bit-compatible with regular
    // integer data in the buffer. (Some old code writes direct integers right
    // into the buffer).
    if (data < 0)
    {
        WriteUBitLong((unsigned int) (0x80000000 + data), numbits - 1, false);
        WriteOneBit(1);
    }
    else
    {
        WriteUBitLong((unsigned int) data, numbits - 1);
        WriteOneBit(0);
    }
}

void bf_write::WriteChar(int val)
{
    WriteSBitLong(val, sizeof(char) << 3);
}

void bf_write::WriteByte(int val)
{
    WriteUBitLong(val, sizeof(unsigned char) << 3);
}

void bf_write::WriteLong(long val)
{
    WriteSBitLong(val, sizeof(long) << 3);
}
void bf_write::WriteWord(int val)
{
    WriteUBitLong(val, sizeof(unsigned short) << 3);
}

bool CLC_RespondCvarValue::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

    buffer.WriteSBitLong(m_iCookie, 32);
    buffer.WriteSBitLong(m_eStatusCode, 4);

    buffer.WriteString(m_szCvarName);
    buffer.WriteString(m_szCvarValue);

    return !buffer.IsOverflowed();
}

bool CLC_RespondCvarValue::ReadFromBuffer(bf_read &buffer)
{
    m_iCookie     = buffer.ReadSBitLong(32);
    m_eStatusCode = (EQueryCvarValueStatus) buffer.ReadSBitLong(4);

    // Read the name.
    buffer.ReadString(m_szCvarNameBuffer, sizeof(m_szCvarNameBuffer));
    m_szCvarName = m_szCvarNameBuffer;

    // Read the value.
    buffer.ReadString(m_szCvarValueBuffer, sizeof(m_szCvarValueBuffer));
    m_szCvarValue = m_szCvarValueBuffer;

    return !buffer.IsOverflowed();
}

const char *CLC_RespondCvarValue::ToString(void) const
{
    return strfmt("%s: status: %d, value: %s, cookie: %d", GetName(), m_eStatusCode, m_szCvarValue, m_iCookie).release();
}

bool NET_NOP::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), 6);
    return !buffer.IsOverflowed();
}

bool NET_NOP::ReadFromBuffer(bf_read &buffer)
{
    return true;
}

const char *NET_NOP::ToString(void) const
{
    return "(null)";
}

bool NET_SignonState::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), 6);
    buffer.WriteByte(m_nSignonState);
    buffer.WriteLong(m_nSpawnCount);

    return !buffer.IsOverflowed();
}

bool NET_SignonState::ReadFromBuffer(bf_read &buffer)
{
    /*m_nSignonState = buffer.ReadByte();
    m_nSpawnCount = buffer.ReadLong();
*/
    return true;
}

const char *NET_SignonState::ToString(void) const
{
    return strfmt("net_SignonState: state %i, count %i", m_nSignonState, m_nSpawnCount).release();
}

#define NUM_NEW_COMMAND_BITS 4
#define MAX_NEW_COMMANDS ((1 << NUM_NEW_COMMAND_BITS) - 1)
#define Bits2Bytes(b) ((b + 7) >> 3)
#define NUM_BACKUP_COMMAND_BITS 3
#define MAX_BACKUP_COMMANDS ((1 << NUM_BACKUP_COMMAND_BITS) - 1)

const char *CLC_VoiceData::ToString(void) const
{
    return strfmt("%s: %i bytes", GetName(), Bits2Bytes(m_nLength)).release();
}

bool CLC_VoiceData::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    m_nLength = m_DataOut.GetNumBitsWritten();
    buffer.WriteWord(m_nLength); // length in bits

    return buffer.WriteBits(m_DataOut.GetBasePointer(), m_nLength);
}

bool CLC_VoiceData::ReadFromBuffer(bf_read &buffer)
{
    m_nLength = buffer.ReadWord(); // length in bits
    m_DataIn  = buffer;

    return buffer.SeekRelative(m_nLength);
}

bool CLC_BaselineAck::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    buffer.WriteLong(m_nBaselineTick);
    buffer.WriteUBitLong(m_nBaselineNr, 1);
    return !buffer.IsOverflowed();
}

bool CLC_BaselineAck::ReadFromBuffer(bf_read &buffer)
{

    m_nBaselineTick = buffer.ReadLong();
    m_nBaselineNr   = buffer.ReadUBitLong(1);
    return !buffer.IsOverflowed();
}

const char *CLC_BaselineAck::ToString(void) const
{
    return strfmt("%s: tick %i", GetName(), m_nBaselineTick).release();
}

bool CLC_ListenEvents::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

    int count = MAX_EVENT_NUMBER / 32;
    for (int i = 0; i < count; ++i)
    {
        buffer.WriteUBitLong(m_EventArray.GetDWord(i), 32);
    }

    return !buffer.IsOverflowed();
}

bool CLC_ListenEvents::ReadFromBuffer(bf_read &buffer)
{
    int count = MAX_EVENT_NUMBER / 32;
    for (int i = 0; i < count; ++i)
    {
        m_EventArray.SetDWord(i, buffer.ReadUBitLong(32));
    }

    return !buffer.IsOverflowed();
}

const char *CLC_ListenEvents::ToString(void) const
{
    int count = 0;

    for (int i = 0; i < MAX_EVENT_NUMBER; i++)
    {
        if (m_EventArray.Get(i))
            count++;
    }

    return strfmt("%s: registered events %i", GetName(), count).release();
}

const char *CLC_Move::ToString(void) const
{
    return "";
}

bool CLC_Move::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    m_nLength = m_DataOut.GetNumBitsWritten();

    buffer.WriteUBitLong(m_nNewCommands, NUM_NEW_COMMAND_BITS);
    buffer.WriteUBitLong(m_nBackupCommands, NUM_BACKUP_COMMAND_BITS);

    buffer.WriteWord(m_nLength);

    return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool CLC_Move::ReadFromBuffer(bf_read &buffer)
{

    m_nNewCommands    = buffer.ReadUBitLong(NUM_NEW_COMMAND_BITS);
    m_nBackupCommands = buffer.ReadUBitLong(NUM_BACKUP_COMMAND_BITS);
    m_nLength         = buffer.ReadWord();
    m_DataIn          = buffer;
    return buffer.SeekRelative(m_nLength);
}

bool NET_SetConVar::WriteToBuffer(bf_write &buffer)
{
    // logging::Info("Writing to buffer 0x%08x!", buf);
    buffer.WriteUBitLong(GetType(), 6);
    // logging::Info("A");
    int numvars = 1; // m_ConVars.Count();
    // logging::Info("B");
    // Note how many we're sending
    buffer.WriteByte(numvars);
    // logging::Info("C");
    // for (int i=0; i< numvars; i++ )
    //{
    // cvar_t * cvar = &m_ConVars[i];
    buffer.WriteString(convar.name);
    buffer.WriteString(convar.value);
    //}
    // logging::Info("D");
    return !buffer.IsOverflowed();
}

bool NET_SetConVar::ReadFromBuffer(bf_read &buffer)
{
    int numvars = buffer.ReadByte();

    // m_ConVars.RemoveAll();

    for (int i = 0; i < numvars; i++)
    {
        cvar_t cvar;
        buffer.ReadString(cvar.name, sizeof(cvar.name));
        buffer.ReadString(cvar.value, sizeof(cvar.value));
        // m_ConVars.AddToTail( cvar );
    }
    return !buffer.IsOverflowed();
}

const char *NET_SetConVar::ToString(void) const
{
    /*snprintf(s_text, sizeof(s_text), "%s: %i cvars, \"%s\"=\"%s\"",
        GetName(), m_ConVars.Count(),
        m_ConVars[0].name, m_ConVars[0].value );
    return s_text;*/
    return "(NULL)";
}

bool NET_StringCmd::WriteToBuffer(bf_write &buffer)
{
    buffer.WriteUBitLong(GetType(), 6);
    return buffer.WriteString(m_szCommand ? m_szCommand : " NET_StringCmd NULL");
}

bool NET_StringCmd::ReadFromBuffer(bf_read &buffer)
{
    m_szCommand = m_szCommandBuffer;
    return buffer.ReadString(m_szCommandBuffer, sizeof(m_szCommandBuffer));
}

const char *NET_StringCmd::ToString(void) const
{
    return "STRINGCMD";
}
