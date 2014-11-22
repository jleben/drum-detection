//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// MIDI File Class Implementation
//

// N A M E S P A C E S
using namespace std;

// S Y S T E M  I N C L U D E S
#include <cstdint>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <string.h>

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "file.h"
#include "midifile.h"

// C O N S T A N T S
const char *Midifile::spcHeaderChunkId = "MThd";
const char *Midifile::spcTrackChunkId  = "MTrk";

// P U B L I C  M E T H O D S
Midifile::Midifile(string name, File::teMode eMode) : File(name, eMode)
{
    uint32_t uFormat;

	// Initialize data.
	mbOnce    = false;
	muChannel = scuDefaultChannel;
	muTrack   = 1;
	memset(&mrEvent, 0, sizeof(mrEvent));
	mpuBuffer = (uint8_t*)malloc(scuDefaultBufferLength);

    // Check mode
    if (eModeBinaryRead == eMode)
    {
        // Retrive meta data and validate file
        mbValid  = header(uFormat, muTracks, muDivision);
        mbValid &= tempo(muTempo);
        mbValid &= (scuSupportedFormat == uFormat) ? true : false;

        // Compute clock tick period
        mfTick = (float)((float)muTempo/1000000.0f/(float)muDivision);
    } else
    {
        // New file
        mbValid = true;
    }
}

Midifile::~Midifile()
{
	// Cleanup.
	free(mpuBuffer);
}

bool Midifile::valid()
{
    return (File::valid() && mbValid) ? true : false;
}

bool Midifile::eventRead(float &fTimestamp, uint32_t &uType, float &fStrength)
{
    uint32_t uTimestamp;
    uint32_t uNote;
    uint32_t uVelocity;

	// Get note on event
	if (noteOn(uTimestamp, uNote, uVelocity))
	{
		fTimestamp = uTimestamp*mfTick;
		uType = uNote;
		fStrength = (float)((float)uVelocity / (float)scuVelocityMax);

		return true;
	}

    return false;
}

bool Midifile::eventWrite(float fTimestamp, uint32_t uType, float fStrength)
{
	uint32_t uDelta;
	uint32_t uValue;
	uint32_t uBytes;
	uint32_t uLength;
	uint32_t uTime;
	uint8_t  uEvent;
	uint8_t  uNote;
	uint8_t  uVelocity;
	float    fTick;
	bool     bStatus;

	// Compute tick
	fTick = (float)((float)muTempo/1000000.0f/(float)muDivision);

	// Translate to MIDI
	uTime = (uint32_t)(fTimestamp / fTick);
	uEvent = eEventNoteOn | scuDefaultChannel;
	uNote = (uint8_t)uType;
	uVelocity = (uint8_t)(scuVelocityMax*fStrength);

	// Compute delta
	uDelta = uTime - muTime;
	muTime = uTime;
	length(uDelta, uValue, uBytes);

	// Format and write
	uLength = 0UL;
	bStatus = write(&uValue, uBytes);
	muLength += uBytes;
	if (!mbOnce)
	{
		mbOnce = true;
		bStatus &= write(&uEvent, sizeof(uEvent));
		uLength++;
	}
	bStatus &= write(&uNote, sizeof(uNote));
	uLength++;
	bStatus &= write(&uVelocity, sizeof(uVelocity));
	uLength++;

	// Update running length
	muLength += uLength;

    return bStatus;
}

bool Midifile::headerWrite(uint32_t uDivision, uint32_t uTempo)
{
	trHeader	 rHeader;
	trTrack		 rTrack;
	struct
	{
		uint8_t uDelta;
		uint8_t uEvent;
		uint8_t uMeta;
		uint8_t uLength;
		uint8_t uTempo[3];
	} rEventTempo;
	struct
	{
		uint8_t uDelta;
		uint8_t uEvent;
		uint8_t uMeta;
		uint8_t uLength;
	} rEventEndOfTrack;

    // Store arguments
    muDivision = uDivision;
    muTempo    = uTempo;

    // Format and write header
	memset(&rHeader, 0, sizeof(rHeader));
	memcpy(rHeader.id, spcHeaderChunkId, sizeof(rHeader.id));
	rHeader.uLength   = bigEndian((uint32_t)6UL);
	rHeader.uFormat   = bigEndian(scuSupportedFormat);
	rHeader.uTracks   = bigEndian((uint16_t)2U);
	rHeader.uDivision = bigEndian((uint16_t)muDivision);
	if (!write(&rHeader, sizeof(rHeader)))
	{
		return false;
	}

    // Format and write start track
	memset(&rTrack, 0, sizeof(rTrack));
	memcpy(rTrack.id, spcTrackChunkId, sizeof(rTrack.id));
	rTrack.uLength = sizeof(rEventTempo)+sizeof(rEventEndOfTrack);
	rTrack.uLength = bigEndian(rTrack.uLength);
	if (!write(&rTrack, sizeof(rTrack)))
	{
		return false;
	}

    // Format and write tempo
	memset(&rEventTempo, 0, sizeof(rEventTempo));
	rEventTempo.uDelta    = 0;
	rEventTempo.uEvent    = eEventMeta;
	rEventTempo.uMeta     = eMetaSetTempo;
	rEventTempo.uLength   = 3;
	rEventTempo.uTempo[0] = (uTempo & 0xff0000) >> 16;
	rEventTempo.uTempo[1] = (uTempo & 0xff00) >> 8;
	rEventTempo.uTempo[2] = uTempo & 0xff;
	if (!write(&rEventTempo, sizeof(rEventTempo)))
	{
		return false;
	}

    // Format and write end track
	memset(&rEventEndOfTrack, 0, sizeof(rEventEndOfTrack));
	rEventEndOfTrack.uDelta  = 0;
	rEventEndOfTrack.uEvent  = eEventMeta;
	rEventEndOfTrack.uMeta   = eMetaEndOfTrack;
	rEventEndOfTrack.uLength = 0;
	if (!write(&rEventEndOfTrack, sizeof(rEventEndOfTrack)))
	{
		return false;
	}

    // Format and write start track
	muLength = 0;
	memset(&rTrack, 0, sizeof(rTrack));
	memcpy(rTrack.id, spcTrackChunkId, sizeof(rTrack.id));
	rTrack.uLength = muLength;
	if (!write(&rTrack, sizeof(rTrack)))
	{
		return false;
	}

	// Clear running time.
	muTime = 0UL;

    return true;
}

bool Midifile::footerWrite(float fTimestamp)
{
	uint32_t	 uDelta;
	uint32_t	 uTime;
	uint32_t	 uValue;
	uint32_t	 uBytes;
    float		 fTick;
	struct
	{
		uint8_t uEvent;
		uint8_t uMeta;
		uint8_t uLength;
	} rEventEndOfTrack;

    // Compute tick
    fTick = (float)((float)muTempo/1000000.0f/(float)muDivision);

    // Translate to MIDI
    uTime = (uint32_t)(fTimestamp/fTick);

	// Compute delta
	uDelta = uTime - muTime;
	muTime = uTime;
	length(uDelta, uValue, uBytes);

    // Format and write end track
	if (!write(&uValue, uBytes))
	{
		return false;
	}
	memset(&rEventEndOfTrack, 0, sizeof(rEventEndOfTrack));
	rEventEndOfTrack.uEvent = eEventMeta;
	rEventEndOfTrack.uMeta = eMetaEndOfTrack;
	rEventEndOfTrack.uLength = 0;
	if (!write(&rEventEndOfTrack, sizeof(rEventEndOfTrack)))
	{
		return false;
	}

	// Update running length
	muLength += uBytes + sizeof(rEventEndOfTrack);

    // Update track length field
	uValue = bigEndian(muLength);
	seek(37UL);
	if (!write(&uValue, sizeof(uValue)))
	{
		return false;
	}

    return true;
}

bool Midifile::header(uint32_t &uFormat, uint32_t &uTracks, uint32_t &uDivision)
{
	trHeader rHeader;

	// Rewind file.
	reset();

	// Read header.
	if (!read(&rHeader, sizeof(rHeader)))
	{
		return false;
	}

	// Validate ID.
	if (0 != strncmp(rHeader.id, spcHeaderChunkId, sizeof(rHeader.id)))
	{
		return false;
	}

	// Fix endianess.
	rHeader.uLength   = bigEndian(rHeader.uLength);
	assert(6 == rHeader.uLength);
	rHeader.uFormat   = bigEndian(rHeader.uFormat);
	rHeader.uTracks   = bigEndian(rHeader.uTracks);
	rHeader.uDivision = bigEndian(rHeader.uDivision);

	// Assign references.
	uFormat   = rHeader.uFormat;
	uTracks   = rHeader.uTracks;
	uDivision = rHeader.uDivision;

	return true;
}

bool Midifile::tempo(uint32_t &uValue)
{
	trTrack  rTrack;
	trEvent  rEvent;

	// Seek past header.
	seek(sizeof(trHeader));

	// Read track.
	if (!read(&rTrack, sizeof(rTrack)))
	{
		return false;
	}

	// Validate ID.
	if (0 != strncmp(rTrack.id, spcTrackChunkId, sizeof(rTrack.id)))
	{
		return false;
	}

	// Fix endianess.
	rTrack.uLength = bigEndian(rTrack.uLength);

	// Iterate over events.
	memset(&rEvent, 0, sizeof(rEvent));
	while (event(rEvent))
	{
		// Check for "set tempo" event.
		if ((eEventMeta == rEvent.eEvent) && (eMetaSetTempo == rEvent.eMeta))
		{
			uValue = rEvent.rSetTempo.uUsPerQuarterNote;

			return true;
		}
	}

	return false;
}

uint32_t Midifile::tempo(void)
{
    return muTempo;
}

uint32_t Midifile::division(void)
{
    return muDivision;
}

// P R I V A T E  M E T H O D S
bool Midifile::event(trEvent &rEvent)
{
	bool	bStatus;
	uint8_t uEvent;

	// Read delta time.
	if (!length(rEvent.uDelta))
	{
		return false;
	}

	// Read event.
	if (!read(&uEvent, sizeof(uEvent)))
	{
		return false;
	}

	// Check for running status.
	if (uEvent & eEventClass)
	{
		rEvent.eEvent = (teEvent)uEvent;
	}
	else
	{
		// Reduce the current file position by one.
		unget();
	}

	// Process event type.
	switch (rEvent.eEvent & eEventMask)
	{
		case eEventNoteOff:
			rEvent.rNoteOff.uChannel = rEvent.eEvent & eEventChannelMask;
			bStatus  = read(&rEvent.rNoteOff.uNote, sizeof(rEvent.rNoteOff.uNote));
			bStatus &= read(&rEvent.rNoteOff.uVelocity, sizeof(rEvent.rNoteOff.uVelocity));
			break;

		case eEventNoteOn:
			rEvent.rNoteOn.uChannel = rEvent.eEvent & eEventChannelMask;
			bStatus = read(&rEvent.rNoteOn.uNote, sizeof(rEvent.rNoteOn.uNote));
			bStatus &= read(&rEvent.rNoteOn.uVelocity, sizeof(rEvent.rNoteOn.uVelocity));
			break;

		case eEventNoteAftertouch:
			rEvent.rNoteOn.uChannel = rEvent.eEvent & eEventChannelMask;
			bStatus = read(&rEvent.rNoteOn.uNote, sizeof(rEvent.rNoteOn.uNote));
			bStatus &= read(&rEvent.rNoteAftertouch.uAftertouch, 
							sizeof(rEvent.rNoteAftertouch.uAftertouch));
			break;


		case eEventController:
			rEvent.rController.uChannel = rEvent.eEvent & eEventChannelMask;
			bStatus = read(&rEvent.rController.uNumber, 
						sizeof(rEvent.rController.uNumber));
			bStatus &= read(&rEvent.rController.uValue,
						sizeof(rEvent.rController.uValue));
			break;

		case eEventProgramChange:
			rEvent.rProgramChange.uChannel = rEvent.eEvent & eEventChannelMask;
			bStatus = read(&rEvent.rProgramChange.uNumber,
						sizeof(rEvent.rProgramChange.uNumber));
			break;

		case eEventChannelAftertouch:
			rEvent.rProgramChange.uChannel = rEvent.eEvent & eEventChannelMask;
			bStatus = read(&rEvent.rChannelAftertouch.uValue,
						sizeof(rEvent.rChannelAftertouch.uValue));
			break;

		case eEventPitchBend:
			rEvent.rPitchBend.uChannel = rEvent.eEvent & eEventChannelMask;
			bStatus = read(&rEvent.rPitchBend.uValue,
						sizeof(rEvent.rPitchBend.uValue));
			rEvent.rPitchBend.uValue = bigEndian(rEvent.rPitchBend.uValue);
			break;

		case eEventSysEx:

			// Check event type.
			if (eEventSysEx == rEvent.eEvent)
			{
				if (!length(rEvent.rSysEx.uLength))
				{
					return false;
				}
				mpuBuffer = (uint8_t*)realloc(mpuBuffer,
								rEvent.rSysEx.uLength);
				assert(NULL != mpuBuffer);
				rEvent.rSysEx.puBuffer = mpuBuffer;
				bStatus = read(rEvent.rSysEx.puBuffer, rEvent.rSysEx.uLength);

				return bStatus;
			}

			// Read meta type.
			if (!read(&rEvent.eMeta, sizeof(uint8_t)))
			{
				return false;
			}

			// Process meta data.
			switch (rEvent.eMeta)
			{
				case eMetaSequence:
					bStatus = read(&rEvent.rSequenceNumber.uLength,
								sizeof(rEvent.rSequenceNumber.uLength));
					bStatus &= read(&rEvent.rSequenceNumber.uValue,
								rEvent.rSequenceNumber.uLength);
					assert(2 == rEvent.rSequenceNumber.uLength);
					rEvent.rSequenceNumber.uValue =
						bigEndian(rEvent.rSequenceNumber.uValue);
					break;

				case eMetaText:
				case eMetaCopyright:
				case eMetaSeqTrackName:
				case eMetaInstrumentName:
				case eMetaLyrics:
				case eMetaMarker:
				case eMetaCuePoint:
					if (!length(rEvent.rString.uLength))
					{
						return false;
					}
					mpuBuffer = (uint8_t*)realloc(mpuBuffer, 
									rEvent.rString.uLength);;
					assert(NULL != mpuBuffer);
					rEvent.rString.pBuffer = (char*)mpuBuffer;
					bStatus = read(rEvent.rString.pBuffer, 
								rEvent.rString.uLength);
					break;

				case eMetaMidiChannelPrefix:
					bStatus  = read(&rEvent.rChannelPrefix.uLength,
								sizeof(rEvent.rChannelPrefix.uLength));
					assert(1 == rEvent.rChannelPrefix.uLength);
					bStatus &= read(&rEvent.rChannelPrefix.uChannel,
								rEvent.rChannelPrefix.uLength	);
					break;

				case eMetaEndOfTrack:
					bStatus = read(&rEvent.rEndOfTrack.uLength,
								sizeof(rEvent.rEndOfTrack.uLength));
					assert(0 == rEvent.rEndOfTrack.uLength);
					break;

				case eMetaSetTempo:
					uint32_t uValue;
					bStatus = read(&rEvent.rSetTempo.uLength,
								sizeof(rEvent.rSetTempo.uLength));
					assert(3 == rEvent.rSetTempo.uLength);
					uValue = 0;
					bStatus = read(&uValue, rEvent.rSetTempo.uLength);
					rEvent.rSetTempo.uUsPerQuarterNote  = (uValue & 0x000000ff) << 16;
					rEvent.rSetTempo.uUsPerQuarterNote += (uValue & 0x0000ff00);
					rEvent.rSetTempo.uUsPerQuarterNote += (uValue & 0x00ff0000) >> 16;
					break;

				case eMetaSmpteOffset:
					bStatus  = read(&rEvent.rSmpteOffset.uLength,
								sizeof(rEvent.rSmpteOffset.uLength));
					assert(5 == rEvent.rSmpteOffset.uLength);
					bStatus &= read(&rEvent.rSmpteOffset.uHour,
								sizeof(rEvent.rSmpteOffset.uHour));
					bStatus &= read(&rEvent.rSmpteOffset.uMin,
								sizeof(rEvent.rSmpteOffset.uMin));
					bStatus &= read(&rEvent.rSmpteOffset.uSec,
								sizeof(rEvent.rSmpteOffset.uSec));
					bStatus &= read(&rEvent.rSmpteOffset.uFrame,
								sizeof(rEvent.rSmpteOffset.uFrame));
					bStatus &= read(&rEvent.rSmpteOffset.uSubFrame,
								sizeof(rEvent.rSmpteOffset.uSubFrame));
					break;

				case eMetaTimeSignature:
					bStatus  = read(&rEvent.rTimeSignature.uLength,
								sizeof(rEvent.rTimeSignature.uLength));
					assert(4 == rEvent.rTimeSignature.uLength);
					bStatus &= read(&rEvent.rTimeSignature.uNumerator,
								sizeof(rEvent.rTimeSignature.uNumerator));
					bStatus &= read(&rEvent.rTimeSignature.uDenominator,
								sizeof(rEvent.rTimeSignature.uDenominator));
					bStatus &= read(&rEvent.rTimeSignature.uMetronome,
								sizeof(rEvent.rTimeSignature.uMetronome));
					bStatus &= read(&rEvent.rTimeSignature.uThritySeconds,
								sizeof(rEvent.rTimeSignature.uThritySeconds));
					break;

				case eMetaKeySignature:
					bStatus =  read(&rEvent.rKeySignature.uLength,
								sizeof(rEvent.rKeySignature.uLength));
					assert(2 == rEvent.rKeySignature.uLength);
					bStatus &= read(&rEvent.rKeySignature.uKey,
								sizeof(rEvent.rKeySignature.uKey));
					bStatus &= read(&rEvent.rKeySignature.uScale,
								sizeof(rEvent.rKeySignature.uScale));
					break;

				case eMetaSequencerSpecific:
					if (!length(rEvent.rSequencerSpecific.uLength))
					{
						return false;
					}
					mpuBuffer = (uint8_t*)realloc(rEvent.rSequencerSpecific.puBuffer,
									rEvent.rSequencerSpecific.uLength);
					assert(NULL != mpuBuffer);
					rEvent.rSequencerSpecific.puBuffer = mpuBuffer;
					bStatus = read(rEvent.rSequencerSpecific.puBuffer,
								rEvent.rSequencerSpecific.uLength);
					break;

				default:
					return false;
			}

			return bStatus;

		default:
			return false;
	}

    return bStatus;
}

bool Midifile::length(uint32_t &uLength)
{
	uint8_t uData;

	// Clear length.
	uLength = 0;

	// Read variable length value.
	for (uint32_t uIndex = 0; uIndex < sizeof(uLength); uIndex++)
	{
		// Read portion of delta time.
		if (!read(&uData, sizeof(uData)))
		{
			return false;
		}

		// Assign length.
		uLength += uData & 0x7f;

		// Check for next byte.
		if (uData & 0x80)
		{
			uLength <<= 7;
		}
		else
		{
			return true;
		}
	}

	return false;
}

void Midifile::length(uint32_t uLength, uint32_t &uValue, uint32_t &uBytes)
{
	// Process length.
	uBytes = 1;
	uValue = uLength & 0x7f;
	while ((uLength >>= 7) > 0)
	{
		uValue <<= 8;
		uValue  |= 0x80;
		uValue += (uLength & 0x7f);
		uBytes++;
	}

}

uint32_t Midifile::bigEndian(uint32_t uValue)
{
	uint32_t uData;

	uData  = (uValue & 0x000000ffUL) << 24U;
	uData |= (uValue & 0x0000ff00UL) << 8U;
	uData |= (uValue & 0x00ff0000UL) >> 8U;
	uData |= (uValue & 0xff000000UL) >> 24U;

	return uData;
}

uint16_t Midifile::bigEndian(uint16_t uValue)
{
	uint16_t uData;

	uData  = (uValue & 0x00FFU) << 8;
	uData |= (uValue & 0xff00U) >> 8;

	return uData;
}

bool Midifile::noteOn(uint32_t &uTimestamp, uint32_t &uNote, uint32_t &uVelocity)
{
	static bool bOnce = false;
	trTrack	    rTrack;

	// Check for first call.
	if (!bOnce)
	{
		// Update state.
		bOnce = true;

		// Reset timestamp and seek to current track.
		muTime = 0;
		if (!seekToTrack(muTrack))
		{
			return false;
		}
	}

	// Iterate over events.
	memset(&mrEvent, 0, sizeof(mrEvent));
	while (event(mrEvent))
	{
		// Check for "note on" event.
		if ((eEventNoteOn == (mrEvent.eEvent & eEventMask)) && (muChannel == mrEvent.rNoteOn.uChannel))
		{
			// Update running time.
			muTime += mrEvent.uDelta;

			// Assign arguments.
			uTimestamp = muTime;
			uNote	   = mrEvent.rNoteOn.uNote;
			uVelocity  = mrEvent.rNoteOn.uVelocity;

			return true;
		}

		// Check for "end of track" event.
		if ((eEventMeta == mrEvent.eEvent) && (eMetaEndOfTrack == mrEvent.eMeta))
		{
			// Check track index.
			if (++muTrack >= muTracks)
			{
				return false;
			}

			// Reset timestamp and seek to current track.
			muTime = 0;
			if (!seekToTrack(muTrack))
			{
				return false;
			}
		}
	}

	return false;
}

void Midifile::channel(uint32_t uValue)
{
	muChannel = uValue;
}

bool Midifile::seekToTrack(uint32_t uTrack)
{
	trTrack	 rTrack;
	uint32_t uOffset;

	// Check track index.
	if (uTrack >= muTracks)
	{
		return false;
	}

	// Compute offset to track.
	uOffset = sizeof(trHeader);
	for (uint32_t uIndex = 0; uIndex < muTrack; uIndex++)
	{
		// Seek to track.
		seek(uOffset);

		// Read track.
		if (!read(&rTrack, sizeof(rTrack)))
		{
			return false;
		}

		// Validate ID.
		if (0 != strncmp(rTrack.id, spcTrackChunkId, sizeof(rTrack.id)))
		{
			return false;
		}

		// Fix endianess.
		rTrack.uLength = bigEndian(rTrack.uLength);

		// Update offset.
		uOffset += sizeof(rTrack) + rTrack.uLength;
	}

	return true;
}