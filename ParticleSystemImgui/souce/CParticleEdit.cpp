#include <list>
#include <vector>
#include <string>
#include "CParticleEdit.h"
#include "CCamera.h"

#include "game.h"

#define CHECK(x) (CheckDataChange += x) //���l�ύX�����m
#define CHECK_RESULT (CheckDataChange > 0) //�ύX���ꂽ���ǂ�������
static int CheckDataChange = 0;//ImGui�Ő��l���ύX���ꂽ��true���Ԃ��Ă��郁�\�b�h�̎d�l�𗘗p���āA���l�ύX���Ǘ�����ϐ�

extern int g_nCountFPS;

//������
void ParticleEditor::Init() {
	
	//�z�[�~���O�^�[�Q�b�g������
	m_TargetBillBoard.Init(
		m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2],
		30, 30,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		"shader/psParticle.fx",
		"shader/vsPrticle.fx"
	);
	strcpy_s(m_FileName, sizeof(m_FileName), "ParticleState");

	m_ParticleSystems.Init();

	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };

	m_TargetBillBoard.SetUV(u, v);
	m_TargetBillBoard.LoadTexTure("assets/ParticleTexture/en.png");
}

//�I������
void ParticleEditor::UnInit() {
	if (m_ViewParticleSystem != nullptr) {
		m_ViewParticleSystem->UnInit();
		m_ViewParticleSystem = nullptr;
	}
	m_ParticleSystems.UnInit();
}

//�X�V
void ParticleEditor::Update() {

	m_TargetBillBoard.SetPosition(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);//�^�[�Q�b�g

	//ImGui�\��
	ImVec4 clear_color = ImVec4(0.25f, 0.5f, 0.35f, 1.00f);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (isActive == true) {
	m_ParticleSystems.Update();//�p�[�e�B�N���V�X�e��
	}

	if (m_ParticleSystems.getParticleSystemCount() <= 0) {
		m_ViewParticleSystem = nullptr;
	}
}
//�`��
void ParticleEditor::Draw() {
	m_ParticleSystems.Draw();

	if (isDrawTargetObj) {
		m_TargetBillBoard.DrawBillBoardAdd(CCamera::GetInstance()->GetCameraMatrix());//�^�[�Q�b�g
	}

	ImGuiDrawMain();//����UI

	//�p�[�e�B�N���V�X�e�����Ƃ�UI
	if (m_ViewParticleSystem != nullptr) {
		ImGuiDrawofParticleSystem(m_ViewParticleSystem);
	}
	// ImGui�`�攽�f
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

//���C���E�B���h�E
void ParticleEditor::ImGuiDrawMain() {
	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, SCREEN_Y), ImGuiSetCond_Once);

	ImGui::Begin("Main");
	ImGui::Text("FPS:%d", g_nCountFPS);//FPS�\��

	//�z�[�~���O�^�[�Q�b�g�ݒ�
	if (ImGui::SliderFloat3("TargetPosition", m_TargetPosf, -100, 100)) {
		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			iParticleSystem.second->SetTargetPos(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);//���W
		}
	}
	ImGui::Checkbox("TargetRendering", &isDrawTargetObj);//�\����\��

	//�p�[�e�B�N���V�X�e���ǉ�����
	if (ImGui::Button("Create")) {
		AddParticleSystem();
	}

	//�t�@�C���ǂݍ���
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;                                              //�t�@�C������n���h��
	std::string DirectoryName = ".\\ParticleData\\*.ptc";         //�ǂݍ��ރt�H���_�̃f�B���N�g���Ɗg���q�w��
	
	if (ImGui::TreeNode("Import")) {                           //Imgui�̃c���[���J���ꂽ��
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//�t�@�C�������݂��邩�m�F
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {        //���������t�@�C�����{�^���Ƃ��ĕ\��
					                                           //�{�^���������ꂽ�炻�̃t�@�C����ǂݍ��ݔ��f
					InputData(win32fd.cFileName);

					m_ViewParticleSystem = nullptr;
				}
			} while (FindNextFile(hFind, &win32fd));           //���̃t�@�C����{��
		}
		ImGui::TreePop();
		FindClose(hFind);

	}
	//�t�@�C�������o��
	
	if (ImGui::TreeNode("Export")) {
		ImGui::InputText("FileName", m_FileName, sizeof(m_FileName));
		if (ImGui::Button("Export Data")) {
			OutputData(m_FileName);
		}
		ImGui::TreePop();
	}


	//�p�[�e�B�N���V�X�e�����X�g
	ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 300), ImGuiWindowFlags_NoTitleBar);
	for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
		if (ImGui::Button(iParticleSystem.second->getName())) {
			m_ViewParticleSystem = iParticleSystem.second;
		}
	}
	ImGui::EndChild();

	//�p�[�e�B�N���V�X�e���S�̂ւ̑���
	{
		static bool DeleteFlag = false;
		if (ImGui::Button("Delete")) {//�p�[�e�B�N���S�폜�{�^��
			if (m_ParticleSystems.getParticleSystemNum() > 0) {
				DeleteFlag = true;
			}
		}

		if (DeleteFlag) {
			WARNING_REACTION Reaction = ImGuiWarningText("Delete?");

			switch (Reaction) {
			case WARNING_REACTION::OK://OK�����ꂽ
				//�p�[�e�B�N���S�폜
				DeleteParticleSystems();
				DeleteFlag = false;
				break;

			case WARNING_REACTION::CANCEL://Cancel�����ꂽ
				//�߂�
				DeleteFlag = false;
				break;

			case WARNING_REACTION::DONT_PUSH://������Ă��Ȃ�
				break;
			}
		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();


	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(200, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	//�p�[�e�B�N���V�X�e���S�̂̎��s����
	ImGui::Begin("SystemOparate");

	if (ImGui::Button("Start")) {
		isActive = true;//�ĊJ
	}
	if (ImGui::Button("Stop")) {
		isActive = false;//��~
	}
	if (ImGui::Button("ReStart")) {
		m_ParticleSystems.Start();//�ăX�^�[�g(�S��)
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

}
//�p�[�e�B�N���V�X�e�����ꂼ���ImGui�`��
void ParticleEditor::ImGuiDrawofParticleSystem(ParticleSystem* pParticleSystem_) {
	t_ParticleSystemState ViewState = pParticleSystem_->GetState();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 300, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, SCREEN_Y), ImGuiSetCond_Once);

	char WindowName[256] = "ParticleSystem : ";
	strcat_s(WindowName, sizeof(WindowName), pParticleSystem_->getName());
	ImGui::Begin(WindowName, nullptr, ImGuiWindowFlags_MenuBar);

	CheckDataChange = 0;//���l�ύX�Ď��ϐ�������

	//�p�[�e�B�N���V�X�e������
	{
		static bool DeleteFlag = false;
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Copy")) {
				AddParticleSystem(&ViewState);//�R�s�[

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Delete")) {//�폜
				DeleteFlag = true;

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (DeleteFlag) {
			WARNING_REACTION Reaction = ImGuiWarningText("Delete?");

			//�x�����ւ̔����ɑ΂��鏈��
			switch (Reaction) {
			case WARNING_REACTION::OK://OK�����ꂽ
				//�Ώۂ̃p�[�e�B�N���V�X�e���폜
				m_ParticleSystems.RemoveParticleSystem(pParticleSystem_->getSystemNumber());
				m_ViewParticleSystem = nullptr;

				DeleteFlag = false;
				break;

			case WARNING_REACTION::CANCEL://Cancel�����ꂽ
				//�߂�
				DeleteFlag = false;
				break;

			case WARNING_REACTION::DONT_PUSH://������Ă��Ȃ�
				break;

			}
		}
	}
	
	//���O�\��
	//#TODO �Ȃ����P�������������͂ł��Ȃ��̂ŏC�����K�v
	if (ImGui::InputText("", ViewState.m_Name, sizeof(ViewState.m_Name))){
		pParticleSystem_->SetName(ViewState.m_Name);
	}

	//�p�[�e�B�N�������󋵃o�[
	ImGui::ProgressBar(1.0f - (pParticleSystem_->getLifeTime() / (ViewState.m_DuaringTime + ViewState.m_StartDelayTime)), ImVec2(250, 20));

	ImGui::Spacing();

	//�p�[�e�B�N���V�X�e���̐ݒ葀��
	//ImGui�̐��l���ύX���ꂽ��true���Ԃ��Ă���@�\�𗘗p��CHECK�}�N���ŊĎ�
	if (ImGui::CollapsingHeader("Main")) {
		CHECK(ImGui::DragFloat3("Position", ViewState.m_Position, 1.0f));                //���W
		CHECK(ImGui::DragInt3("Angle", ViewState.m_Angle, 1.0f));                        //���o�p�x
		CHECK(ImGui::DragInt("AngleRange", &ViewState.m_AngleRange, 1, 1, 360));         //���o�p�x�͈�
		CHECK(ImGui::InputInt("ParticleNum", &ViewState.m_ParticleNum, 5, 1000));        //�p�[�e�B�N���̌�
		CHECK(ImGui::InputFloat("Size", &ViewState.m_Size, 1.0f, 100.0f));               //���q�̑傫��
	}

	if (ImGui::CollapsingHeader("Time")) {
		CHECK(ImGui::InputFloat("Duaring", &ViewState.m_DuaringTime, 1, 10));            //����
		CHECK(ImGui::InputFloat("StartDelayTime", &ViewState.m_StartDelayTime, 1, 10));  //�J�n�x������
		CHECK(ImGui::InputFloat("MaxLifeTime", &ViewState.m_MaxLifeTime, 1.0f, 10.0f));  //�ő吶������
	}

	if (ImGui::CollapsingHeader("Move")) {
		CHECK(ImGui::InputFloat("Speed", &ViewState.m_Speed, 1.0f, 100.0f));             //�ړ����x
		CHECK(ImGui::InputFloat("Accel", &ViewState.m_Accel, 0.1f, 10.0f));              //�����x
		if (ViewState.m_Accel != 0.0f) {
			CHECK(ImGui::DragFloat("minSpeed", &ViewState.m_MinSpeed));                  //�ŏ����x
			CHECK(ImGui::DragFloat("maxSpeed", &ViewState.m_MaxSpeed));                  //�ő呬�x
		}
		CHECK(ImGui::InputInt("RotateSpeed", &ViewState.m_RotateSpeed, 1, 100));         //��]���x
	}
	//GPU�p�[�e�B�N��OnOff�`�F�b�N�{�b�N�X
	if (ImGui::Checkbox("GPUParticle", &ViewState.isGPUParticle)){
		pParticleSystem_->ChangeGPUParticleMode(ViewState.isGPUParticle);
		pParticleSystem_->Start();
		CheckDataChange += 1;
	}
	
	CHECK(ImGui::Checkbox("isChaser", &ViewState.isChaser));                    //�I�u�W�F�N�g��ǂ������邩

	if (ViewState.isChaser) {
		//#TODO ���ʂ��킩��Â炢
		CHECK(ImGui::SliderInt("minAngle", &ViewState.m_MinChaseAngle, 0, 360));//�ŏ��p�x
		CHECK(ImGui::SliderInt("maxAngle", &ViewState.m_MaxChaseAngle, 0, 360));//�ő�p�x
	}

	CHECK(ImGui::Checkbox("useGravity", &ViewState.UseGravity));                //�d�͗��p
	if (ViewState.UseGravity) {
		CHECK(ImGui::InputFloat3("Gravity", ViewState.m_Gravity));              //�d�͉����x�ݒ�
	}

	CHECK(ImGui::Checkbox("Looping", &ViewState.isLooping));                    //���[�v�����邩�ǂ���

	if (ImGui::Checkbox("isEmitter", &ViewState.isEmitting)) {                  //���̃p�[�e�B�N������̔����Ɍ��点�邩
		pParticleSystem_->SetEmitte(ViewState.isEmitting);
		CheckDataChange += 1;
	}
	if (ImGui::Checkbox("isActive", &ViewState.isActive)) {                     //�L������
		pParticleSystem_->SetActive(ViewState.isActive);
		pParticleSystem_->Start();
	}

	CHECK(ImGui::ColorEdit4("Color", ViewState.m_Color, 0));//�F

	//�Ď��������ݒ荀�ڂ͂�������̍s�ɏ���
	//�Ď�����-----------------------------------------------------------------------------
	//���l���ύX����Ă����甽�f
	if (CHECK_RESULT) {
		//�X�V
		pParticleSystem_->SetParticleSystemState(&ViewState);//�p�[�e�B�N���V�X�e���ݒ�\����
		pParticleSystem_->UpdateSRV();                       //GPGPU�ŗ��p����V�F�[�_�[���\�[�X�r���[
	}
	//-----------------------------------------------------------------------------------


	//�G�~�b�^�[

	//�G�~�b�^�[�Ώۃp�[�e�B�N���V�X�e�����\��
	std::vector<int> NextSystemNumbers = pParticleSystem_->getNextSystemNumbers();
	if (NextSystemNumbers.size() < 1) {
		ImGui::Text("null");
	}
	else {
		for (int num = 0; num < NextSystemNumbers.size(); num++) {
			ImGui::Text(m_ParticleSystems.getParticleSystem()[NextSystemNumbers[num]]->getName());
		}
	}

	//�G�~�b�^�[�̐ݒ�
	//���ׂẴp�[�e�B�N���V�X�e���̃��X�g���{�^���Ƃ��ĕ\������
	if (ImGui::TreeNode("Emitter")) {

		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			if (ImGui::Button(iParticleSystem.second->getName())) {//�{�^���������ꂽ��Ώۂ��G�~�b�^�[�ɒǉ�
				pParticleSystem_->SetNextParticleSystem(iParticleSystem.second->getSystemNumber());
			}
		}
		if (ImGui::Button("Null")) {
			pParticleSystem_->SetNextParticleSystem(-1);//-1������ƃ��X�g������
		}
		ImGui::TreePop();
	}
	
	//�t�@�C���A�N�Z�X�֘A-----------------------------------------------------------------------
	//�e�N�X�`���֘A
	{

		ImGui::Text(ViewState.m_TextureName);                      //�e�N�X�`���p�X�\��
		
		WIN32_FIND_DATA win32fd;
		HANDLE hFind;                                              //�t�@�C������n���h��

		std::string DirectoryName = ".\\InPutData\\*.png";
		if (ImGui::TreeNode("inputTexture")) {                     //Imgui�̃c���[���J���ꂽ��
			hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//�t�@�C�������݂��邩�m�F
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (ImGui::Button(win32fd.cFileName)) {        //���������t�@�C�����{�^���Ƃ��ĕ\��
						                                           //�{�^���������ꂽ�炻�̃t�@�C����ǂݍ��ݔ��f
						pParticleSystem_->FInTex(win32fd.cFileName);
					}
				} while (FindNextFile(hFind, &win32fd));           //���̃t�@�C����{��
			}
			ImGui::TreePop();
			FindClose(hFind);

		}
	}
	//---------------------------------------------------------------------------------------
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 500, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	ImGui::Begin("ParticleOparate");

	//���ꂼ��̏�����L�����������鑀��E�B���h�E
	ImGui::Checkbox("Update", pParticleSystem_->getisUpdateActive());
	ImGui::Checkbox("Draw", pParticleSystem_->getisDrawActive());
	if (ImGui::Button("ReStart")) {
		pParticleSystem_->Start();//�ăX�^�[�g(�P��)
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

//�x���\��
ParticleEditor::WARNING_REACTION ParticleEditor::ImGuiWarningText(const char* text) {
	WARNING_REACTION Reaction = WARNING_REACTION::DONT_PUSH;//�Ԃ�l

	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X / 2 - 75, SCREEN_Y / 2 - 40), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(150, 80), ImGuiSetCond_Once);

	ImGui::Begin("Watning");

	//�\��
	ImGui::Text(text);
	if (ImGui::Button("OK")) {
		Reaction = WARNING_REACTION::OK;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel")) {
		Reaction = WARNING_REACTION::CANCEL;
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	return Reaction;
}

//�S�폜
void ParticleEditor::DeleteParticleSystems() {
	m_ViewParticleSystem = nullptr;
	m_ParticleSystems.DeleteParticleSystem();
}

//�p�[�e�B�N���V�X�e���ǉ�
void ParticleEditor::AddParticleSystem(t_ParticleSystemState* setState) {
	ParticleSystem* CreatedParticleSystem;
	if (setState != nullptr) {
		CreatedParticleSystem = m_ParticleSystems.AddParticleSystem(setState);//�ǉ�	
	}
	else {
		CreatedParticleSystem = m_ParticleSystems.AddParticleSystem();//�ǉ�
	}

	//���O�ݒ�
	std::string Name = "Particle" + std::to_string(m_ParticleSystems.getParticleSystemCount());
	CreatedParticleSystem->SetName(Name.c_str());
	
	//�ҏW���ɂ���
	m_ViewParticleSystem = CreatedParticleSystem;
}

bool ParticleEditor::InputData(const char* FileName_) {
	DeleteParticleSystems();//�ҏW���̃p�[�e�B�N���V�X�e�����폜

	//�p�[�e�B�N���V�X�e���̏����t�@�C������ǂݍ���

	//�J�����g�f�B���N�g���𑀍삷��
	TCHAR crDir[MAX_PATH + 1];
	t_ParticleSystemState GetState;


	GetCurrentDirectory(MAX_PATH + 1, crDir);//����O�̃f�B���N�g�����擾
	SetCurrentDirectory(".\\ParticleData");//�ǂݍ��ݐ�t�H���_�փJ�����g�f�B���N�g����ύX

	//�t�@�C���ǂݍ���
	FILE *Fp;
	errno_t ErrorCheck;
	try {
		ErrorCheck = fopen_s(&Fp, FileName_, "rb");//

		SetCurrentDirectory(crDir);//�J�����g�f�B���N�g�������ɖ߂�

		if (ErrorCheck != 0) {
			throw 1;
		}

		int ParticleCount = 0;
		fread(&ParticleCount, sizeof(int), 1, Fp);//read1
		std::vector<int> nextNumbers;
		int NumbersSize;
		for (int Count = 0; Count < ParticleCount; Count++) {
			//�f�[�^��ǂݍ���Œǉ�
			fread(&GetState, sizeof(t_ParticleSystemState), 1, Fp);//read2
			
			fread(&NumbersSize, sizeof(int), 1, Fp);//read3 �v�f���ǂݍ���

			nextNumbers.clear();
			for (int num = 0; num < NumbersSize; num++) {
				int setNumber;
				fread(&setNumber, sizeof(int), 1, Fp);//read4 �v�f�ǂݍ���
				nextNumbers.emplace_back(setNumber);
			}

			m_ParticleSystems.AddParticleSystem(&GetState,nextNumbers);


		}
		m_ParticleSystems.setParticleCounter(ParticleCount);

		fclose(Fp);
	}
	catch (int i) {//�t�@�C���ǂݍ��ݎ��s������
		return i;
	};
	return true;
}

void ParticleEditor::OutputData(char* FileName_) {
	//�p�[�e�B�N���V�X�e���̏����t�@�C���֏����o��
	if (m_ParticleSystems.getParticleSystemCount() == 0) {
		return;
	}
	//�J�����g�f�B���N�g���𑀍삷��
	TCHAR crDir[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH + 1, crDir);//����O�̃f�B���N�g�����擾
	SetCurrentDirectory(".\\ParticleData");//�����o����̃t�H���_�փJ�����g�f�B���N�g����ύX

	//�t�@�C�������o��
	FILE *Fp;

	char ExtName[128];
	strcpy_s(ExtName, sizeof(ExtName), FileName_);
	strcat_s(ExtName, sizeof(ExtName), ".ptc");

	fopen_s(&Fp, ExtName, "wb");

	//�擪�Ƀp�[�e�B�N���̌����L�q
	int ParticleCount = m_ParticleSystems.getParticleSystemCount();
	fwrite(&ParticleCount, sizeof(int), 1, Fp);//write1

	//�f�[�^�����o��
	std::vector<int> nextNumbers;
	int NumbersSize;
	for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
		fwrite(&iParticleSystem.second->GetState(), sizeof(t_ParticleSystemState), 1, Fp);//write2

		//�p�[�e�B�N���ԍ����X�g�o��
		nextNumbers = iParticleSystem.second->getNextSystemNumbers();
		NumbersSize = static_cast<int>(nextNumbers.size());
		fwrite(&NumbersSize, sizeof(int), 1, Fp);//write3 //�v�f�������o��
		for (int num = 0; num < NumbersSize; num++) {
			fwrite(&nextNumbers[num], sizeof(int), 1, Fp);//write4 //�v�f��z��ŏ����o��
		}

	}

	fclose(Fp);

	SetCurrentDirectory(crDir);//�J�����g�f�B���N�g�������ɖ߂�

}