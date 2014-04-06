//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// MIDI FILE Class Definition
//
#ifndef _MIDIFILE_H
#define _MIDIFILE_H

// P R A G M A S
#pragma pack(1)

// C L A S S
class Midifile : public File
{
public:  

    // Constant[s]
    static const char	  *spcHeaderChunkId;
	static const char	  *spcTrackChunkId;
	static const uint8_t   scuDefaultChannel	  = 9;
	static const uint8_t   scuVelocityMax		  = 127;
    static const uint16_t  scuSupportedFormat	  = 1;
	static const uint32_t  scuDefaultBufferLength = 1024;
	typedef enum
	{
		eEventNone				= 0x00,
		eEventNoteOff			= 0x80,
		eEventNoteOn			= 0x90,
		eEventNoteAftertouch	= 0xa0,
		eEventController		= 0xb0,
		eEventProgramChange		= 0xc0,
		eEventChannelAftertouch = 0xd0,
		eEventPitchBend			= 0xe0,
		eEventSysEx				= 0xf0,
		eEventMeta				= 0xff,
		eEventClass 			= 0x80,
		eEventMask				= 0xf0,
		eEventChannelMask		= 0x0f
	} teEvent;
	typedef enum
	{
		eMetaSequence			= 0x00,
		eMetaText				= 0x01,
		eMetaCopyright			= 0x02,
		eMetaSeqTrackName		= 0x03,
		eMetaInstrumentName		= 0x04,
		eMetaLyrics				= 0x05,
		eMetaMarker				= 0x06,
		eMetaCuePoint			= 0x07,
		eMetaMidiChannelPrefix	= 0x20,
		eMetaEndOfTrack			= 0x2f,
		eMetaSetTempo			= 0x51,
		eMetaSmpteOffset		= 0x54,
		eMetaTimeSignature		= 0x58,
		eMetaKeySignature		= 0x59,
		eMetaSequencerSpecific	= 0x7f,
		eMetaNone				= 0xff
	} teMeta;

    // Constructor[s]
    Midifile(string name, File::teMode eMode);

    // Destructor
    ~Midifile();

    // Method[s]
    bool     valid();
    bool     eventRead(float &fTimestamp, uint32_t &uType, float &fStrength);
    bool     eventWrite(float fTimestamp, uint32_t uType, float fStrength);
    bool     header(uint32_t &uFormat, uint32_t &uTracks, uint32_t &uDivision);
    bool     headerWrite(uint32_t uDivision, uint32_t uTempo);
    bool     footerWrite(float fTimestamp);
    bool     tempo(uint32_t &uValue);
    uint32_t tempo(void);
    uint32_t division(void);

private:

	// Data Structure[s]
	typedef struct
	{
		char	 id[4];
		uint32_t uLength;
		uint16_t uFormat;
		uint16_t uTracks;
		uint16_t uDivision;
	} trHeader;
	typedef struct
	{
		char     id[4];
		uint32_t uLength;
	} trTrack;
	typedef struct
	{
		uint8_t uChannel;
		uint8_t uNote;
		uint8_t uVelocity;
	} trNoteOff;
	typedef struct
	{
		uint8_t uChannel;
		uint8_t uNote;
		uint8_t uVelocity;
	} trNoteOn;
	typedef struct
	{
		uint8_t uChannel;
		uint8_t uNote;
		uint8_t uAftertouch;
	} trNoteAftertouch;
	typedef struct
	{
		uint8_t uChannel;
		uint8_t uNumber;
		uint8_t uValue;
	} trController;
	typedef struct
	{
		uint8_t uChannel;
		uint8_t uNumber;
	} trProgramChange;
	typedef struct
	{
		uint8_t uChannel;
		uint8_t uValue;
	} trChannelAftertouch;
	typedef struct
	{
		uint8_t uChannel;
		uint16_t uValue;
	} trPitchBend;
	typedef struct
	{
		uint8_t  uLength;
		uint16_t uValue;
	} trSequenceNumber;
	typedef struct
	{
		uint32_t  uLength;
		char     *pBuffer;
	} trString;
	typedef struct
	{
		uint8_t uLength;
		uint8_t uChannel;
	} trChannelPrefix;
	typedef struct
	{
		uint8_t uLength;
	} trEndOfTrack;
	typedef struct
	{
		uint8_t  uLength;
		uint32_t uUsPerQuarterNote;
	} trSetTempo;
	typedef struct
	{
		uint8_t uLength;
		uint8_t uHour;
		uint8_t uMin;
		uint8_t uSec;
		uint8_t uFrame;
		uint8_t uSubFrame;
	} trSmpteOffset;
	typedef struct
	{
		uint8_t uLength;
		uint8_t uNumerator;
		uint8_t uDenominator;
		uint8_t uMetronome;
		uint8_t uThritySeconds;
	} trTimeSignature;
	typedef struct
	{
		uint8_t uLength;
		uint8_t uKey;
		uint8_t uScale;
	} trKeySignature;
	typedef struct
	{
		uint32_t  uLength;
		uint8_t  *puBuffer;
	} trSequencerSpecific;
	typedef struct
	{
		uint32_t  uLength;
		uint8_t  *puBuffer;
	} trSysEx;
	typedef struct
	{
		uint32_t			uDelta;
		teEvent				eEvent;
		teMeta				eMeta;
		union
		{
			trNoteOff			rNoteOff;
			trNoteOn			rNoteOn;
			trNoteAftertouch	rNoteAftertouch;
			trController		rController;
			trProgramChange		rProgramChange;
			trChannelAftertouch rChannelAftertouch;
			trPitchBend			rPitchBend;
			trSequenceNumber	rSequenceNumber;
			trString			rString;
			trChannelPrefix		rChannelPrefix;
			trEndOfTrack		rEndOfTrack;
			trSetTempo			rSetTempo;
			trSmpteOffset		rSmpteOffset;
			trTimeSignature		rTimeSignature;
			trKeySignature		rKeySignature;
			trSequencerSpecific rSequencerSpecific;
			trSysEx				rSysEx;
		};
	} trEvent;

    // Method[s]
	bool	 event(trEvent &rEvent);
	bool	 length(uint32_t &uLength);
	void	 length(uint32_t uLength, uint32_t &uValue, uint32_t &uBytes);
	bool	 noteOn(uint32_t &uTimestamp, uint32_t &uNote, uint32_t &uVelocity);
	uint32_t bigEndian(uint32_t uValue);
	uint16_t bigEndian(uint16_t uValue);

    // Data
    bool     mbValid;
	bool	 mbOnce;
    uint32_t muDivision;
    uint32_t muTempo;
	uint32_t muTime;
	uint32_t muLength;
    float    mfTick;
	uint8_t *mpuBuffer;
	trEvent  mrEvent;
};

#endif // _MIDIFILE_H