/**
 * File : huit.cpp
 *
 * Copyright (c) 2010, Paul ADENOT
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Paul ADENOT nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Paul ADENOT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **/

#include "huit.h"

#include <sstream>
#include <math.h>

AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new Huit (audioMaster);
}

Huit::Huit (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, 3)	// 1 program, 1 parameter only
,undersampling_factor_(50.0f)
,fGain_(1.0f)
,gate_(0.0f)
{
	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID ('huIt');	// identify
	canProcessReplacing ();	// supports replacing output
	canDoubleReplacing ();	// supports double precision processing

	setParameter(undersampling, 0.0f);

	vst_strncpy (programName, "Huit", kVstMaxProgNameLen);	// default program name
}


Huit::~Huit ()
{ }


void Huit::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}


void Huit::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}


void Huit::setParameter (VstInt32 index, float value)
{
	switch(index)
	{
	case gain:
		fGain_ = value;
		break;
	case undersampling:
		if(value*undersampling_factor_ > 1)
			undersampling_ = static_cast<float>(static_cast<int>(value * undersampling_factor_));
		else
			undersampling_ = 1.0f;
		break;
	case gate:
		gate_ = value;
		break;
	}
}


float Huit::getParameter (VstInt32 index)
{
	switch(index)
	{
	case gain :
		return fGain_;
	case undersampling :
		return undersampling_;
	case gate:
		return gate_;
	}
	return 0.0f;
}


void Huit::getParameterName (VstInt32 index, char* label)
{
	switch (index)
	{
	case gain :
		vst_strncpy (label, "Gain", kVstMaxParamStrLen);
		break;
	case undersampling :
		vst_strncpy (label, "Undersampling", kVstMaxParamStrLen);
		break;
	case gate :
		vst_strncpy(label, "Gate", kVstMaxParamStrLen);
		break;
	}
}


void Huit::getParameterDisplay (VstInt32 index, char* text)
{
	switch(index)
	{
	case gain:
		dB2string (fGain_, text, kVstMaxParamStrLen);
		break;
	case undersampling:
		{
			std::stringstream strs;
			strs << static_cast<int>(undersampling_);
			vst_strncpy(text, strs.str().c_str(), kVstMaxParamStrLen);
		}
		break;
	case gate:
		//dB2string(gate_, text, kVstMaxParamStrLen);
				{
			std::stringstream strs;
			strs << gate_;
			vst_strncpy(text, strs.str().c_str(), kVstMaxParamStrLen);
		}
				break;
	}
}


void Huit::getParameterLabel (VstInt32 index, char* label)
{
	switch(index)
	{
	case gain:
		vst_strncpy (label, "dB", kVstMaxParamStrLen);
		break;
	case undersampling:
		vst_strncpy (label, "x", kVstMaxParamStrLen);
		break;
	case gate:
		vst_strncpy(label, "dB", kVstMaxParamStrLen);
	}
}


bool Huit::getEffectName (char* name)
{
	vst_strncpy (name, "HuitProto", kVstMaxEffectNameLen);
	return true;
}


bool Huit::getProductString (char* text)
{
	vst_strncpy (text, "HuitProto", kVstMaxProductStrLen);
	return true;
}


bool Huit::getVendorString (char* text)
{
	vst_strncpy (text, "Paul ADENOT", kVstMaxVendorStrLen);
	return true;
}


VstInt32 Huit::getVendorVersion ()
{ 
	return 1000; 
}


void Huit::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	// Left input
    float* in1  =  inputs[0];
    // Right input
	float* in2  =  inputs[1];
    // Left output
	float* out1 = outputs[0];
	// Right output
	float* out2 = outputs[1];

	// Accumulator for the left buffer.
	float acc_left = 0.0f;
	// Accumulator for the right buffer
	float acc_right = 0.0f;
	int cursor = 0;

    while (--sampleFrames >= 0)
    {
		if(++cursor == static_cast<int>(undersampling_) && sampleFrames >= undersampling_)
		{
			// Compute mean value using undersampling_ values.
			for(int i = 0; i < undersampling_; ++i)
			{
				acc_left  += *(out1 - i);
				acc_right += *(out2 - i);
			}

			acc_left  /= undersampling_;
			acc_right /= undersampling_;
			
			for(int i=0; i < undersampling_; ++i)
			{
				*(out1 - i) = acc_left;
				*(out2 - i) = acc_left;
			}
			cursor = 0;
		}
		else
		{
			*out1 = *in1;
			*out2 = *in2;
		}
        (*out1++) = (*out1) * fGain_;
        (*out2++) = (*out2) * fGain_;
    }
}


void Huit::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
    double* in1  =  inputs[0];
    double* in2  =  inputs[1];
    double* out1 = outputs[0];
    double* out2 = outputs[1];
	double dGain = fGain_;

			double acc_left = 0.0f;
	double acc_right = 0.0f;
	int cursor = 0;

    while (--sampleFrames >= 0)
    {
		// In place computation
		*out1 = *in1++;
		*out2 = *in2++;
		// Downsampler stage
		if(++cursor == static_cast<int>(undersampling_) && sampleFrames >= undersampling_)
		{
			for(int i = 0; i < undersampling_; ++i)
			{
				acc_left  += *(out1 - i);
				acc_right += *(out2 - i);
			}

			acc_left  /= undersampling_;
			acc_right /= undersampling_;
			
			for(int i=0; i < undersampling_; ++i)
			{
				*(out1 - i) = acc_left;
				*(out2 - i) = acc_left;
			}
			cursor = 0;
			acc_left = 0.0f;
			acc_right = 0.0f;
		}

		// Gate or Gain stage
		// abs(*out1++) >= gate_ ? *out1 = 0.0f : *out1 *= dGain;
		// abs(*out2++) >= gate_ ? *out2 = 0.0f : *out2 *= dGain;
	
		out1++;
		out2++;
	}
}
