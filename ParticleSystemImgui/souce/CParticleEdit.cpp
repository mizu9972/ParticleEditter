#include <list>
#include <vector>
#include <string>
#include "CParticleEdit.h"
#include "CCamera.h"

#include "game.h"

#define CHECK(x) (CheckDataChange += x) //数値変更を感知
#define CHECK_RESULT (CheckDataChange > 0) //変更されたかどうか判定
static int CheckDataChange = 0;//ImGuiで数値が変更されたらtrueが返ってくるメソッドの仕様を利用して、数値変更を管理する変数

extern int g_nCountFPS;
constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";

extern float g_SectionTime;
//初期化
void ParticleEditor::Init() {
	
	//ホーミングターゲット初期化
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

	m_TargetBillBoard.SetPosition(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);//ターゲット

	//ImGui表示
	ImVec4 clear_color = ImVec4(0.25f, 0.5f, 0.35f, 1.00f);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (isActive == true) {
	m_ParticleSystems.Update();//パーティクルシステム
	}

	if (m_ParticleSystems.getParticleSystemCount() <= 0) {
		m_ViewParticleSystem = nullptr;
	}
}
//描画
void ParticleEditor::Draw() {
	m_ParticleSystems.Draw();

	m_TargetBillBoard.DrawBillBoardAdd(CCamera::GetInstance()->GetCameraMatrix());//ターゲット

	ImGuiDrawMain();//総合UI

	//パーティクルシステムごとのUI
	if (m_ViewParticleSystem != nullptr) {
		ImGuiDrawofParticleSystem(m_ViewParticleSystem);
	}
	// ImGui描画反映
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

	if (ImGui::SliderFloat3("TargetPosition", m_TargetPosf, -100, 100)) {
		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			iParticleSystem.second->SetTargetPos(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);
		}
	}

	//パーティクルシステム追加操作
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

	if (ImGui::Button("Delete")) {
		DeleteParticleSystems();
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();


	//Imguiウィンドウ設定
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

	CheckDataChange = 0;
	
	//名前
	if (ImGui::InputText("", ViewState.m_Name, sizeof(ViewState.m_Name))){
		pParticleSystem_->SetName(ViewState.m_Name);
	}

	ImGui::ProgressBar(1.0f - (pParticleSystem_->getLifeTime() / ViewState.m_DuaringTime), ImVec2(200, 20));
	CHECK(ImGui::DragFloat3("Position", ViewState.m_Position, 1.0f));  //座標
	CHECK(ImGui::DragInt3("Angle", ViewState.m_Angle, 1.0f));                   //放出角度
	CHECK(ImGui::DragInt("AngleRange", &ViewState.m_AngleRange,1, 1, 360));         //放出角度範囲
	CHECK(ImGui::InputFloat("Duaring", &ViewState.m_DuaringTime, 1, 10));          //周期


	CHECK(ImGui::InputInt("ParticleNum", &ViewState.m_ParticleNum, 5, 1000));       //パーティクルの個数
	CHECK(ImGui::InputFloat("MaxLifeTime", &ViewState.m_MaxLifeTime, 1.0f, 10.0f));//最大生存時間
	CHECK(ImGui::InputFloat("Size", &ViewState.m_Size, 1.0f, 100.0f));             //粒子の大きさ
	CHECK(ImGui::InputFloat("Speed", &ViewState.m_Speed, 1.0f, 100.0f));           //移動速度
	CHECK(ImGui::InputFloat("Accel", &ViewState.m_Accel, 0.1f));                          //加速度
	CHECK(ImGui::InputInt("RotateSpeed", &ViewState.m_RotateSpeed, 1, 100));       //回転速度

	//GPUパーティクルのチェックボックスがfalse->trueに変更されたらStart関数呼び出し
	if (ImGui::Checkbox("GPUParticle", &ViewState.isGPUParticle)){//GPUパーティクル) {//変更判定
		//if (ViewState.isGPUParticle == true) { //true判定
		//	pParticleSystem_->StartGPUParticle();
		//}
		pParticleSystem_->ChangeGPUParticleMode(ViewState.isGPUParticle);
		pParticleSystem_->Start();
		CheckDataChange += 1;
	}
	
	CHECK(ImGui::Checkbox("isChaser", &ViewState.isChaser));//オブジェクトを追いかけるか

	if (ViewState.isChaser) {
		CHECK(ImGui::SliderInt("minAngle", &ViewState.m_MinChaseAngle, 0, 360));
		CHECK(ImGui::SliderInt("maxAngle", &ViewState.m_MaxChaseAngle, 0, 360));
	}

	CHECK(ImGui::Checkbox("useGravity", &ViewState.UseGravity));
	if (ViewState.UseGravity) {
		CHECK(ImGui::InputFloat3("Gravity", ViewState.m_Gravity));
	}

	CHECK(ImGui::Checkbox("isLooping", &ViewState.isLooping));//ループさせるかどうか

	if (ImGui::Checkbox("isEmitter", &ViewState.isEmitting)) {//他のパーティクルからの発生に限らせるか
		pParticleSystem_->SetEmitte(ViewState.isEmitting);
		CheckDataChange += 1;
	}
	if (ImGui::Checkbox("isActive", &ViewState.isActive)) {
		pParticleSystem_->SetActive(ViewState.isActive);
		pParticleSystem_->Start();
	}
	//CHECK(ImGui::Checkbox("isActive", &ViewState.isActive));//発生させるかどうか

	CHECK(ImGui::ColorEdit4("Color", ViewState.m_Color, 0));//色

	if (CHECK_RESULT) {//数値が変更されていたら反映
		pParticleSystem_->SetParticleSystemState(&ViewState);
		pParticleSystem_->UpdateSRV();
	}
	//エミッター
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
	//エミッターの設定
	//すべてのパーティクルシステムのリストをボタンとして表示する
	if (ImGui::TreeNode("Emitter")) {

		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			if (ImGui::Button(iParticleSystem.second->getName())) {
				pParticleSystem_->SetNextParticleSystem(iParticleSystem.second->getSystemNumber());
			}
		}
		if (ImGui::Button("Null")) {
			pParticleSystem_->SetNextParticleSystem(-1);//-1を入れるとリスト初期化
		}
		ImGui::TreePop();
	}
	
	//ファイルアクセス関連

	//テクスチャ関連
	{

		ImGui::Text(ViewState.m_TextureName);//テクスチャパス表示
		
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

	//Imguiウィンドウ設定
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
		std::vector<int> nextNumbers;
		int NumbersSize;
		for (int Count = 0; Count < ParticleCount; Count++) {
			//データを読み込んで追加
			fread(&GetState, sizeof(t_ParticleSystemState), 1, Fp);//read2
			fread(&NumbersSize, sizeof(int), 1, Fp);//read3 要素数読み込み

			nextNumbers.clear();
			for (int num = 0; num < NumbersSize; num++) {
				int setNumber;
				fread(&setNumber, sizeof(int), 1, Fp);//read4 要素読み込み
				nextNumbers.emplace_back(setNumber);
			}

			m_ParticleSystems.AddParticleSystem(&GetState,nextNumbers);


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
	std::vector<int> nextNumbers;
	int NumbersSize;
	for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
		fwrite(&iParticleSystem.second->GetState(), sizeof(t_ParticleSystemState), 1, Fp);//write2

		//パーティクル番号リスト出力
		nextNumbers = iParticleSystem.second->getNextSystemNumbers();
		NumbersSize = static_cast<int>(nextNumbers.size());
		fwrite(&NumbersSize, sizeof(int), 1, Fp);//write3 //要素数書き出し
		for (int num = 0; num < NumbersSize; num++) {
			fwrite(&nextNumbers[num], sizeof(int), 1, Fp);//write4 //要素を配列で書き出し
		}

	}

	fclose(Fp);

	SetCurrentDirectory(crDir);//カレントディレクトリを元に戻す

}