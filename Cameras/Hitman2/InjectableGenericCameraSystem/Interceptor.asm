;////////////////////////////////////////////////////////////////////////////////////////////////////////
;// Part of Injectable Generic Camera System
;// Copyright(c) 2017, Frans Bouma
;// All rights reserved.
;// https://github.com/FransBouma/InjectableGenericCameraSystem
;//
;// Redistribution and use in source and binary forms, with or without
;// modification, are permitted provided that the following conditions are met :
;//
;//  * Redistributions of source code must retain the above copyright notice, this
;//	  list of conditions and the following disclaimer.
;//
;//  * Redistributions in binary form must reproduce the above copyright notice,
;//    this list of conditions and the following disclaimer in the documentation
;//    and / or other materials provided with the distribution.
;//
;// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
;// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
;// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
;// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
;// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
;// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
;// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;////////////////////////////////////////////////////////////////////////////////////////////////////////
;---------------------------------------------------------------
; Game specific asm file to intercept execution flow to obtain addresses, prevent writes etc.
;---------------------------------------------------------------

;---------------------------------------------------------------
; Public definitions so the linker knows which names are present in this file
PUBLIC cameraAddressInterceptor
PUBLIC cameraWriteInterceptor1
PUBLIC fovWriteInterceptor

PUBLIC anselUnlock
PUBLIC anselCameraWriteInterceptor
PUBLIC anselFovWriteInterceptor

;---------------------------------------------------------------

;---------------------------------------------------------------
; Externs which are used and set by the system. Read / write these
; values in asm to communicate with the system
EXTERN g_cameraStructAddress: qword
EXTERN g_gamespeedStructAddress: qword
EXTERN g_fovStructAddress: qword
EXTERN g_cameraEnabled: BYTE
EXTERN g_anselCameraEnabled: BYTE
EXTERN g_aimFrozen: BYTE
EXTERN g_originalQuaternion: qword
EXTERN g_originalCoords: qword
EXTERN g_anselCameraStructAddress: qword
EXTERN g_anselFovStructAddress: qword
EXTERN g_positionStructAddress: qword

;---------------------------------------------------------------

;---------------------------------------------------------------
; Own externs, defined in InterceptorHelper.cpp
EXTERN _cameraStructInterceptionContinue: qword
EXTERN _cameraWriteInterceptionContinue1: qword
EXTERN _cameraReadInterceptionContinue1: qword
EXTERN _gamespeedInterceptionContinue: qword
EXTERN _fovWriteInterceptionContinue: qword
EXTERN _anselUnlockContinue: qword
EXTERN _anselCameraWriteContinue: qword
EXTERN _anselFovWriteContinue: qword
EXTERN _anselCameraInterceptContinue: qword

; Scratch pad
;hitman.exe+E609DA - 0F28 CE               - movaps xmm1,xmm6
;hitman.exe+E609DD - 8B 0D 19EFD300        - mov ecx,[hitman.exe+1B9F8FC]
;hitman.exe+E609E3 - F3 0F10 05 9D6C0C02   - movss xmm0,[hitman.exe+2F27688] << supersampling read here
;hitman.exe+E609EB - 85 C9                 - test ecx,ecx
;hitman.exe+E609ED - 74 08                 - je hitman.exe+E609F7
;
;---------------------------------------------------------------
.code

cameraWriteInterceptor1 PROC
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A1492 - E8 C9412BFF           - call hitman2.Ordinal326+F5820
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A1497 - F3 0F10 4C 24 54      - movss xmm1,[rsp+54]
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A149D - 0F28 00               - movaps xmm0,[rax]
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14A0 - 0F11 43 50            - movups [rbx+50],xmm0 << QUAT WRITE << intercept
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14A4 - F3 0F10 44 24 50      - movss xmm0,[rsp+50]
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14AA - F3 0F11 43 60         - movss [rbx+60],xmm0 << X WRITE
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14AF - F3 0F10 44 24 58      - movss xmm0,[rsp+58]
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14B5 - F3 0F11 43 68         - movss [rbx+68],xmm0 << Y WRITE
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14BA - F3 0F11 4B 64         - movss [rbx+64],xmm1 << Z WRITE
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14BF - EB 2F                 - jmp hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14F0 <<continue
;hitman2.AK::AkRuntimeEnvironmentMgr::Instance+A14C1 - 48 8D 4C 24 60        - lea rcx,[rsp+60]
;hitman2.CAkMixer::MixChannelSIMD+13D436 - E8 55452BFF           - call hitman2.Ordinal326+F6300

	; Game jmps to this location due to the hook set in C function SetCameraStructInterceptorHook
	cmp g_cameraStructAddress, rbx						; make sure this is the camera address
	jne originalCode									; if not the camera, execute original code
	cmp BYTE ptr [g_cameraEnabled], 1					; check if the user enabled the camera. If so, just skip the write statements, otherwise just execute the original code.
	je exit												; our own camera is enabled, just skip the writes
originalCode:
	movups [rbx+50h],xmm0								; origial quat write
	movss xmm0, dword ptr[rsp+50h]
	movss dword ptr[rbx+60h],xmm0						; original X write
	movss xmm0, dword ptr[rsp+58h]
	movss dword ptr[rbx+68h],xmm0						; original Z write
	movss dword ptr[rbx+64h],xmm1						; original Y write
exit:
	jmp qword ptr [_cameraWriteInterceptionContinue1]	; jmp back into the original game code, which is the location after the original statements above.
cameraWriteInterceptor1 ENDP


cameraAddressInterceptor PROC
;hitman2.AK::MemoryMgr::SetPoolName+9E8A8D - 8B 83 80040000        - mov eax,[rbx+00000480] << intercept
;hitman2.AK::MemoryMgr::SetPoolName+9E8A93 - 48 8B 4B 10           - mov rcx,[rbx+10]
;hitman2.AK::MemoryMgr::SetPoolName+9E8A97 - F3 0F10 4C 24 20      - movss xmm1,[rsp+20]
;hitman2.AK::MemoryMgr::SetPoolName+9E8A9D - 89 83 00020000        - mov [rbx+00000200],eax << intercept
;hitman2.AK::MemoryMgr::SetPoolName+9E8AA3 - 8B 83 D4030000        - mov eax,[rbx+000003D4]
;hitman2.AK::MemoryMgr::SetPoolName+9E8AA9 - 89 43 2C              - mov [rbx+2C],eax
;hitman2.AK::MemoryMgr::SetPoolName+9E8AAC - 8B 83 D0030000        - mov eax,[rbx+000003D0]
;hitman2.AK::MemoryMgr::SetPoolName+9E8AB2 - 89 43 30              - mov [rbx+30],eax 
;hitman2.AK::MemoryMgr::SetPoolName+9E8AB5 - 48 8B 01              - mov rax,[rcx] << rcx is camera address struct
;hitman2.AK::MemoryMgr::SetPoolName+9E8AB8 - FF 90 88010000        - call qword ptr [rax+00000188] << return
;hitman2.AK::MemoryMgr::SetPoolName+9E8ABE - 48 8B 4B 10           - mov rcx,[rbx+10]
;hitman2.AK::MemoryMgr::SetPoolName+9E8AC2 - 48 8D 54 24 30        - lea rdx,[rsp+30]
;<<return
	mov [rbx+00000200h],eax
	mov eax,[rbx+000003D4h]
	mov [rbx+2Ch],eax
	mov eax,[rbx+000003D0h]
	mov [rbx+30h],eax
	mov rax, [rcx]
	mov [g_cameraStructAddress],rcx					; get camera struct address
exit:
	jmp qword ptr [_cameraStructInterceptionContinue]
cameraAddressInterceptor ENDP

gamespeedAddressInterceptor PROC
;TrSpeedHack_x64.dll+2D79 - 76 0A                 - jna TrSpeedHack_x64.dll+2D85 << interrupt here?
;TrSpeedHack_x64.dll+2D7B - F3 0F11 35 910D0400   - movss [TrSpeedHack_x64.dll+43B14],xmm6 { [1.00] } << writes speed value to memory
;TrSpeedHack_x64.dll+2D83 - EB 10                 - jmp TrSpeedHack_x64.dll+2D95
;TrSpeedHack_x64.dll+2D85 - F3 0F10 05 03770300   - movss xmm0,[TrSpeedHack_x64.dll+3A490] { [1.00] } << always 1? default speed
;TrSpeedHack_x64.dll+2D8D - F3 0F11 05 7F0D0400   - movss [TrSpeedHack_x64.dll+43B14],xmm0 { [1.00] } << restores default speed value in mem
;TrSpeedHack_x64.dll+2D95 - 48 8D 0D C43A0400     - lea rcx,[TrSpeedHack_x64.dll+46860] { [4AB5D5E0] } <<resume here
	mov rbx, 7FFF44E93B14h
	mov [g_gamespeedStructAddress], rbx
	jmp qword ptr [_gamespeedInterceptionContinue]
gamespeedAddressInterceptor ENDP



cameraReadInterceptor1 PROC
; v1.13.2

;hitman2.AK::WriteBytesMem::Bytes+A1EE8 - 48 8B 81 80000000     - mov rax,[rcx+00000080]
;hitman2.AK::WriteBytesMem::Bytes+A1EEF - F3 0F10 41 68         - movss xmm0,[rcx+68] <<intercept, height read
;hitman2.AK::WriteBytesMem::Bytes+A1EF4 - 0F29 7C 24 60         - movaps [rsp+60],xmm7
;hitman2.AK::WriteBytesMem::Bytes+A1EF9 - F3 0F7E 79 60         - movq xmm7,[rcx+60] <<position read
;hitman2.AK::WriteBytesMem::Bytes+A1EFE - 44 0F29 44 24 50      - movaps [rsp+50],xmm8
;hitman2.AK::WriteBytesMem::Bytes+A1F04 - 0FC6 F8 04            - shufps xmm7,xmm0,04 { 4 }
;hitman2.AK::WriteBytesMem::Bytes+A1F08 - 44 0F10 41 50         - movups xmm8,[rcx+50] <<quat read
; << continue

	cmp BYTE ptr [g_cameraEnabled], 1					
	jne originalCode
	cmp BYTE ptr [g_aimFrozen], 1
	jne originalCode
	; aim is frozen and camera is enabled. We now issue a 'ret', which means it will return to the caller of the original code as we jmp-ed to this location
	; from there. This means we won't continue in the original code. 
	
	movss xmm0, dword ptr[g_originalCoords+8h]
	movaps [rsp+60h],xmm7
	movq xmm7, qword ptr[g_originalCoords]
	movaps [rsp+50h],xmm8
	shufps xmm7,xmm0,04h
	movups xmm8, dword ptr[g_originalQuaternion]
	jmp exit

originalCode:
	movss xmm0, dword ptr[rcx+68h]
	movaps [rsp+60h],xmm7
	movq xmm7, qword ptr[rcx+60h]
	movaps [rsp+50h],xmm8
	shufps xmm7,xmm0,04h
	movups xmm8, [rcx+50h]
exit:
	jmp qword ptr [_cameraReadInterceptionContinue1]	; jmp back into the original game code which is the location after the original statements above.
cameraReadInterceptor1 ENDP


fovWriteInterceptor PROC
; FOV write:  (1.13.2)
;hitman2.AK::WriteBytesMem::Clear+1B0A482 - 0F11 46 24            - movups [rsi+24],xmm0
;hitman2.AK::WriteBytesMem::Clear+1B0A486 - 0F10 48 10            - movups xmm1,[rax+10]
;hitman2.AK::WriteBytesMem::Clear+1B0A48A - 0F11 4E 34            - movups [rsi+34],xmm1 << intercept << FOV WRITE
;hitman2.AK::WriteBytesMem::Clear+1B0A48E - 0F10 40 20            - movups xmm0,[rax+20]
;hitman2.AK::WriteBytesMem::Clear+1B0A492 - 0F11 46 44            - movups [rsi+44],xmm0
;hitman2.AK::WriteBytesMem::Clear+1B0A496 - 0F10 48 30            - movups xmm1,[rax+30]
;hitman2.AK::WriteBytesMem::Clear+1B0A49A - 0F11 4E 54            - movups [rsi+54],xmm1
;hitman2.AK::WriteBytesMem::Clear+1B0A49E - 0F10 40 40            - movups xmm0,[rax+40]
;hitman2.AK::WriteBytesMem::Clear+1B0A4A2 - 0F11 46 64            - movups [rsi+64],xmm0 
;hitman2.AK::WriteBytesMem::Clear+1B0A4A6 - 0F10 48 50            - movups xmm1,[rax+50] << continue
;hitman2.AK::WriteBytesMem::Clear+1B0A4AA - 0F11 4E 74            - movups [rsi+74],xmm1

	mov [g_fovStructAddress], rsi					; intercept address of fov struct
	cmp BYTE ptr [g_cameraEnabled], 1					
	je exit
originalCode:
	movups [rsi+34h],xmm1
exit:
	movups xmm0,[rax+20h]
	movups [rsi+44h],xmm0
	movups xmm1,[rax+30h]
	movups [rsi+54h],xmm1
	movups xmm0,[rax+40h]
	movups [rsi+64h],xmm0
	jmp qword ptr [_fovWriteInterceptionContinue]	; jmp back into the original game code which is the location after the original statements above.
fovWriteInterceptor ENDP

anselUnlock PROC

;hitman2.AK::MemoryMgr::SetPoolName+7DB078 - 0FC6 E4 00            - shufps xmm4,xmm4,00 { 0 }
;hitman2.AK::MemoryMgr::SetPoolName+7DB07C - 0F28 C4               - movaps xmm0,xmm4
;hitman2.AK::MemoryMgr::SetPoolName+7DB07F - 0FC2 C1 01            - cmpps xmm0,xmm1,01 { 1 }
;hitman2.AK::MemoryMgr::SetPoolName+7DB083 - 0F50 C0               - movmskps eax,xmm0
;hitman2.AK::MemoryMgr::SetPoolName+7DB086 - 83 F8 0F              - cmp eax,0F { 15 } << intercept
;hitman2.AK::MemoryMgr::SetPoolName+7DB089 - 75 1C                 - jne hitman2.AK::MemoryMgr::SetPoolName+7DB0A7
;hitman2.AK::MemoryMgr::SetPoolName+7DB08B - 0F54 D6               - andps xmm2,xmm6
;hitman2.AK::MemoryMgr::SetPoolName+7DB08E - F2 0F                 -  
;hitman2.AK::MemoryMgr::SetPoolName+7DB090 - 7C D2                 - jl hitman2.AK::MemoryMgr::SetPoolName+7DB064
;hitman2.AK::MemoryMgr::SetPoolName+7DB092 - F2 0F                 -  
;hitman2.AK::MemoryMgr::SetPoolName+7DB094 - 7C D2                 - jl hitman2.AK::MemoryMgr::SetPoolName+7DB068
;hitman2.AK::MemoryMgr::SetPoolName+7DB096 - 0F52 C2               - rsqrtps xmm0,xmm2
;hitman2.AK::MemoryMgr::SetPoolName+7DB099 - 0F59 C3               - mulps xmm0,xmm3
;hitman2.AK::MemoryMgr::SetPoolName+7DB09C - 0F59 C4               - mulps xmm0,xmm4
;hitman2.AK::MemoryMgr::SetPoolName+7DB09F - 0F58 C5               - addps xmm0,xmm5
;hitman2.AK::MemoryMgr::SetPoolName+7DB0A2 - 0F29 44 24 20         - movaps [rsp+20],xmm0
;hitman2.AK::MemoryMgr::SetPoolName+7DB0A7 - 4C 8D 87 90000000     - lea r8,[rdi+00000090] << continue

	;do nothing, nothing needs to be done
	jmp qword ptr [_anselUnlockContinue]
anselUnlock ENDP

anselCameraAddressInterceptor PROC
;hitman2.AK::WriteBytesMem::Bytes+1A9B0E - 0F10 6C 01 08         - movups xmm5,[rcx+rax+08] <<  rcx + rax = ansel cam struct? << intercept
;hitman2.AK::WriteBytesMem::Bytes+1A9B13 - F3 0F7E 54 01 24      - movq xmm2,[rcx+rax+24]
;hitman2.AK::WriteBytesMem::Bytes+1A9B19 - 66 0F61 D1            - punpcklwd xmm2,xmm1
;hitman2.AK::WriteBytesMem::Bytes+1A9B1D - 0F28 DD               - movaps xmm3,xmm5
;hitman2.AK::WriteBytesMem::Bytes+1A9B20 - 66 0F6F C2            - movdqa xmm0,xmm2
;hitman2.AK::WriteBytesMem::Bytes+1A9B24 - 0FC6 DD 95            - shufps xmm3,xmm5,-6B { 149 }
; 26 bytes
	mov r10, rcx
	add r10, rax
	mov [g_anselCameraStructAddress], r10
	movups xmm5,[rcx+rax+08h]
	movq xmm2, qword ptr[rcx+rax+24h]
	punpcklwd xmm2,xmm1
	movaps xmm3,xmm5
	movdqa xmm0,xmm2
	shufps xmm3,xmm5,-6Bh
exit:
	jmp qword ptr [_anselCameraInterceptContinue]
anselCameraAddressInterceptor ENDP


anselCameraWriteInterceptor PROC
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCA9 - F3 44 0F11 B5 A0010000  - movss [rbp+000001A0],xmm14
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCB2 - 0F28 30               - movaps xmm6,[rax]
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCB5 - 0F11 73 08            - movups [rbx+08],xmm6 << ansel writes quaternion << intercept
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCB9 - 0F28 4E 30            - movaps xmm1,[rsi+30]
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCBD - 0F28 C1               - movaps xmm0,xmm1
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCC0 - F3 0F11 4B 18         - movss [rbx+18],xmm1 << ansel writes X
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCC5 - 0FC6 C1 55            - shufps xmm0,xmm1,55 { 85 }
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCC9 - 0FC6 C9 AA            - shufps xmm1,xmm1,-56 { 170 }
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCCD - F3 0F11 4B 20         - movss [rbx+20],xmm1 << ansel writes Y
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCD2 - F3 0F11 43 1C         - movss [rbx+1C],xmm0 << ansel writes Z
;hitman2.AK::MemoryMgr::SetMonitoring+6ADCD7 - F3 0F10 8D A0010000   - movss xmm1,[rbp+000001A0] <<continue
;34 bytes
	;mov [g_anselCameraStructAddress], rbx
	cmp g_anselCameraStructAddress, rbx
	jne originalCode
	cmp BYTE ptr [g_cameraEnabled], 1
	jne originalCode
	cmp BYTE ptr [g_anselCameraEnabled], 1
	je exit
originalCode:
	movups [rbx+08h],xmm6
	movaps xmm1,[rsi+30h]
	movaps xmm0,xmm1
	movss dword ptr[rbx+18h],xmm1
	shufps xmm0,xmm1,55h
	shufps xmm1,xmm1,-56h
	movss dword ptr[rbx+20h],xmm1
	movss dword ptr[rbx+1Ch],xmm0
exit:
	jmp qword ptr [_anselCameraWriteContinue]
anselCameraWriteInterceptor ENDP

anselFovWriteInterceptor PROC
;NvCamera64.AnselGetFunctionTable+7642 - F3 0F10 05 AE844400   - movss xmm0,[NvCamera64.OnInstall+375AD8] { [1.00] }
;NvCamera64.AnselGetFunctionTable+764A - F3 0F5F C1            - maxss xmm0,xmm1 << intercept
;NvCamera64.AnselGetFunctionTable+764E - F3 0F11 40 08         - movss [rax+08],xmm0 << ansel writes FOV
;NvCamera64.AnselGetFunctionTable+7653 - 41 80 7C 24 19 00     - cmp byte ptr [r12+19],00 { 0 }
;NvCamera64.AnselGetFunctionTable+7659 - 74 1C                 - je NvCamera64.AnselGetFunctionTable+7677 << continue
;15 bytes
	maxss xmm0,xmm1
	mov [g_anselFovStructAddress], rax
	cmp BYTE ptr [g_cameraEnabled], 1
	je exit
originalCode:
	movss dword ptr[rax+08h],xmm0
exit:
	cmp byte ptr [r12+19h],00
	jmp qword ptr [_anselFovWriteContinue]
anselFovWriteInterceptor ENDP

END