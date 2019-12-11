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
constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";

extern float g_SectionTime;
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

	m_TargetBillBoard.DrawBillBoardAdd(CCamera::GetInstance()->GetCameraMatrix());//�^�[�Q�b�g

	ImGuiDrawMain();//����UI

	//�p�[�e�B�N���V�X�e�����Ƃ�UI
	if (m_ViewParticleSystem != nullptr) {
		ImGuiDrawofParticleSystem(m_ViewParticleSystem);
	}
	// ImGui�`�攽�f
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ParticleEditor::ImGuiDrawMain() {
	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, SCREEN_Y), ImGuiSetCond_Once);

	ImGui::Begin("Main");
	ImGui::Text("FPS:%d", g_nCountFPS);

	if (ImGui::SliderFloat3("TargetPosition", m_TargetPosf, -100, 100)) {
		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			iParticleSystem.second->SetTargetPos(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);
		}
	}

	//�p�[�e�B�N���V�X�e���ǉ�����
	if (ImGui::Button("Create")) {
		ParticleSystem* CreatedParticleSystem = m_ParticleSystems.AddParticleSystem();
		std::string Name = "Particle" + std::to_string(m_ParticleSystems.getParticleSystemCount());
		CreatedParticleSystem->SetName(Name.c_str());//���O�ݒ�
		m_ViewParticleSystem = CreatedParticleSystem;
	}

	//�t�@�C���ǂݍ���
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;//�t�@�C������n���h��
	std::string DirectoryName = ".\\InPutData\\*.txt";//�ǂݍ��ރt�H���_�̃f�B���N�g���Ɗg���q�w��


	if (ImGui::TreeNode("Import")) {//Imgui�̃c���[���J���ꂽ��
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//�t�@�C�������݂��邩�m�F
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {//���������t�@�C�����{�^���Ƃ��ĕ\��
					//�{�^���������ꂽ�炻�̃t�@�C����ǂݍ��ݔ��f
					InputData(win32fd.cFileName);
				}
			} while (FindNextFile(hFind, &win32fd));//���̃t�@�C����{��
		}
		ImGui::TreePop();
		FindClose(hFind);

	}
	//�t�@�C�������o��
	if (ImGui::Button("Export")) {
		OutputData();
	}

	ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 300), ImGuiWindowFlags_NoTitleBar);
	//for (ParticleSystem* iParticleSystem : m_ParticleSystems.getParticleSystem()) {
	//	if (ImGui::Button(iParticleSystem->getName())) {
	//		m_ViewParticleSystem = iParticleSystem;
	//	}
	//}

	for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
		if (ImGui::Button(iParticleSystem.second->getName())) {
			m_ViewParticleSystem = iParticleSystem.second;
		}
	}
	ImGui::EndChild();

	if (ImGui::Button("Delete")) {
		DeleteParticleSystems();
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();


	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(200, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	ImGui::Begin("SystemOparate");

	if (ImGui::Button("Start")) {
		isActive = true;
	}
	if (ImGui::Button("Stop")) {
		isActive = false;
	}
	if (ImGui::Button("Reset")) {
		m_ParticleSystems.Start();
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

}
//�p�[�e�B�N���V�X�e�����ꂼ���ImGui�`��p
void ParticleEditor::ImGuiDrawofParticleSystem(ParticleSystem* pParticleSystem_) {
	t_ParticleSystemState ViewState = pParticleSystem_->GetState();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 200, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, SCREEN_Y), ImGuiSetCond_Once);

	ImGui::Begin(pParticleSystem_->getName());

	CheckDataChange = 0;
	
	//���O
	if (ImGui::InputText("", ViewState.m_Name, sizeof(ViewState.m_Name))){
		pParticleSystem_->SetName(ViewState.m_Name);
	}

	ImGui::ProgressBar(1.0f - (pParticleSystem_->getLifeTime() / ViewState.m_DuaringTime), ImVec2(200, 20));
	CHECK(ImGui::DragFloat3("Position", ViewState.m_Position, 1.0f));  //���W
	CHECK(ImGui::DragInt3("Angle", ViewState.m_Angle, 1.0f));                   //���o�p�x
	CHECK(ImGui::DragInt("AngleRange", &ViewState.m_AngleRange,1, 1, 360));         //���o�p�x�͈�
	CHECK(ImGui::InputFloat("Duaring", &ViewState.m_DuaringTime, 1, 10));          //����


	CHECK(ImGui::InputInt("ParticleNum", &ViewState.m_ParticleNum, 5, 1000));       //�p�[�e�B�N���̌�
	CHECK(ImGui::InputFloat("MaxLifeTime", &ViewState.m_MaxLifeTime, 1.0f, 10.0f));//�ő吶������
	CHECK(ImGui::InputFloat("Size", &ViewState.m_Size, 1.0f, 100.0f));             //���q�̑傫��
	CHECK(ImGui::InputFloat("Speed", &ViewState.m_Speed, 1.0f, 100.0f));           //�ړ����x
	CHECK(ImGui::InputFloat("Accel", &ViewState.m_Accel, 0.1f));                          //�����x
	CHECK(ImGui::InputInt("RotateSpeed", &ViewState.m_RotateSpeed, 1, 100));       //��]���x

	//GPU�p�[�e�B�N���̃`�F�b�N�{�b�N�X��false->true�ɕύX���ꂽ��Start�֐��Ăяo��
	if (ImGui::Checkbox("GPUParticle", &ViewState.isGPUParticle)){//GPU�p�[�e�B�N��) {//�ύX����
		//if (ViewState.isGPUParticle == true) { //true����
		//	pParticleSystem_->StartGPUParticle();
		//}
		pParticleSystem_->ChangeGPUParticleMode(ViewState.isGPUParticle);
		pParticleSystem_->Start();
		CheckDataChange += 1;
	}
	
	CHECK(ImGui::Checkbox("isChaser", &ViewState.isChaser));//�I�u�W�F�N�g��ǂ������邩

	if (ViewState.isChaser) {
		CHECK(ImGui::SliderInt("minAngle", &ViewState.m_MinChaseAngle, 0, 360));
		CHECK(ImGui::SliderInt("maxAngle", &ViewState.m_MaxChaseAngle, 0, 360));
	}

	CHECK(ImGui::Checkbox("useGravity", &ViewState.UseGravity));
	if (ViewState.UseGravity) {
		CHECK(ImGui::InputFloat3("Gravity", ViewState.m_Gravity));
	}

	CHECK(ImGui::Checkbox("isLooping", &ViewState.isLooping));//���[�v�����邩�ǂ���

	if (ImGui::Checkbox("isEmitter", &ViewState.isEmitting)) {//���̃p�[�e�B�N������̔����Ɍ��点�邩
		pParticleSystem_->SetEmitte(ViewState.isEmitting);
		CheckDataChange += 1;
	}
	if (ImGui::Checkbox("isActive", &ViewState.isActive)) {
		pParticleSystem_->SetActive(ViewState.isActive);
		pParticleSystem_->Start();
	}
	//CHECK(ImGui::Checkbox("isActive", &ViewState.isActive));//���������邩�ǂ���

	CHECK(ImGui::ColorEdit4("Color", ViewState.m_Color, 0));//�F

	if (CHECK_RESULT) {//���l���ύX����Ă����甽�f
		pParticleSystem_->SetParticleSystemState(&ViewState);
		pParticleSystem_->UpdateSRV();
	}
	//�G�~�b�^�[
	std::vector<int> NextSystemNumbers = pParticleSystem_->getNextSystemNumbers();
	if (NextSystemNumbers.size() < 1) {
		ImGui::Text("null");
	}
	else {
		for (int num = 0; num < NextSystemNumbers.size(); num++) {
			ImGui::Text(m_ParticleSystems.getParticleSystem()[NextSystemNumbers[num]]->getName());
			//ImGui::Text(m_ParticleSystems.getParticleSystem()[NextSystemNumber]->getName());
		}
	}
	//�G�~�b�^�[�̐ݒ�
	//���ׂẴp�[�e�B�N���V�X�e���̃��X�g���{�^���Ƃ��ĕ\������
	if (ImGui::TreeNode("Emitter")) {

		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			if (ImGui::Button(iParticleSystem.second->getName())) {
				pParticleSystem_->SetNextParticleSystem(iParticleSystem.second->getSystemNumber());
			}
		}
		if (ImGui::Button("Null")) {
			pParticleSystem_->SetNextParticleSystem(-1);//-1������ƃ��X�g������
		}
		ImGui::TreePop();
	}
	
	//�t�@�C���A�N�Z�X�֘A

	//�e�N�X�`���֘A
	{

		ImGui::Text(ViewState.m_TextureName);//�e�N�X�`���p�X�\��
		
		//�e�N�X�`���ǂݍ���
		WIN32_FIND_DATA win32fd;
		HANDLE hFind;//�t�@�C������n���h��

		std::string DirectoryName = ".\\InPutData\\*.png";
		if (ImGui::TreeNode("inputTexture")) {//Imgui�̃c���[���J���ꂽ��
			hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//�t�@�C�������݂��邩�m�F
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (ImGui::Button(win32fd.cFileName)) {//���������t�@�C�����{�^���Ƃ��ĕ\��
						//�{�^���������ꂽ�炻�̃t�@�C����ǂݍ��ݔ��f
						pParticleSystem_->FInTex(win32fd.cFileName);
					}
				} while (FindNextFile(hFind, &win32fd));//���̃t�@�C����{��
			}
			ImGui::TreePop();
			FindClose(hFind);

		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	//Imgui�E�B���h�E�ݒ�
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 400, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	ImGui::Begin("ParticleOparate");

	ImGui::Checkbox("Update", pParticleSystem_->getisUpdateActive());
	ImGui::Checkbox("Draw", pParticleSystem_->getisDrawActive());
	if (ImGui::Button("Reset")) {
		pParticleSystem_->Start();
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void ParticleEditor::DeleteParticleSystems() {
	m_ParticleSystems.DeleteParticleSystem();
}

bool ParticleEditor::InputData(const char* FileName_) {
	DeleteParticleSystems();//�ҏW���̃p�[�e�B�N���V�X�e�����폜

	//�p�[�e�B�N���V�X�e���̏����t�@�C������ǂݍ���

	//�J�����g�f�B���N�g���𑀍삷��
	TCHAR crDir[MAX_PATH + 1];
	t_ParticleSystemState GetState;


	GetCurrentDirectory(MAX_PATH + 1, crDir);//����O�̃f�B���N�g�����擾
	SetCurrentDirectory(".\\InPutData");//�ǂݍ��ݐ�t�H���_�փJ�����g�f�B���N�g����ύX

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

void ParticleEditor::OutputData() {
	//�p�[�e�B�N���V�X�e���̏����t�@�C���֏����o��
	if (m_ParticleSystems.getParticleSystemCount() == 0) {
		return;
	}
//�J�����g�f�B���N�g���𑀍삷��
	TCHAR crDir[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH + 1, crDir);//����O�̃f�B���N�g�����擾
	SetCurrentDirectory(".\\OutPutData");//�����o����̃t�H���_�փJ�����g�f�B���N�g����ύX

	//�t�@�C�������o��
	FILE *Fp;

	fopen_s(&Fp, "ParticleState.txt", "wb");

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