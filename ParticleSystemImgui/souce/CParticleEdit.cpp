#include <list>
#include <string>
#include "CParticleEdit.h"
#include "CCamera.h"

#include "game.h"

#define CHECK(x) (CheckDataChange += x)
#define CHECK_RESULT (CheckDataChange > 0)

extern int g_nCountFPS;
constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";

extern float g_SectionTime;
static int CheckDataChange = 0;
//初期化
void ParticleEditor::Init() {
	m_TargetBillBoard.Init(
		m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2],
		50, 50,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		"shader/psParticle.fx",
		"shader/vsPrticle.fx"
	);

	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };

	m_TargetBillBoard.SetUV(u, v);
	m_TargetBillBoard.LoadTexTure(PARTICLE_TEXTURE);
}

//終了処理
void ParticleEditor::UnInit() {
	if (m_ViewParticleSystem != nullptr) {
		m_ViewParticleSystem->UnInit();
		m_ViewParticleSystem = nullptr;
	}
	m_ParticleSystems.UnInit();
}

//更新
void ParticleEditor::Update() {
	m_TargetBillBoard.SetPosition(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);

	if (m_ViewParticleSystem != nullptr) {
		m_ViewParticleSystem->SetTargetPos(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);
	}
	//ImGui表示
	ImVec4 clear_color = ImVec4(0.25f, 0.5f, 0.35f, 1.00f);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_ParticleSystems.Update();

	if (m_ParticleSystems.getParticleSystemCount() <= 0) {
		m_ViewParticleSystem = nullptr;
	}
}
//描画
void ParticleEditor::Draw() {
	m_ParticleSystems.Draw();

	m_TargetBillBoard.DrawBillBoardAdd(CCamera::GetInstance()->GetCameraMatrix());

	ImGuiDrawMain();

	if (m_ViewParticleSystem != nullptr) {
		ImGuiDrawofParticleSystem(m_ViewParticleSystem);
	}
	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ParticleEditor::ImGuiDrawMain() {

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, SCREEN_Y), ImGuiSetCond_Once);

	ImGui::Begin("Main");
	ImGui::Text("FPS:%d", g_nCountFPS);

	ImGui::SliderFloat3("TargetPosition", m_TargetPosf, -100, 100);

	if (ImGui::Button("Create")) {
		ParticleSystem* CreatedParticleSystem = m_ParticleSystems.AddParticleSystem();
		std::string Name = "Particle" + std::to_string(m_ParticleSystems.getParticleSystemCount());
		CreatedParticleSystem->SetName(Name.c_str());//名前設定
		m_ViewParticleSystem = CreatedParticleSystem;
	}

	//ファイル読み込み
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;//ファイル操作ハンドル
	std::string DirectoryName = ".\\InPutData\\*.txt";//読み込むフォルダのディレクトリと拡張子指定


	if (ImGui::TreeNode("Import")) {//Imguiのツリーが開かれたら
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//ファイルが存在するか確認
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {//発見したファイルをボタンとして表示
					//ボタンが押されたらそのファイルを読み込み反映
					InputData(win32fd.cFileName);
				}
			} while (FindNextFile(hFind, &win32fd));//次のファイルを捜索
		}
		ImGui::TreePop();
		FindClose(hFind);

	}
	//ファイル書き出し
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

	if (ImGui::Button("Reset")) {
		DeleteParticleSystems();
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();


	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 400, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	ImGui::Begin("oparate");

	if (ImGui::Button("Start")) {

	}
	if (ImGui::Button("Stop")) {

	}
	if (ImGui::Button("Reset")) {
		m_ParticleSystems.Start();
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

}
//パーティクルシステムそれぞれのImGui描画用
void ParticleEditor::ImGuiDrawofParticleSystem(ParticleSystem* pParticleSystem_) {
	t_ParticleSystemState ViewState = pParticleSystem_->GetState();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 200, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, SCREEN_Y), ImGuiSetCond_Once);

	ImGui::Begin(pParticleSystem_->getName());
	std::string str;

	ImGui::InputText("",ViewState.m_Name,sizeof(ViewState.m_Name));

	CheckDataChange = 0;

	//座標
	if (ImGui::TreeNode("Position")) {
		CHECK(ImGui::SliderFloat("X", &ViewState.m_PositionX, -100.0f, 100.0f));
		CHECK(ImGui::SliderFloat("Y", &ViewState.m_PositionY, -100.0f, 100.0f));
		CHECK(ImGui::SliderFloat("Z", &ViewState.m_PositionZ, -100.0f, 100.0f));
		ImGui::TreePop();
	}
	//放出角度
	if (ImGui::TreeNode("Angle")) {
		CHECK(ImGui::SliderInt("X", &ViewState.m_AngleX, 1, 360));
		CHECK(ImGui::SliderInt("Y", &ViewState.m_AngleY, 1, 360));
		CHECK(ImGui::SliderInt("Z", &ViewState.m_AngleZ, 1, 360));
		ImGui::TreePop();
	}
	//放出角度範囲
	CHECK(ImGui::SliderInt("AngleRange", &ViewState.m_AngleRange, 1, 360));

	CHECK(ImGui::SliderFloat("Duaring", &ViewState.m_DuaringTime, 0, 10));//周期
	CHECK(ImGui::InputInt("ParticleNum", &ViewState.m_ParticleNum, 5, 1000));//パーティクルの個数
	CHECK(ImGui::SliderFloat("MaxLifeTime", &ViewState.m_MaxLifeTime, 0.0f, 10.0f));//最大生存時間
	CHECK(ImGui::SliderFloat("Size", &ViewState.m_Size, 0.0f, 100.0f));//粒子の大きさ
	CHECK(ImGui::SliderFloat("Speed", &ViewState.m_Speed, 0.0f, 100.0f));//移動速度
	CHECK(ImGui::SliderInt("RotateSpeed", &ViewState.m_RotateSpeed, 0, 100));//回転速度


	//GPUパーティクルのチェックボックスがfalse->trueに変更されたらStart関数呼び出し
	if (ImGui::Checkbox("GPUParticle", &ViewState.isGPUParticle)){//GPUパーティクル) {//変更判定
		if (ViewState.isGPUParticle == true) { //true判定
			pParticleSystem_->StartGPUParticle();
		}
		pParticleSystem_->ChangeGPUParticleMode(ViewState.isGPUParticle);
		CheckDataChange += 1;
	}
	
	CHECK(ImGui::Checkbox("isChaser", &ViewState.isChaser));//オブジェクトを追いかけるか
	CHECK(ImGui::Checkbox("isLooping", &ViewState.isLooping));//ループさせるかどうか
	CHECK(ImGui::Checkbox("isActive", &ViewState.isActive));//発生させるかどうか

	CHECK(ImGui::ColorEdit4("Color", ViewState.m_Color, 0));//色

	if (CHECK_RESULT) {//数値が変更されていたら反映
		pParticleSystem_->SetParticleSystemState(&ViewState);
		pParticleSystem_->UpdateConstantBuffer();
	}
	//エミッター
	int NextSystemNumber = ViewState.m_NextSystemNumber;
	if (NextSystemNumber == -1) {
		ImGui::Text("null");
	}
	else {
		ImGui::Text(m_ParticleSystems.getParticleSystem()[NextSystemNumber]->getName());
	}
	//エミッターの設定
	//すべてのパーティクルシステムのリストをボタンとして表示する
	if (ImGui::TreeNode("Emitter")) {

		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			if (ImGui::Button(iParticleSystem.second->getName())) {
				pParticleSystem_->SetNextParticleSystem(iParticleSystem.second->getSystemNumber());
			}
		}
		ImGui::TreePop();
	}
	
	//ファイルアクセス関連

	//テクスチャ関連
	{
		ImGui::Text(ViewState.m_TextureName);
		//テクスチャ読み込み
		WIN32_FIND_DATA win32fd;
		HANDLE hFind;//ファイル操作ハンドル

		std::string DirectoryName = ".\\InPutData\\*.png";
		if (ImGui::TreeNode("inputTexture")) {//Imguiのツリーが開かれたら
			hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//ファイルが存在するか確認
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (ImGui::Button(win32fd.cFileName)) {//発見したファイルをボタンとして表示
						//ボタンが押されたらそのファイルを読み込み反映
						pParticleSystem_->FInTex(win32fd.cFileName);
					}
				} while (FindNextFile(hFind, &win32fd));//次のファイルを捜索
			}
			ImGui::TreePop();
			FindClose(hFind);

		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

}

void ParticleEditor::DeleteParticleSystems() {
	m_ParticleSystems.DeleteParticleSystem();
}

bool ParticleEditor::InputData(const char* FileName_) {
	DeleteParticleSystems();//編集中のパーティクルシステムを削除

	//パーティクルシステムの情報をファイルから読み込む

	//カレントディレクトリを操作する
	TCHAR crDir[MAX_PATH + 1];
	t_ParticleSystemState GetState;


	GetCurrentDirectory(MAX_PATH + 1, crDir);//操作前のディレクトリを取得
	SetCurrentDirectory(".\\InPutData");//読み込み先フォルダへカレントディレクトリを変更

	//ファイル読み込み
	FILE *Fp;
	errno_t ErrorCheck;
	try {
		ErrorCheck = fopen_s(&Fp, FileName_, "rb");//

		SetCurrentDirectory(crDir);//カレントディレクトリを元に戻す

		if (ErrorCheck != 0) {
			throw 1;
		}

		int ParticleCount = 0;
		fread(&ParticleCount, sizeof(int), 1, Fp);//read1

		for (int Count = 0; Count < ParticleCount; Count++) {
			//データを読み込んで追加
			fread(&GetState, sizeof(t_ParticleSystemState), 1, Fp);//read2
			m_ParticleSystems.AddParticleSystem(&GetState);
		}
		m_ParticleSystems.setParticleCounter(ParticleCount);

		fclose(Fp);
	}
	catch (int i) {//ファイル読み込み失敗したら
		return i;
	};
	return true;
}

void ParticleEditor::OutputData() {
	//パーティクルシステムの情報をファイルへ書き出し
	if (m_ParticleSystems.getParticleSystemCount() == 0) {
		return;
	}
//カレントディレクトリを操作する
	TCHAR crDir[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH + 1, crDir);//操作前のディレクトリを取得
	SetCurrentDirectory(".\\OutPutData");//書き出し先のフォルダへカレントディレクトリを変更

	//ファイル書き出し
	FILE *Fp;

	fopen_s(&Fp, "ParticleState.txt", "wb");

	//先頭にパーティクルの個数を記述
	int ParticleCount = m_ParticleSystems.getParticleSystemCount();
	fwrite(&ParticleCount, sizeof(int), 1, Fp);//write1

	//データ書き出し
	for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
		fwrite(&iParticleSystem.second->GetState(), sizeof(t_ParticleSystemState), 1, Fp);//write2
	}

	fclose(Fp);

	SetCurrentDirectory(crDir);//カレントディレクトリを元に戻す

}