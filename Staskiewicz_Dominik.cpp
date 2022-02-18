#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
using namespace std;


fstream inFile("example_new.ts", ios::binary | ios::in);
fstream outFile136("PID136.mp2", ios::binary | ios::out);
fstream outFile174("PID174.264", ios::binary | ios::out);

class TSHeader
{
private:
    uint8_t     _syncByte = 0;		                //8 bits
    uint8_t     _transportErrorIndicator = 0;		//1 bit
    uint8_t     _payloadUnitStartIndicator = 0;	    //1 bit
    uint8_t     _transportPriority = 0;		        //1 bit
    uint16_t    _PID = 0;	                        //13 bits
    uint8_t     _transportScramblingControl = 0;	//2 bits
    uint8_t     _adaptationFieldControl = 0;	    //2 bits
    uint8_t     _continuityCounter = 0;		        //4 bits

public:

    void Parse(char* buffer)
    {
        _syncByte = *buffer;
        buffer++;
        _transportErrorIndicator        = ((*buffer >> 7) & 1);
        _payloadUnitStartIndicator      = ((*buffer >> 6) & 1);
        _transportPriority              = ((*buffer >> 5) & 1);
        for (int i = 12; i >= 8; i--)
        {
            _PID += ((*buffer >> (i - 8)) & 1) * static_cast<int>(pow(2, i));
        }

        buffer++;

        _PID += static_cast<uint8_t>(*buffer);
        
        buffer++;
        for (int i = 7; i >= 6; i--)
            _transportScramblingControl += ((*buffer >> i) & 1) * static_cast<int>(pow(2, i - 6));
        for (int i = 5; i >= 4; i--)
            _adaptationFieldControl += ((*buffer >> i) & 1) * static_cast<int>(pow(2, i - 4));
        for (int i = 3; i >= 0; i--)
            _continuityCounter += ((*buffer >> i) & 1) * static_cast<int>(pow(2, i));
    }

    void Print()
    {
        cout << setfill(' ') <<
            " TS: SB = " << setw(3) << static_cast<int>(_syncByte) <<
            " E = " << setw(1) << static_cast<int>(_transportErrorIndicator) <<
            " S = " << setw(1) << static_cast<int>(_payloadUnitStartIndicator) <<
            " T = " << setw(1) << static_cast<int>(_transportPriority) <<
            " PID = " << setw(4) << static_cast<int>(_PID) <<
            " TSC = " << setw(1) << static_cast<int>(_transportScramblingControl) <<
            " AFC = " << setw(1) << static_cast<int>(_adaptationFieldControl) <<
            " CC = " << setw(2) << static_cast<int>(_continuityCounter);
    }

    void Reset() {
        _syncByte = 0;
        _transportErrorIndicator = 0;
        _payloadUnitStartIndicator = 0;
        _transportPriority = 0;
        _PID = 0;
        _transportScramblingControl = 0;
        _adaptationFieldControl = 0;
        _continuityCounter = 0;
    }

    uint8_t     GetAFC() { return _adaptationFieldControl; }
    uint8_t     GetStartBit() { return _payloadUnitStartIndicator; }
    uint16_t    GetPID() { return _PID; }
    uint8_t     GetCC() { return _continuityCounter; }
};

class TSAdaptationField
{
private:
    uint8_t     _adaptationFieldLength = 0;	                    //8 bits
    uint8_t     _discontinuityIndicator = 0;		            //1 bit
    uint8_t     _randomAccessIndicator = 0;		                //1 bit
    uint8_t     _elementaryStreamPriorityIndicator = 0;	        //1 bit
    uint8_t     _PCRFlag = 0;		                            //1 bit
    uint8_t     _OPCRFlag = 0;		                            //1 bit
    uint8_t     _splicingPointFlag = 0;	                        //1 bit
    uint8_t     _transportPrivateDataFlag = 0;		            //1 bit
    uint8_t     _adaptationFieldExtensionFlag = 0;		        //1 bit

    uint64_t    _programClockReferenceBase = 0;                 //33 bits
    uint16_t    _programClockReferenceExtension = 0;            //9 bits

    uint64_t    _originalProgramClockReferenceBase = 0;         //33 bits
    uint16_t    _originalProgramClockReferenceExtension = 0;    //9 bits

    uint8_t     _spliceCountdown = 0;                           //8 bits

    uint8_t     _adaptaionFieldExtensionLength = 0;             //8 bits
    uint8_t     _ltwFlag = 0;                                   //1 bit
    uint8_t     _piecewiseRateFlag = 0;                         //1 bit
    uint8_t     _seamlessSpliceFlag = 0;                        //1 bit

    uint8_t     _ltwValidFlag = 0;                              //1 bit
    uint16_t    _ltwOffset = 0;                                 //15 bits

    uint32_t    _piecewiseRate = 0;                             //22 bits

    uint8_t     _spliceType = 0;                                //4 bits
    uint32_t    _DTSNextAU = 0;                                 //32 bits

    uint8_t     _stuffingBytesCount = 0;                        //8 bits, cannot be longer than AFL

public:
    void Parse(char* buffer)
    {
        _adaptationFieldLength                  = *buffer;
        _stuffingBytesCount                     = _adaptationFieldLength;

        if (_adaptationFieldLength > 0)
        {
            _stuffingBytesCount--;

            buffer++;

            _discontinuityIndicator             = ((*buffer >> 7) & 1);
            _randomAccessIndicator              = ((*buffer >> 6) & 1);
            _elementaryStreamPriorityIndicator  = ((*buffer >> 5) & 1);
            _PCRFlag                            = ((*buffer >> 4) & 1);
            _OPCRFlag                           = ((*buffer >> 3) & 1);
            _splicingPointFlag                  = ((*buffer >> 2) & 1);
            _transportPrivateDataFlag           = ((*buffer >> 1) & 1);
            _adaptationFieldExtensionFlag       = ((*buffer >> 0) & 1);

            if (_PCRFlag)
            {
                _stuffingBytesCount -= 6;
                buffer++;
                for (int i = 32; i >= 25; i--)
                {
                    _programClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 25)) & 1) * static_cast<int>(pow(2, i));
                }
                buffer++;
                for (int i = 24; i >= 17; i--)
                {
                    _programClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 17)) & 1) * static_cast<int>(pow(2, i));
                }
                buffer++;
                for (int i = 16; i >= 9; i--)
                {
                    _programClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 9)) & 1) * static_cast<int>(pow(2, i));
                }
                buffer++;
                for (int i = 8; i >= 1; i--)
                {
                    _programClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 1)) & 1) * static_cast<int>(pow(2, i)); //55608000
                }
                buffer++;
                
                _programClockReferenceBase += ((*buffer >> 7) & 1);
                _programClockReferenceExtension += ((*buffer >> 0) & 1) * static_cast<int>(pow(2, 8));

                buffer++;

                _programClockReferenceExtension += static_cast<uint8_t>(*buffer);
                
            }
            if (_OPCRFlag)
            {
                _stuffingBytesCount -= 6;
                buffer++;
                for (int i = 32; i >= 25; i--)
                {
                    _originalProgramClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 25)) & 1) * static_cast<int>(pow(2, i));
                }
                buffer++;
                for (int i = 24; i >= 17; i--)
                {
                    _originalProgramClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 17)) & 1) * static_cast<int>(pow(2, i));
                }
                buffer++;
                for (int i = 16; i >= 9; i--)
                {
                    _originalProgramClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 9)) & 1) * static_cast<int>(pow(2, i));
                }
                buffer++;
                for (int i = 8; i >= 1; i--)
                {
                    _originalProgramClockReferenceBase += ((static_cast<__int64>(*buffer) >> (i - 1)) & 1) * static_cast<int>(pow(2, i));
                }
                buffer++;

                _originalProgramClockReferenceBase += ((*buffer >> 7) & 1);
                _originalProgramClockReferenceExtension += ((*buffer >> 0) & 1) * static_cast<int>(pow(2, 8));

                buffer++;

                _originalProgramClockReferenceExtension += static_cast<uint8_t>(*buffer);
            }
            if (_splicingPointFlag)
            {
                _stuffingBytesCount--;
                buffer++;
                _spliceCountdown = *buffer;
            }
            if (_adaptationFieldExtensionFlag)
            {
                _stuffingBytesCount -= 2;
                buffer++;
                _adaptaionFieldExtensionLength = static_cast<uint8_t>(*buffer);

                buffer++;
                _ltwFlag = ((*buffer >> 7) & 1);
                _piecewiseRateFlag = ((*buffer >> 6) & 1);
                _seamlessSpliceFlag = ((*buffer >> 5) & 1);
                //5 bits reserved

                if (_ltwFlag)
                {
                    _stuffingBytesCount -= 2;
                    buffer++;
                    _ltwValidFlag = ((*buffer >> 7) & 1);
                    for (int i = 6; i >= 0; i--)
                    {
                        _ltwOffset += ((*buffer >> i) & 1) * static_cast<int>(pow(2, i));
                    }
                    
                    buffer++;
                    _ltwValidFlag += static_cast<uint8_t>(*buffer);
                }
                if (_piecewiseRateFlag)
                {
                    _stuffingBytesCount -= 3;

                    buffer++;
                    for (int i = 21; i >= 16; i--)
                    {
                        _piecewiseRate += ((*buffer >> (i - 16)) & 1) * static_cast<int>(pow(2, i));
                    }

                    buffer++;
                    for (int i = 15; i >= 8; i--)
                    {
                        _piecewiseRate += ((*buffer >> (i - 8)) & 1) * static_cast<int>(pow(2, i));
                    }

                    buffer++;
                    _piecewiseRate += static_cast<uint8_t>(*buffer);
                }
                if (_seamlessSpliceFlag)
                {
                    _stuffingBytesCount -= 5;
                    
                    buffer++;
                    for (int i = 7; i >= 4; i--)
                    {
                        _spliceType += ((*buffer >> (i - 4)) & 1) * static_cast<int>(pow(2, i));
                    }
                    for (int i = 32; i >= 30; i--)
                    {
                        _DTSNextAU += ((*buffer >> (i - 30)) & 1) * static_cast<int>(pow(2, i));
                    }

                    //1 marker bit

                    buffer++;
                    for (int i = 29; i >= 22; i--)
                    {
                        _DTSNextAU += ((*buffer >> (i - 22)) & 1) * static_cast<int>(pow(2, i));
                    }

                    buffer++;
                    for (int i = 21; i >= 15; i--)
                    {
                        _DTSNextAU += ((*buffer >> (i - 15)) & 1) * static_cast<int>(pow(2, i));
                    }

                    //1 marker bit

                    buffer++;
                    for (int i = 14; i >= 7; i--)
                    {
                        _DTSNextAU += ((*buffer >> (i - 7)) & 1) * static_cast<int>(pow(2, i));
                    }

                    buffer++;
                    for (int i = 6; i >= 0; i--)
                    {
                        _DTSNextAU += ((*buffer >> i) & 1) * static_cast<int>(pow(2, i));
                    }

                }
            }
        }
    }

    void Print()
    {

        cout << setfill(' ') <<
            " AF: AFL = " << setw(2) << static_cast<int>(_adaptationFieldLength) <<
            " DC = " << setw(1) << static_cast<int>(_discontinuityIndicator) <<
            " RA = " << setw(1) << static_cast<int>(_randomAccessIndicator) <<
            " SP = " << setw(1) << static_cast<int>(_elementaryStreamPriorityIndicator) <<
            " PR = " << setw(1) << static_cast<int>(_PCRFlag) <<
            " OR = " << setw(1) << static_cast<int>(_OPCRFlag) <<
            " SP = " << setw(1) << static_cast<int>(_splicingPointFlag) <<
            " TP = " << setw(1) << static_cast<int>(_transportPrivateDataFlag) <<
            " EX = " << setw(1) << static_cast<int>(_adaptationFieldExtensionFlag);

        if (_PCRFlag) 
        {
            cout << " PCR = " << static_cast<int>(_programClockReferenceBase * 300 + _programClockReferenceExtension);
        }
        if (_OPCRFlag)
        {
            cout << " OPCR = " << static_cast<int>(_originalProgramClockReferenceBase);
        }

        cout <<" Stuffing = " << setw(2) << static_cast<int>(_stuffingBytesCount);
        
    }

    void Reset()
    {
        _discontinuityIndicator                 = 0;
        _randomAccessIndicator                  = 0;
        _elementaryStreamPriorityIndicator      = 0;
        _PCRFlag                                = 0;
        _OPCRFlag                               = 0;
        _splicingPointFlag                      = 0;
        _transportPrivateDataFlag               = 0;
        _adaptationFieldExtensionFlag           = 0;

        _programClockReferenceBase              = 0;
        _programClockReferenceExtension         = 0;

        _originalProgramClockReferenceBase      = 0;
        _originalProgramClockReferenceExtension = 0;

        _spliceCountdown                        = 0;

        _adaptaionFieldExtensionLength          = 0;
        _ltwFlag                                = 0;
        _piecewiseRateFlag                      = 0;
        _seamlessSpliceFlag                     = 0;

        _ltwValidFlag                           = 0;
        _ltwOffset                              = 0;

        _piecewiseRate                          = 0;

        _spliceType                             = 0;
        _DTSNextAU                              = 0;

        _stuffingBytesCount                     = 0;
    }

    uint8_t GetAFL() { return _adaptationFieldLength; }

};


class PESPacket
{
private:
    enum eStreamID : uint8_t
    {
        eSID_programStreamMap = 0xBC,
        eSID_paddingStream = 0xBE,
        eSID_privateStream2 = 0xBF,
        eSID_ECM = 0xF0,
        eSID_EMM = 0xF1,
        eSID_programStreamDirectory = 0xFF,
        eSID_DSMCCStream = 0xF2,
        eSID_ITUT_H222_1_Type_E = 0xF8
    };

    //PES Header
    uint32_t    _packetStartCodePrefix = 0;     //24 bits
    uint8_t     _streamID = 0;                  //8 bits
    uint16_t    _PESPacketLength = 0;           //16 bits

    uint8_t     _PESScramblingControl = 0;     //2 bits
    uint8_t     _PESPriority = 0;               //1 bit
    uint8_t     _dataAlignmentIndicator = 0;   //1 bit
    uint8_t     _copyright = 0;                 //1 bit
    uint8_t     _originalOrCopy = 0;            //1 bit
    uint8_t     _PTSDTSFlags = 0;               //2 bits
    uint8_t     _ESCRFlag = 0;                  //1 bit
    uint8_t     _ESRateFlag = 0;                //1 bit
    uint8_t     _DSMTrickModeFlag = 0;          //1 bit
    uint8_t     _additionalCopyInfoFlag = 0;    //1 bit
    uint8_t     _PESCRCFlag = 0;                //1 bit
    uint8_t     _PESExtensionFlag = 0;          //1 bit
    uint8_t     _PESHeaderDataLength = 0;       //8 bits

    uint32_t    _PTS = 0;                       //32 bits
    uint32_t    _DTS = 0;                       //32 bits

    uint32_t    _ESCRBase = 0;                  //32 bits
    uint16_t    _ESCRExtension = 0;             //9 bits

    uint32_t    _ESRate = 0;                    //22 bits

    uint8_t     _PESPrivateDataFlag = 0;                //1 bit
    uint8_t     _packHeaderFieldFlag = 0;               //1 bit
    uint8_t     _programPacketSequenceCounterFlag = 0;  //1 bit
    uint8_t     _PSTDBufferFlag = 0;                    //1 bit
    uint8_t     _PESExtensionFlag2 = 0;                 //1 bit

    uint8_t     _PESExtensionFieldLength = 0;           //7 bits
    uint8_t     _streamIDExtensionFlag = 0;             //1 bit

    uint8_t     _trefExtensionFlag = 0;                 //1 bit


    uint8_t         _headerLength = 0;
    unsigned int    _packetLength = 0;

    uint16_t _prevPID = 0;

public:

    void HeaderParse(char* buffer)
    {
        _headerLength += 6;

        _packetStartCodePrefix += static_cast<uint8_t>(*buffer) * static_cast<int>(pow(2, 16));
        buffer++;
        _packetStartCodePrefix += static_cast<uint8_t>(*buffer) * static_cast<int>(pow(2, 8));
        buffer++;
        _packetStartCodePrefix += static_cast<uint8_t>(*buffer);
        buffer++;


        _streamID = static_cast<uint8_t>(*buffer);
        buffer++;

        _PESPacketLength += static_cast<uint8_t>(*buffer) * static_cast<int>(pow(2, 8));
        buffer++;
        _PESPacketLength += static_cast<uint8_t>(*buffer);

        if (_streamID != eSID_programStreamMap && _streamID != eSID_paddingStream && _streamID != eSID_privateStream2 && _streamID != eSID_ECM &&
            _streamID != eSID_EMM && _streamID != eSID_programStreamDirectory && _streamID != eSID_DSMCCStream && _streamID != eSID_ITUT_H222_1_Type_E)
        {
            _headerLength += 3;
            buffer++;

            _PESScramblingControl = ((*buffer >> 5) & 1) * 2 + ((*buffer >> 4) & 1);
            _PESPriority = ((*buffer >> 3) & 1);
            _dataAlignmentIndicator = ((*buffer >> 2) & 1);
            _copyright = ((*buffer >> 1) & 1);
            _originalOrCopy = ((*buffer >> 0) & 1);

            buffer++;

            _PTSDTSFlags = ((*buffer >> 7) & 1) * 2 + ((*buffer >> 6) & 1);
            _ESCRFlag = ((*buffer >> 5) & 1);
            _ESRateFlag = ((*buffer >> 4) & 1);
            _DSMTrickModeFlag = ((*buffer >> 3) & 1);
            _additionalCopyInfoFlag = ((*buffer >> 2) & 1);
            _PESCRCFlag = ((*buffer >> 1) & 1);
            _PESExtensionFlag = ((*buffer >> 0) & 1);

            buffer++;

            _PESHeaderDataLength = static_cast<uint8_t>(*buffer);

            if (_PTSDTSFlags == 0b10)
            {
                _headerLength += 5;
                buffer++;

                for (int i = 32; i >= 30; i--)
                    _PTS += ((*buffer >> (i - 29)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 29; i >= 22; i--)
                    _PTS += ((*buffer >> (i - 15)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 21; i >= 15; i--)
                    _PTS += ((*buffer >> (i - 14)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 14; i >= 7; i--)
                    _PTS += ((*buffer >> (i - 7)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 6; i >= 0; i--)
                    _PTS += ((*buffer >> (i + 1)) & 1) * static_cast<int>(pow(2, i));


            }
            else if(_PTSDTSFlags == 0b11)
            {
                _headerLength += 10;

                buffer++;

                for (int i = 32; i >= 30; i--)
                    _PTS += ((*buffer >> (i - 29)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 29; i >= 22; i--)
                    _PTS += ((*buffer >> (i - 15)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 21; i >= 15; i--)
                    _PTS += ((*buffer >> (i - 14)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 14; i >= 7; i--)
                    _PTS += ((*buffer >> (i - 7)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 6; i >= 0; i--)
                    _PTS += ((*buffer >> (i + 1)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 32; i >= 30; i--)
                    _DTS += ((*buffer >> (i - 29)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 29; i >= 22; i--)
                    _DTS += ((*buffer >> (i - 15)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 21; i >= 15; i--)
                    _DTS += ((*buffer >> (i - 14)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 14; i >= 7; i--)
                    _DTS += ((*buffer >> (i - 7)) & 1) * static_cast<int>(pow(2, i));

                buffer++;

                for (int i = 6; i >= 0; i--)
                    _DTS += ((*buffer >> (i + 1)) & 1) * static_cast<int>(pow(2, i));

            }
            if (_ESCRFlag)
            {
                _headerLength += 6;
            }
            if (_ESRateFlag)
            {
                _headerLength += 3;
            }
            if (_DSMTrickModeFlag)
            {
                _headerLength += 1;
            }
            if (_additionalCopyInfoFlag)
            {
                _headerLength += 1;
            }
            if (_PESCRCFlag)
            {
                _headerLength += 2;
            }
            if (_PESExtensionFlag)
            {
                _headerLength += 1;
                buffer++;

                _PESPrivateDataFlag                     = ((*buffer >> 7) & 1);
                _packHeaderFieldFlag                    = ((*buffer >> 6) & 1);
                _programPacketSequenceCounterFlag       = ((*buffer >> 5) & 1);
                _PSTDBufferFlag                         = ((*buffer >> 4) & 1);
                _PESExtensionFlag2                      = ((*buffer >> 0) & 1);
                
                if (_PESPrivateDataFlag)
                {
                    _headerLength += 16;
                }
                if (_packHeaderFieldFlag)
                {
                    _headerLength += 1;
                }
                if (_programPacketSequenceCounterFlag)
                {
                    _headerLength += 2;
                }
                if (_PSTDBufferFlag)
                {
                    _headerLength += 2;
                }
                if (_PESExtensionFlag2)
                {
                    _headerLength += 2;
                    buffer++;
                    for (int i = 6; i >= 0; i--)
                        _PESExtensionFieldLength += ((*buffer >> i) & 1) * static_cast<int>(pow(2, i));
                    buffer++;
                    _streamIDExtensionFlag = ((*buffer >> 7) & 1);

                    if (!_streamIDExtensionFlag)
                    {
                        
                    }
                    else
                    {
                        _trefExtensionFlag = ((*buffer >> 0) & 1);
                        if (_trefExtensionFlag)
                        {
                            _headerLength += 5;
                        }
                    }
                }
                for (int i = 0; i < _PESExtensionFieldLength; i++)
                {
                    _headerLength += 1;
                }

            }
        }

        

    }

    uint16_t GetPID() { return _prevPID; }

    void AddBytes()
    {
        _packetLength += 184;
    }

    void AddBytes(uint8_t remainingBytes)
    {
        _packetLength += remainingBytes;
    }

    void SaveToFile(char* buffer, uint8_t remainingBytes, uint16_t PID)
    {
        
        switch (PID)
        {
        case 136:
            outFile136.write(buffer, remainingBytes);
            break;

        case 174:
            outFile174.write(buffer, remainingBytes);
            break;
        }
        
    }

    uint16_t GetPrevPID() { return _prevPID; }

    uint8_t GetHeaderLen() { return _headerLength; }

    void Print()
    {
        cout << setfill(' ') <<
            " Started  PES: PSCP = " << setw(8) << static_cast<int>(_packetStartCodePrefix) <<
            " SID = " << setw(3) << static_cast<int>(_streamID) <<
            " PL = " << setw(3) << static_cast<int>(_PESPacketLength);

        if(_PTSDTSFlags == 0b10)
            cout << " PTS = " << setw(3) << static_cast<int>(_PTS);
        if(_PTSDTSFlags == 0b11)
            cout << 
            " PTS = " << setw(3) << static_cast<int>(_PTS) <<
            " DTS = " << setw(3) << static_cast<int>(_DTS);
    }

    void PrintEnd()
    {
        cout << " Finished PES: Len = " << setw(4) << static_cast<unsigned int>(_packetLength)
            << " HeaderLen = " << setw(4) << static_cast<int>(_headerLength)
            << " DataLen = " << setw(4) << static_cast<int>(_packetLength - _headerLength);
    }


    void Reset()
    {
        _packetStartCodePrefix              = 0;
        _streamID                           = 0;
        _PESPacketLength                    = 0;

        _PESScramblingControl               = 0;
        _PESPriority                        = 0;
        _dataAlignmentIndicator             = 0;
        _copyright                          = 0;
        _originalOrCopy                     = 0;
        _PTSDTSFlags                        = 0;
        _ESCRFlag                           = 0;
        _ESRateFlag                         = 0;
        _DSMTrickModeFlag                   = 0;
        _additionalCopyInfoFlag             = 0;
        _PESCRCFlag                         = 0;
        _PESExtensionFlag                   = 0;
        _PESHeaderDataLength                = 0;

        _PTS                                = 0;
        _DTS                                = 0;

        _ESCRBase                           = 0;
        _ESCRExtension                      = 0;

        _ESRate                             = 0;

        _PESPrivateDataFlag                 = 0;
        _packHeaderFieldFlag                = 0;
        _programPacketSequenceCounterFlag   = 0;
        _PSTDBufferFlag                     = 0;
        _PESExtensionFlag2                  = 0;

        _PESExtensionFieldLength            = 0;
        _streamIDExtensionFlag              = 0;

        _trefExtensionFlag                  = 0;

        _headerLength                       = 0;
        _packetLength                       = 0;

        _prevPID                            = 0;
    }
};

class Parser
{
private:
    TSHeader                _tsHeader;
    TSAdaptationField       _tsAF;
    PESPacket               _pesPacket;
    unsigned long long int  _packetNumber = 0;

    uint8_t                 _nextCC136 = 0;
    uint8_t                 _nextCC174 = 0;

public:
    void Parse(char* buffer)
    {
        
        char* tsHeaderBuffer = buffer;
        uint8_t bufferSize = 0;
        bufferSize += 4;
        char* tsAFBuffer = buffer + bufferSize;
        uint8_t prevStartBit = _tsHeader.GetStartBit();
        _tsHeader.Reset();
        _tsHeader.Parse(tsHeaderBuffer);
        //wait until the next packet and check if the PIDs check out
        if (_tsHeader.GetStartBit() && (_tsHeader.GetPID() == 136 || _tsHeader.GetPID() == 174))
        {
            _pesPacket.PrintEnd();
            _pesPacket.Reset();
        }

        cout << endl;

        cout << setfill('0') << setw(10) << _packetNumber++;

        _tsHeader.Print();

        if (_tsHeader.GetAFC() == 2 || _tsHeader.GetAFC() == 3)
        {

            _tsAF.Reset();
            _tsAF.Parse(tsAFBuffer);
            _tsAF.Print();
            bufferSize += _tsAF.GetAFL() + 1;

        }
        char* pesPacketBuffer = buffer + bufferSize;
        if ((_tsHeader.GetPID() == 136 && _tsHeader.GetCC() == _nextCC136) || (_tsHeader.GetPID() == 174 && _tsHeader.GetCC() == _nextCC174) )
        {
            if (_tsHeader.GetStartBit())
            {
                _pesPacket.HeaderParse(pesPacketBuffer);
                _pesPacket.AddBytes(188 - bufferSize);
                _pesPacket.SaveToFile(pesPacketBuffer + _pesPacket.GetHeaderLen(), 188 - bufferSize - _pesPacket.GetHeaderLen(), _tsHeader.GetPID());

                _pesPacket.Print();
            }
            else if (_tsHeader.GetAFC() == 1)
            {
                _pesPacket.AddBytes();
                _pesPacket.SaveToFile(pesPacketBuffer, 188 - bufferSize, _tsHeader.GetPID());
                cout << " Continue";
            }
            else
            {
                _pesPacket.AddBytes(188 - bufferSize);
                _pesPacket.SaveToFile(pesPacketBuffer, 188 - bufferSize, _tsHeader.GetPID());
            }
            if (_tsHeader.GetPID() == 136)
            {
                _nextCC136 = (_tsHeader.GetCC() == 15) ? 0 : (_tsHeader.GetCC() + 1);
            }
            else
            {

                _nextCC174 = (_tsHeader.GetCC() == 15) ? 0 : (_tsHeader.GetCC() + 1);
            }
        }
            

        
    }
};

int main()
{

    char c;
    char buffer[188];
    uint8_t counter = 0;
    Parser parser;
    while (inFile.get(c))
    {
        buffer[counter] = c;
        if (counter == 187)
        {
            counter = 0;
            parser.Parse(buffer);
        }
        else
        {
            counter++;
        }
            

    }
    inFile.close();
    outFile136.close();
    outFile174.close();
}
