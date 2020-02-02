////////////////////////////////////////////////////////////////////////////////////////////////////////
// Part of Injectable Generic Camera System
// Copyright(c) 2017, Frans Bouma
// All rights reserved.
// https://github.com/FransBouma/InjectableGenericCameraSystem
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
//  * Redistributions of source code must retain the above copyright notice, this
//	  list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and / or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CameraManipulator.h"
#include "GameConstants.h"
#include "Console.h"

using namespace DirectX;
using namespace std;

extern "C" {
	LPBYTE g_cameraStructAddress = nullptr;
	LPBYTE g_anselCameraStructAddress = nullptr;
	LPBYTE g_gamespeedStructAddress = nullptr;
	LPBYTE g_resolutionScaleMenuValueAddress = nullptr;
	LPBYTE g_fovStructAddress = nullptr;
	LPBYTE g_anselFovStructAddress = nullptr;
	LPBYTE g_originalQuaternion = nullptr;
	LPBYTE g_originalCoords = nullptr;
}

namespace IGCS::GameSpecific::CameraManipulator
{
	static float _originalLookData[4];
	static float _originalCoordsData[3];
	static bool _timeHasBeenStopped = false;
	
	LPBYTE currentCameraAddress() {
		return g_cameraStructAddress;
	}

	void setResolutionScaleMenuValueAddress(LPBYTE address)
	{
		g_resolutionScaleMenuValueAddress = address;
	}

	void changeSupersampling(LPBYTE hostImageAddress, float amount)
	{		
		float* superSamplingValueAddress = reinterpret_cast<float*>(SUPERSAMPLING_ADDRESS);
		float newValue = (*superSamplingValueAddress) + amount;
		if (newValue < 0.5f)
		{
			newValue = 0.5f;
		}
		*superSamplingValueAddress = newValue;
		cout << "The supersampling value is now: " + to_string(newValue) << endl;
		
	}
	
	// decreases / increases the gamespeed till 0.1f or 10f. 
	void modifyGameSpeed(bool decrease)
	{
		//disabled for now
		/*
		if (!_timeHasBeenStopped)
		{
			return;
		}
		float* gamespeedAddress = reinterpret_cast<float*>(g_gamespeedStructAddress + GAMESPEED_IN_STRUCT_OFFSET);
		float newValue = *gamespeedAddress;
		newValue += (decrease ? -0.1f : 0.1f);
		if (newValue < DEFAULT_MIN_GAME_SPEED)
		{
			newValue = DEFAULT_MIN_GAME_SPEED;
		}
		if (newValue > DEFAULT_MAX_GAME_SPEED)
		{
			newValue = DEFAULT_MAX_GAME_SPEED;
		}
		*gamespeedAddress = newValue;
		Console::WriteLine("Game speed is set to now: " + to_string(newValue));
		*/
	}

	// newValue: 1 == time should be frozen, 0 == normal gameplay
	void setTimeStopValue(BYTE newValue)
	{
		//disabled for now
		/*
		// set flag so camera works during menu driven timestop
		_timeHasBeenStopped = (newValue == 1);
		float* gamespeedAddress = reinterpret_cast<float*>(g_gamespeedStructAddress + GAMESPEED_IN_STRUCT_OFFSET);
		*gamespeedAddress = _timeHasBeenStopped ? 0.000001f : 1.0f;
		*/
	}


	XMFLOAT3 getCurrentCameraCoords()
	{
		float* coordsInMemory = reinterpret_cast<float*>(g_cameraStructAddress + CAMERA_COORDS_IN_CAMERA_STRUCT_OFFSET);
		XMFLOAT3 currentCoords = XMFLOAT3(coordsInMemory);
		return currentCoords;
	}


	// newLookQuaternion: newly calculated quaternion of camera view space. Can be used to construct a 4x4 matrix if the game uses a matrix instead of a quaternion
	// newCoords are the new coordinates for the camera in worldspace.
	void writeNewCameraValuesToGameData(XMVECTOR newLookQuaternion, XMFLOAT3 newCoords)
	{
		XMFLOAT4 qAsFloat4;
		XMStoreFloat4(&qAsFloat4, newLookQuaternion);

		// look Quaternion
		float* lookInMemory = reinterpret_cast<float*>(g_cameraStructAddress + LOOK_QUATERNION_IN_CAMERA_STRUCT_OFFSET);
		lookInMemory[0] = qAsFloat4.x;
		lookInMemory[1] = qAsFloat4.y;
		lookInMemory[2] = qAsFloat4.z;
		lookInMemory[3] = qAsFloat4.w;

		// Coords
		float* coordsInMemory = reinterpret_cast<float*>(g_cameraStructAddress + CAMERA_COORDS_IN_CAMERA_STRUCT_OFFSET);
		coordsInMemory[0] = newCoords.x;
		coordsInMemory[1] = newCoords.y;
		coordsInMemory[2] = newCoords.z;
	}

	void writeNewCameraValuesToAnsel(XMVECTOR newLookQuaternion, XMFLOAT3 newCoords)
	{
		XMFLOAT4 qAsFloat4;
		XMStoreFloat4(&qAsFloat4, newLookQuaternion);

		if (g_anselCameraStructAddress != nullptr) {
			// look Quaternion
			float* anselLookInMemory = reinterpret_cast<float*>(g_anselCameraStructAddress + ANSEL_LOOK_QUATERNION_IN_CAMERA_STRUCT_OFFSET);
			anselLookInMemory[0] = qAsFloat4.x;
			anselLookInMemory[1] = qAsFloat4.y;
			anselLookInMemory[2] = qAsFloat4.z;
			anselLookInMemory[3] = qAsFloat4.w;

			// Coords
			float* anselCoordsInMemory = reinterpret_cast<float*>(g_anselCameraStructAddress + ANSEL_CAMERA_COORDS_IN_CAMERA_STRUCT_OFFSET);
			anselCoordsInMemory[0] = newCoords.x;
			anselCoordsInMemory[1] = newCoords.y;
			anselCoordsInMemory[2] = newCoords.z;
		}
	}


	// Waits for the interceptor to pick up the camera struct address. Should only return if address is found 
	void waitForCameraStructAddresses()
	{
		Console::WriteLine("Waiting for camera struct interception...");
		while (nullptr == g_cameraStructAddress)
		{
			Sleep(100);
		}
		Console::WriteLine("Camera found.");

#ifdef _DEBUG
		cout << "Camera address: " << hex << (void*)g_cameraStructAddress << endl;
#endif
	}


	// Resets the FOV to the default
	void resetFoV()
	{
		float* fovInMemory = reinterpret_cast<float*>(g_fovStructAddress + FOV_IN_FOV_STRUCT_OFFSET);
		*fovInMemory = DEFAULT_FOV_DEGREES;
	}


	// changes the FoV with the specified amount
	void changeFoV(float amount)
	{
		float* fovInMemory = reinterpret_cast<float*>(g_fovStructAddress + FOV_IN_FOV_STRUCT_OFFSET);
		*fovInMemory += amount;
		if (g_anselFovStructAddress != nullptr) {
			float* anselFovInMemory = reinterpret_cast<float*>(g_anselFovStructAddress + ANSEL_FOV_IN_FOV_STRUCT_OFFSET);
			*anselFovInMemory += amount;
		}
	}


	// should restore the camera values in the camera structures to the cached values. This assures the free camera is always enabled at the original camera location.
	void restoreOriginalCameraValues()
	{
		float* lookInMemory = reinterpret_cast<float*>(g_cameraStructAddress + LOOK_QUATERNION_IN_CAMERA_STRUCT_OFFSET);
		float* coordsInMemory = reinterpret_cast<float*>(g_cameraStructAddress + CAMERA_COORDS_IN_CAMERA_STRUCT_OFFSET);
		memcpy(lookInMemory, _originalLookData, 4 * sizeof(float));
		memcpy(coordsInMemory, _originalCoordsData, 3 * sizeof(float));
	}


	void cacheOriginalCameraValues()
	{
		float* lookInMemory = reinterpret_cast<float*>(g_cameraStructAddress + LOOK_QUATERNION_IN_CAMERA_STRUCT_OFFSET);
		float* coordsInMemory = reinterpret_cast<float*>(g_cameraStructAddress + CAMERA_COORDS_IN_CAMERA_STRUCT_OFFSET);
		memcpy(_originalLookData, lookInMemory, 4 * sizeof(float));
		memcpy(_originalCoordsData, coordsInMemory, 3 * sizeof(float));

		g_originalCoords = reinterpret_cast<LPBYTE>(_originalCoordsData);
		g_originalQuaternion = reinterpret_cast<LPBYTE>(_originalLookData);
	}

}