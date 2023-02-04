// The MIT License (MIT)
//
// Copyright (c) 2013 Erick Fuentes http://erickfuent.es
// Copyright (c) 2014 Kuy Mainwaring http://logiblock.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "SDIOSimulationDataGenerator.h"
#include "SDIOAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

SDIOSimulationDataGenerator::SDIOSimulationDataGenerator() : mSerialText( "My first analyzer, woo hoo!" ), mStringIndex( 0 )
{
}

SDIOSimulationDataGenerator::~SDIOSimulationDataGenerator()
{
}

void SDIOSimulationDataGenerator::Initialize( U32 simulation_sample_rate, SDIOAnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mSerialSimulationData.SetChannel( mSettings->mInputChannel );
    mSerialSimulationData.SetSampleRate( simulation_sample_rate );
    mSerialSimulationData.SetInitialBitState( BIT_HIGH );
}

U32 SDIOSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate,
                                                         SimulationChannelDescriptor** simulation_channel )
{
    U64 adjusted_largest_sample_requested =
        AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while( mSerialSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
        CreateSerialByte();
    }

    *simulation_channel = &mSerialSimulationData;
    return 1;
}

void SDIOSimulationDataGenerator::CreateSerialByte()
{
    U32 samples_per_bit = mSimulationSampleRateHz / 1000;

    U8 byte = mSerialText[ mStringIndex ];
    mStringIndex++;
    if( mStringIndex == mSerialText.size() )
        mStringIndex = 0;

    // we're currently high
    // let's move forward a little
    mSerialSimulationData.Advance( samples_per_bit * 10 );

    mSerialSimulationData.Transition();               // low-going edge for start bit
    mSerialSimulationData.Advance( samples_per_bit ); // add start bit time

    U8 mask = 0x1 << 7;
    for( U32 i = 0; i < 8; i++ )
    {
        if( ( byte & mask ) != 0 )
            mSerialSimulationData.TransitionIfNeeded( BIT_HIGH );
        else
            mSerialSimulationData.TransitionIfNeeded( BIT_LOW );

        mSerialSimulationData.Advance( samples_per_bit );
        mask = mask >> 1;
    }

    mSerialSimulationData.TransitionIfNeeded( BIT_HIGH ); // we need to end high

    // lets pad the end a bit for the stop bit:
    mSerialSimulationData.Advance( samples_per_bit );
}
