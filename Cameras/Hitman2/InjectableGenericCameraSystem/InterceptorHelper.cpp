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
#include <map>
#include "InterceptorHelper.h"
#include "GameConstants.h"
#include "GameImageHooker.h"
#include "Utils.h"
#include "AOBBlock.h"
#include "Console.h"
#include "CameraManipulator.h"

using namespace std;

//--------------------------------------------------------------------------------------------------------------------------------
// external asm functions
extern "C" {
	void cameraAddressInterceptor();
	void cameraWriteInterceptor1();		// create as much interceptors for write interception as needed. In the example game, there are 4.
	void fovWriteInterceptor();

	void anselUnlock();
	void anselCameraAddressInterceptor();
	void anselCameraWriteInterceptor();
	void anselFovWriteInterceptor();
}

// external addresses used in asm.
extern "C" {
	LPBYTE _cameraStructInterceptionContinue = nullptr;
	LPBYTE _cameraWriteInterceptionContinue1 = nullptr;
	LPBYTE _gamespeedInterceptionContinue = nullptr;
	LPBYTE _cameraReadInterceptionContinue1 = nullptr;
	LPBYTE _fovWriteInterceptionContinue = nullptr;
	LPBYTE _anselUnlockContinue = nullptr;
	LPBYTE _anselCameraInterceptContinue = nullptr;
	LPBYTE _anselCameraWriteContinue = nullptr;
	LPBYTE _anselFovWriteContinue = nullptr;
}


namespace IGCS::GameSpecific::InterceptorHelper
{
	void initializeAOBBlocks(LPBYTE hostImageAddress, DWORD hostImageSize, map<string, AOBBlock*> &aobBlocks)
	{
		aobBlocks[CAMERA_ADDRESS_INTERCEPT_KEY] = new AOBBlock(CAMERA_ADDRESS_INTERCEPT_KEY, "89 83 00020000 8B 83 D4030000 89 43 2C 8B 83 D0030000 89 43 30 48 8B 01", 1);
		aobBlocks[CAMERA_WRITE_INTERCEPT1_KEY] = new AOBBlock(CAMERA_WRITE_INTERCEPT1_KEY, "0F11 43 50 F3 0F10 44 24 50 F3 0F11 43 60 F3 0F10 44 24 58 F3 0F11 43 68 F3 0F11 4B 64 EB 2F 48 8D 4C 24 60", 3);
		//aobBlocks[CAMERA_READ_INTERCEPT_KEY] = new AOBBlock(CAMERA_READ_INTERCEPT_KEY, "F3 0F10 41 68 0F29 7C 24 60 F3 0F7E 79 60 44 0F29 44 24 50 0FC6 F8 04 44 0F10 41 50", 1);
		//aobBlocks[GAMESPEED_ADDRESS_INTERCEPT_KEY] = new AOBBlock(GAMESPEED_ADDRESS_INTERCEPT_KEY, "76 0A F3 0F11 35 910D0400", 1);
		aobBlocks[FOV_WRITE_INTERCEPT_KEY] = new AOBBlock(FOV_WRITE_INTERCEPT_KEY, "0F10 00 0F11 46 24 0F10 48 10 | 0F11 4E 34", 1);
		//aobBlocks[SUPERSAMPLING_ADDRESS_KEY] = new AOBBlock(SUPERSAMPLING_ADDRESS_KEY, "8B 0D ?? ?? ?? ?? F3 0F 10 05 | ?? ?? ?? ?? 85 C9", 1);
		aobBlocks[ANSEL_UNLOCK_KEY] = new AOBBlock(ANSEL_UNLOCK_KEY, "0FC6 E4 00 0F28 C4 0FC2 C1 01 0F50 C0 | 83 F8 0F", 1);
		
		aobBlocks[ANSEL_CAMERA_WRITE_INTERCEPT_KEY] = new AOBBlock(ANSEL_CAMERA_WRITE_INTERCEPT_KEY, "0F28 30 | 0F11 73 08 0F28 4E 30 0F 28 C1", 1);
		aobBlocks[ANSEL_CAMERA_ADDRESS_INTERCEPT_KEY] = new AOBBlock(ANSEL_CAMERA_ADDRESS_INTERCEPT_KEY, "0F10 6C 01 08 F3 0F7E 54 01 24", 1);
		//aobBlocks[ANSEL_FOV_WRITE_INTERCEPT_KEY] = new AOBBlock(ANSEL_FOV_WRITE_INTERCEPT_KEY, "F3 0F5F C1 F3 0F11 40 08 41 80 7C 24 19 00", 1);


		map<string, AOBBlock*>::iterator it;
		bool result = true;
		for(it = aobBlocks.begin(); it!=aobBlocks.end();it++)
		{
			result &= it->second->scan(hostImageAddress, hostImageSize);
		}
		if (result)
		{
			Console::WriteLine("All interception offsets found.");
		}
		else
		{
			Console::WriteError("One or more interception offsets weren't found: tools aren't compatible with this game's version.");
		}
	}


	void setCameraStructInterceptorHook(map<string, AOBBlock*> &aobBlocks)
	{
		GameImageHooker::setHook(aobBlocks[CAMERA_ADDRESS_INTERCEPT_KEY], 0x1B, &_cameraStructInterceptionContinue, &cameraAddressInterceptor);
		GameImageHooker::setHook(aobBlocks[FOV_WRITE_INTERCEPT_KEY], 0x1C, &_fovWriteInterceptionContinue, &fovWriteInterceptor);
	}
	

	void setCameraWriteInterceptorHooks(map<string, AOBBlock*> &aobBlocks)
	{
		// for each block of code that writes to the camera values we're manipulating we need an interception to block it. For the example game there are 3. 
		//GameImageHooker::setHook(aobBlocks[CAMERA_WRITE_INTERCEPT1_KEY], 0x1F, &_cameraWriteInterceptionContinue1, &cameraWriteInterceptor1);
		//GameImageHooker::setHook(aobBlocks[CAMERA_WRITE_INTERCEPT2_KEY], 0x10, &_cameraWriteInterceptionContinue2, &cameraWriteInterceptor2);
		//GameImageHooker::setHook(aobBlocks[CAMERA_WRITE_INTERCEPT3_KEY], 0x2E, &_cameraWriteInterceptionContinue3, &cameraWriteInterceptor3);
		//GameImageHooker::setHook(aobBlocks[CAMERA_READ_INTERCEPT_KEY], 0x1E, &_cameraReadInterceptionContinue1, &cameraReadInterceptor1);
		GameImageHooker::setHook(aobBlocks[ANSEL_UNLOCK_KEY], 0x21, &_anselUnlockContinue, &anselUnlock);
		GameImageHooker::setHook(aobBlocks[ANSEL_CAMERA_WRITE_INTERCEPT_KEY], 0x25, &_anselCameraWriteContinue, &anselCameraWriteInterceptor);
		GameImageHooker::setHook(aobBlocks[ANSEL_CAMERA_ADDRESS_INTERCEPT_KEY], 0x1A, &_anselCameraInterceptContinue, &anselCameraAddressInterceptor);
		GameImageHooker::setHook(aobBlocks[CAMERA_WRITE_INTERCEPT1_KEY], 0x1F, &_cameraWriteInterceptionContinue1, &cameraWriteInterceptor1);

		//Ansel FOV is out of process, find other way?
		//GameImageHooker::setHook(aobBlocks[ANSEL_FOV_WRITE_INTERCEPT_KEY], 0x0F, &_anselFovWriteContinue, &anselFovWriteInterceptor);
		//CameraManipulator::setResolutionScaleMenuValueAddress(Utils::calculateAbsoluteAddress(aobBlocks[SUPERSAMPLING_ADDRESS_KEY], 4));	//hitman.exe+E609E3 - F3 0F10 05 9D6C0C02   - movss xmm0,[hitman.exe+2F27688] << supersampling read here
	}


	void setTimestopInterceptorHook(map<string, AOBBlock*> &aobBlocks)
	{
		//GameImageHooker::setHook(aobBlocks[GAMESPEED_ADDRESS_INTERCEPT_KEY], 0x1C, &_gamespeedInterceptionContinue, &gamespeedAddressInterceptor);
	}
}
