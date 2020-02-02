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
#pragma once

namespace IGCS::GameSpecific
{
	// Mandatory constants to define for a game
	#define GAME_NAME									"Hitman 2 v1.13.2"
	#define CAMERA_VERSION								"1.0.1"
	#define CAMERA_CREDITS								"WEDS_GARBAGEDAY, Otis_Inf, Jim2Point0. Special thanks to: One3rd"
	#define GAME_WINDOW_TITLE							"HITMAN 2"
	#define INITIAL_PITCH_RADIANS						(-90.0f * XM_PI) / 180.f	// World has Z up and Y out of the screen, so rotate around X (pitch) -90 degrees.
	#define INITIAL_YAW_RADIANS							0.0f
	#define INITIAL_ROLL_RADIANS						0.0f
	#define CONTROLLER_Y_INVERT							true
	#define FASTER_MULTIPLIER							6.0f
	#define SLOWER_MULTIPLIER							0.05f
	#define DEFAULT_CAMERA_MULTIPLIER					1.0f
	#define MOUSE_SPEED_CORRECTION						0.2f	// to correct for the mouse-deltas related to normal rotation.
	#define DEFAULT_SUPERSAMPLING_SPEED					0.05f
	#define DEFAULT_MOVEMENT_SPEED						0.03f
	#define DEFAULT_ROTATION_SPEED						0.01f
	#define DEFAULT_FOV_DEGREES							40.0f
	#define DEFAULT_FOV_SPEED							0.1f
	#define DEFAULT_Z_MOVEMENT_MULTIPLIER				0.5f
	#define DEFAULT_MIN_GAME_SPEED						0.1f
	#define DEFAULT_MAX_GAME_SPEED						10.0f
	// End Mandatory constants

	// AOB Keys for interceptor's AOB scanner
	#define CAMERA_ADDRESS_INTERCEPT_KEY				"AOB_CAMERA_ADDRESS_INTERCEPT"
	#define CAMERA_WRITE_INTERCEPT1_KEY					"AOB_CAMERA_WRITE_INTERCEPT1"
	#define CAMERA_WRITE_INTERCEPT2_KEY					"AOB_CAMERA_WRITE_INTERCEPT2"
	#define CAMERA_WRITE_INTERCEPT3_KEY					"AOB_CAMERA_WRITE_INTERCEPT3"
	#define CAMERA_READ_INTERCEPT_KEY					"AOB_CAMERA_READ_INTERCEPT3"
	#define GAMESPEED_ADDRESS_INTERCEPT_KEY				"AOB_GAMESPEED_ADDRESS_INTERCEPT"
	#define FOV_WRITE_INTERCEPT_KEY						"AOB_FOV_WRITE_INTERCEPT"
	#define FOV_ADDRESS_INTERCEPT_KEY					"AOB_FOV_ADDRESS_INTERCEPT"
	#define SUPERSAMPLING_ADDRESS_KEY					"AOB_SUPERSAMPLING_ADDRESS"
	#define SUPERSAMPLING_ADDRESS						0x1433B7D08 //supersampling address is static and only written to when value is changed in menu
	#define ANSEL_UNLOCK_KEY							"AOB_ANSEL_UNLOCK"
	#define ANSEL_CAMERA_WRITE_INTERCEPT_KEY			"AOB_ANSEL_CAMERA_WRITE_INTERCEPT"
	#define ANSEL_CAMERA_ADDRESS_INTERCEPT_KEY			"AOB_ANSEL_CAMERA_ADDRESS_INTERCEPT"
	#define ANSEL_FOV_WRITE_INTERCEPT_KEY				"AOB_ANSEL_FOV_WRITE_INTERCEPT"
	#define POSITION_WRITE_INTERCEPT1_KEY				"AOB_POSITION_WRITE_INTERCEPT1"
	#define POSITION_WRITE_INTERCEPT2_KEY				"AOB_POSITION_WRITE_INTERCEPT2"

	// Indices in the structures read by interceptors 
	#define LOOK_QUATERNION_IN_CAMERA_STRUCT_OFFSET		0x50		
	#define CAMERA_COORDS_IN_CAMERA_STRUCT_OFFSET		0x60		
	#define FOV_IN_FOV_STRUCT_OFFSET					0x38	
	#define GAMESPEED_IN_STRUCT_OFFSET					0x0	
	#define ANSEL_CAMERA_COORDS_IN_CAMERA_STRUCT_OFFSET			0x18
	#define ANSEL_LOOK_QUATERNION_IN_CAMERA_STRUCT_OFFSET		0x08
	#define ANSEL_FOV_IN_FOV_STRUCT_OFFSET						0x08
	#define POSITION_COORDS_IN_STRUCT_OFFSET			0x60

	// specific option.
	#define IGCS_KEY_FREEZE_47							VK_END	
}