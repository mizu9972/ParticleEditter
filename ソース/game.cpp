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
#include "dx11mathutil.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "CXFileParticleSystem.h"
//�p�[�e�B�N���Ŏg�p����e�N�X�`���̖��O
constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/DefaultParticle.png";
//-----------------------------------------------------------------------------
// �O���[�o���ϐ�
//-----------------------------------------------------------------------------

CModel				g_skydome;
XMFLOAT4X4			g_matskydome;

ParticleSystem g_NomalParticleSystem;
XFileParticleSystem g_XfileParticleSystem;
t_ParticleSystemState g_setParticleState;
t_ParticleSystemState g_ParticleState4ImGui;

extern int g_nCountFPS;//FPS

//-----------------------------------------------------------------------------
// �v���g�^�C�v�錾
//-----------------------------------------------------------------------------
void ImGuiDraw();
bool isPartilceStateEqual();
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

	//�p�[�e�B�N���V�X�e���ݒ�--------------------
	const char* filename_(PARTICLE_TEXTURE);

	g_setParticleState.m_PositionX = 0;
	g_setParticleState.m_PositionY = 0;
	g_setParticleState.m_PositionZ = 0;

	g_setParticleState.m_AngleX = 0;
	g_setParticleState.m_AngleY = 0;
	g_setParticleState.m_AngleZ = 0;

	g_setParticleState.m_AngleRangeX = 360;
	g_setParticleState.m_AngleRangeY = 360;
	g_setParticleState.m_AngleRangeZ = 360;

	g_setParticleState.m_DuaringTime = 10;

	g_setParticleState.m_MaxLifeTime = 5;
	g_setParticleState.m_ParticleNum = 100;
	g_setParticleState.m_Size = 20;
	g_setParticleState.m_Speed = 30;

	XMFLOAT4 setColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	memcpy(&g_setParticleState.m_Color, &setColor, sizeof(XMFLOAT4));
	g_setParticleState.isLooping = true;
	//----------------------------------------

	//ImGui�p�ɕۑ�
	memcpy(&g_ParticleState4ImGui, &g_setParticleState, sizeof(t_ParticleSystemState));

	//g_XfileParticleSystem.ModelInit("assets/f1.x.dat", "shader/vs.fx", "shader/ps.fx");
	//g_XfileParticleSystem.Init(g_setParticleState);

	g_NomalParticleSystem.Init(g_setParticleState, filename_);
	// �X�J�C�h�[���ǂݍ���
	g_skydome.Init("assets/skydome.x.dat","shader/vs.fx","shader/psskydome.fx");

	DX11MtxIdentity(g_matskydome);


	// �J�����ϊ��s�񏉊���
	// �v���W�F�N�V�����ϊ��s�񏉊���
	XMFLOAT3 eye =	  {0,0,-50};				// ���_
	XMFLOAT3 lookat = { 0,0,0 };			// �����_
	XMFLOAT3 up = {0,1,0};					// ������x�N�g��

	CCamera::GetInstance()->Init(1.0f, 10000.0f, XM_PIDIV2, SCREEN_X, SCREEN_Y, eye, lookat, up);

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
	g_NomalParticleSystem.Update();
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

	// ���[���h�ϊ��s��
	DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::WORLD, g_matskydome);
	g_skydome.Draw();

	CDirectXGraphics::GetInstance()->TurnOnAlphaBlending();//�A���t�@�u�����h��L����
	g_NomalParticleSystem.Draw();

	bool show_demo_window = true;
	bool show_another_window = false;

	ImGuiDraw();
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
	g_NomalParticleSystem.UnInit();
	DX11Uninit();
}

//==============================================================================
//!	@fn		ImGuiDraw
//!	@brief	�Q�[���I������
//!	@param�@	
//!	@retval	�Ȃ�
//==============================================================================
void ImGuiDraw() {
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	static int counter = 0;

	const char* filename_(PARTICLE_TEXTURE);
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).

	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiSetCond_Once);

	ImGui::Begin("ParticleSystem Settings");
	std::string str;

	ImGui::Text("FPS:%d", g_nCountFPS);

	if (ImGui::TreeNode("Position")) {
		ImGui::SliderFloat("X", &g_ParticleState4ImGui.m_PositionX, -1000.0f, 1000.0f);
		ImGui::SliderFloat("Y", &g_ParticleState4ImGui.m_PositionY, -1000.0f, 1000.0f);
		ImGui::SliderFloat("Z", &g_ParticleState4ImGui.m_PositionZ, -1000.0f, 1000.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Angle")) {
		ImGui::SliderInt("X", &g_ParticleState4ImGui.m_AngleX, 1, 360);
		ImGui::SliderInt("Y", &g_ParticleState4ImGui.m_AngleY, 1, 360);
		ImGui::SliderInt("Z", &g_ParticleState4ImGui.m_AngleZ, 1, 360);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("AngleRange")) {
		ImGui::SliderInt("X", &g_ParticleState4ImGui.m_AngleRangeX, 1, 360);
		ImGui::SliderInt("Y", &g_ParticleState4ImGui.m_AngleRangeY, 1, 360);
		ImGui::SliderInt("Z", &g_ParticleState4ImGui.m_AngleRangeZ, 1, 360);
		ImGui::TreePop();

	}
	ImGui::SliderFloat("Duaring", &g_ParticleState4ImGui.m_DuaringTime, 0, 1000);


	ImGui::SliderInt("ParticleNum", &g_ParticleState4ImGui.m_ParticleNum, 0, 1000);
	ImGui::SliderFloat("MaxLifeTime", &g_ParticleState4ImGui.m_MaxLifeTime, 0.0f, 1000.0f);
	ImGui::SliderFloat("Size", &g_ParticleState4ImGui.m_Size, 0.0f, 1000.0f);
	ImGui::SliderFloat("Speed", &g_ParticleState4ImGui.m_Speed, 0.0f, 1000.0f);
	ImGui::Checkbox("isLooping", &g_ParticleState4ImGui.isLooping);


	ImGui::ColorEdit4("Color", g_ParticleState4ImGui.m_Color, 0);

	if (isPartilceStateEqual() == false) {
		memcpy(&g_setParticleState, &g_ParticleState4ImGui, sizeof(t_ParticleSystemState));
		g_NomalParticleSystem.Init(g_setParticleState, filename_);
	}


	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	g_NomalParticleSystem.SetParticleSystemState(g_ParticleState4ImGui);
	//�t�@�C���A�N�Z�X�֘A

	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.7f, 0.0f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.3f, 0.0f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 220, 20), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 500), ImGuiSetCond_Once);


	ImGui::Begin("ParticleFile");

	//�t�@�C�������o��
	if (ImGui::Button("Export")) {
		g_NomalParticleSystem.FOutState();
	}

	//�t�@�C���ǂݍ���
	WIN32_FIND_DATA win32fd;

	std::string DirectoryName = ".\\InPutData\\*.txt";//�ǂݍ��ރt�H���_�̃f�B���N�g���Ɗg���q�w��
	HANDLE hFind;//�t�@�C������n���h��

	if (ImGui::TreeNode("Import")) {//Imgui�̃c���[���J���ꂽ��
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//�t�@�C�������݂��邩�m�F
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {//���������t�@�C�����{�^���Ƃ��ĕ\��
					//�{�^���������ꂽ�炻�̃t�@�C����ǂݍ��ݔ��f
					g_NomalParticleSystem.FInState(win32fd.cFileName);
				}
			} while (FindNextFile(hFind, &win32fd));//���̃t�@�C����{��
		}
		ImGui::TreePop();
		FindClose(hFind);

	}

	//�e�N�X�`���ǂݍ���
	DirectoryName = ".\\InPutData\\*.png";
	if (ImGui::TreeNode("inputTexture")) {//Imgui�̃c���[���J���ꂽ��
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//�t�@�C�������݂��邩�m�F
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {//���������t�@�C�����{�^���Ƃ��ĕ\��
					//�{�^���������ꂽ�炻�̃t�@�C����ǂݍ��ݔ��f
					g_NomalParticleSystem.FInTex(win32fd.cFileName);
				}
			} while (FindNextFile(hFind, &win32fd));//���̃t�@�C����{��
		}
		ImGui::TreePop();
		FindClose(hFind);

	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

//==============================================================================
//!	@fn		ImGuiReflection
//!	@brief	�Q�[���I������
//!	@param�@	
//!	@retval	�Ȃ�
//==============================================================================
bool isPartilceStateEqual() {
	double sumState1, sumState2;


	//�v�C��
	//�S���̃����o�̐��l�����Z���Ēl���r
	sumState1 = g_setParticleState.m_PositionX + g_setParticleState.m_PositionY + g_setParticleState.m_PositionZ
		+ g_setParticleState.m_AngleRangeX + g_setParticleState.m_AngleRangeY + g_setParticleState.m_AngleRangeZ
		+ g_setParticleState.m_Color[0] + g_setParticleState.m_Color[1] + g_setParticleState.m_Color[2] + g_setParticleState.m_Color[3]
		+ g_setParticleState.m_ParticleNum + g_setParticleState.m_MaxLifeTime + g_setParticleState.m_DuaringTime
		+ g_setParticleState.m_Size + g_setParticleState.m_Speed + g_setParticleState.isLooping
		+ g_setParticleState.m_AngleX + g_setParticleState.m_AngleY + g_setParticleState.m_AngleZ;

	sumState2 = g_ParticleState4ImGui.m_PositionX + g_ParticleState4ImGui.m_PositionY + g_ParticleState4ImGui.m_PositionZ
		+ g_ParticleState4ImGui.m_AngleRangeX + g_ParticleState4ImGui.m_AngleRangeY + g_ParticleState4ImGui.m_AngleRangeZ
		+ g_ParticleState4ImGui.m_Color[0] + g_ParticleState4ImGui.m_Color[1] + g_ParticleState4ImGui.m_Color[2] + g_ParticleState4ImGui.m_Color[3]
		+ g_ParticleState4ImGui.m_ParticleNum + g_ParticleState4ImGui.m_MaxLifeTime + g_ParticleState4ImGui.m_DuaringTime
		+ g_ParticleState4ImGui.m_Size + g_ParticleState4ImGui.m_Speed + g_ParticleState4ImGui.isLooping
		+ g_ParticleState4ImGui.m_AngleX + g_ParticleState4ImGui.m_AngleY + g_ParticleState4ImGui.m_AngleZ;

	if (sumState1 == sumState2) {
		return true;
	}
	return false;
}
//******************************************************************************
//	End of file.
//******************************************************************************
