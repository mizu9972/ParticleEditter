//*****************************************************************************
//!	@file	game.cpp
//!	@brief	
//!	@note	�Q�[������
//!	@author	
//*****************************************************************************

//-----------------------------------------------------------------------------
//	Include header files.
//-----------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <Crtdbg.h>
#include <DirectXMath.h>
#include <random>
#include "DX11util.h"
#include "CModel.h"
#include "game.h"
#include "CCamera.h"
#include "CDirectInput.h"
#include "DX11Settransform.h"
#include "Lib/ParticleMathUtil.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "CParticleEdit.h"
//�p�[�e�B�N���Ŏg�p����e�N�X�`���̖��O
constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";
//-----------------------------------------------------------------------------
// �O���[�o���ϐ�
//-----------------------------------------------------------------------------
extern int g_nCountFPS;//FPS

//-----------------------------------------------------------------------------
// �v���g�^�C�v�錾
//-----------------------------------------------------------------------------

//==============================================================================
//!	@fn		GameInit
//!	@brief	�Q�[����������
//!	@param	�C���X�^���X�l
//!	@param	�E�C���h�E�n���h���l
//!	@param	�E�C���h�E�T�C�Y���i�␳�ς݂̒l�j
//!	@param	�E�C���h�E�T�C�Y�����i�␳�ς݂̒l�j
//!	@param�@	�t���X�N���[���t���O�@true ; �t���X�N���[���@false : �E�C���h�E
//!	@retval	true �����@false ���s
//==============================================================================
bool GameInit(HINSTANCE hinst, HWND hwnd, int width, int height, bool fullscreen)
{
	bool		sts;

	// DX11��������
	sts = DX11Init(hwnd,width,height,fullscreen);
	if (!sts) {
		MessageBox(hwnd, "DX11 init error", "error", MB_OK);
		return false;
	}

	// im gui ������
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(GetDX11Device(), GetDX11DeviceContext());

	ImGui::StyleColorsClassic();

	// DIRECTINPUT������
	CDirectInput::GetInstance().Init(hinst, hwnd, width, height);

	ParticleEditor::getInstance()->Init(width, height, CDirectXGraphics::GetInstance()->GetDXDevice(),CDirectXGraphics::GetInstance()->GetImmediateContext());//�G�f�B�^�[������

	// �J�����ϊ��s�񏉊���
	// �v���W�F�N�V�����ϊ��s�񏉊���
	XMFLOAT3 eye =	  {0,0,50};				// ���_
	XMFLOAT3 lookat = { 0,0,0 };			// �����_
	XMFLOAT3 up = {0,1,0};					// ������x�N�g��

	CCamera::GetInstance()->Init(1.0f, 10000.0f, XM_PIDIV2, SCREEN_X, SCREEN_Y, eye, lookat, up);

	CCamera::GetInstance()->SetRadius(50.0f);
	// ���s����������
	DX11LightInit(DirectX::XMFLOAT4(1,1,-1,0));		// ���s�����̕������Z�b�g

	return	true;
}

//==============================================================================
//!	@fn		GameInput
//!	@brief	�Q�[�����͌��m����
//!	@param�@	
//!	@retval	�Ȃ�
//==============================================================================
void GameInput(){
	CDirectInput::GetInstance().GetKeyBuffer();			// �L�[���͎擾
}

//==============================================================================
//!	@fn		GameUpdate
//!	@brief	�Q�[���X�V����
//!	@param�@	
//!	@retval	�Ȃ�
//==============================================================================
void GameUpdate(){
	ParticleEditor::getInstance()->Update();
	CCamera::GetInstance()->UpdateSphereCamera();
}

//==============================================================================
//!	@fn		GameRender
//!	@brief	�Q�[���X�V����
//!	@param�@	
//!	@retval	�Ȃ�
//==============================================================================
void GameRender() {
	// �^�[�Q�b�g�o�b�t�@�N���A	
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; //red,green,blue,alpha

	DirectX::XMFLOAT4X4 mat;

	// �����_�����O�O����
	DX11BeforeRender(ClearColor);

	// �r���[�ϊ��s��Z�b�g
	mat = CCamera::GetInstance()->GetCameraMatrix();
	DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::VIEW, mat);

	// �v���W�F�N�V�����ϊ��s��Z�b�g
	mat = CCamera::GetInstance()->GetProjectionMatrix();
	DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::PROJECTION, mat);

	ParticleEditor::getInstance()->Draw();

	bool show_demo_window = true;
	bool show_another_window = false;

	// �����_�����O�㏈��
	DX11AfterRender();
}

//==============================================================================
//!	@fn		GameMain
//!	@brief	�Q�[�����[�v����
//!	@param�@	
//!	@retval	�Ȃ�
//==============================================================================
void GameMain()
{
	GameInput();					// ����
	GameUpdate();					// �X�V
	GameRender();					// �`��
}

//==============================================================================
//!	@fn		GameExit
//!	@brief	�Q�[���I������
//!	@param�@	
//!	@retval	�Ȃ�
//==============================================================================
void GameExit()
{
	ImGui::DestroyContext();
	ParticleEditor::getInstance()->DeleteParticleSystems();
	ParticleEditor::getInstance()->UnInit();
	DX11Uninit();
}

//******************************************************************************
//	End of file.
//******************************************************************************
