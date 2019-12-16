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
	strcpy_s(m_FileName, sizeof(m_FileName), "ParticleState");

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

	if (isDrawTargetObj) {
		m_TargetBillBoard.DrawBillBoardAdd(CCamera::GetInstance()->GetCameraMatrix());//ターゲット
	}

	ImGuiDrawMain();//総合UI

	//パーティクルシステムごとのUI
	if (m_ViewParticleSystem != nullptr) {
		ImGuiDrawofParticleSystem(m_ViewParticleSystem);
	}
	// ImGui描画反映
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

//メインウィンドウ
void ParticleEditor::ImGuiDrawMain() {
	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, SCREEN_Y), ImGuiSetCond_Once);

	ImGui::Begin("Main");
	ImGui::Text("FPS:%d", g_nCountFPS);//FPS表示

	//ホーミングターゲット設定
	if (ImGui::SliderFloat3("TargetPosition", m_TargetPosf, -100, 100)) {
		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			iParticleSystem.second->SetTargetPos(m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2]);//座標
		}
	}
	ImGui::Checkbox("TargetRendering", &isDrawTargetObj);//表示非表示

	//パーティクルシステム追加操作
	if (ImGui::Button("Create")) {
		AddParticleSystem();
	}

	//ファイル読み込み
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;                                              //ファイル操作ハンドル
	std::string DirectoryName = ".\\ParticleData\\*.ptc";         //読み込むフォルダのディレクトリと拡張子指定
	
	if (ImGui::TreeNode("Import")) {                           //Imguiのツリーが開かれたら
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//ファイルが存在するか確認
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {        //発見したファイルをボタンとして表示
					                                           //ボタンが押されたらそのファイルを読み込み反映
					InputData(win32fd.cFileName);

					m_ViewParticleSystem = nullptr;
				}
			} while (FindNextFile(hFind, &win32fd));           //次のファイルを捜索
		}
		ImGui::TreePop();
		FindClose(hFind);

	}
	//ファイル書き出し
	
	if (ImGui::TreeNode("Export")) {
		ImGui::InputText("FileName", m_FileName, sizeof(m_FileName));
		if (ImGui::Button("Export Data")) {
			OutputData(m_FileName);
		}
		ImGui::TreePop();
	}


	//パーティクルシステムリスト
	ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 300), ImGuiWindowFlags_NoTitleBar);
	for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
		if (ImGui::Button(iParticleSystem.second->getName())) {
			m_ViewParticleSystem = iParticleSystem.second;
		}
	}
	ImGui::EndChild();

	//パーティクルシステム全体への操作
	{
		static bool DeleteFlag = false;
		if (ImGui::Button("Delete")) {//パーティクル全削除ボタン
			if (m_ParticleSystems.getParticleSystemNum() > 0) {
				DeleteFlag = true;
			}
		}

		if (DeleteFlag) {
			WARNING_REACTION Reaction = ImGuiWarningText("Delete?");

			switch (Reaction) {
			case WARNING_REACTION::OK://OK押された
				//パーティクル全削除
				DeleteParticleSystems();
				DeleteFlag = false;
				break;

			case WARNING_REACTION::CANCEL://Cancel押された
				//戻す
				DeleteFlag = false;
				break;

			case WARNING_REACTION::DONT_PUSH://押されていない
				break;
			}
		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();


	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(200, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	//パーティクルシステム全体の実行制御
	ImGui::Begin("SystemOparate");

	if (ImGui::Button("Start")) {
		isActive = true;//再開
	}
	if (ImGui::Button("Stop")) {
		isActive = false;//停止
	}
	if (ImGui::Button("ReStart")) {
		m_ParticleSystems.Start();//再スタート(全体)
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

}
//パーティクルシステムそれぞれのImGui描画
void ParticleEditor::ImGuiDrawofParticleSystem(ParticleSystem* pParticleSystem_) {
	t_ParticleSystemState ViewState = pParticleSystem_->GetState();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 300, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, SCREEN_Y), ImGuiSetCond_Once);

	char WindowName[256] = "ParticleSystem : ";
	strcat_s(WindowName, sizeof(WindowName), pParticleSystem_->getName());
	ImGui::Begin(WindowName, nullptr, ImGuiWindowFlags_MenuBar);

	CheckDataChange = 0;//数値変更監視変数初期化

	//パーティクルシステム制御
	{
		static bool DeleteFlag = false;
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Copy")) {
				AddParticleSystem(&ViewState);//コピー

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Delete")) {//削除
				DeleteFlag = true;

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (DeleteFlag) {
			WARNING_REACTION Reaction = ImGuiWarningText("Delete?");

			//警告文への反応に対する処理
			switch (Reaction) {
			case WARNING_REACTION::OK://OK押された
				//対象のパーティクルシステム削除
				m_ParticleSystems.RemoveParticleSystem(pParticleSystem_->getSystemNumber());
				m_ViewParticleSystem = nullptr;

				DeleteFlag = false;
				break;

			case WARNING_REACTION::CANCEL://Cancel押された
				//戻す
				DeleteFlag = false;
				break;

			case WARNING_REACTION::DONT_PUSH://押されていない
				break;

			}
		}
	}
	
	//名前表示
	//#TODO なぜか１文字ずつしか入力できないので修正が必要
	if (ImGui::InputText("", ViewState.m_Name, sizeof(ViewState.m_Name))){
		pParticleSystem_->SetName(ViewState.m_Name);
	}

	//パーティクル発生状況バー
	ImGui::ProgressBar(1.0f - (pParticleSystem_->getLifeTime() / (ViewState.m_DuaringTime + ViewState.m_StartDelayTime)), ImVec2(250, 20));

	ImGui::Spacing();

	//パーティクルシステムの設定操作
	//ImGuiの数値が変更されたらtrueが返ってくる機能を利用しCHECKマクロで監視
	if (ImGui::CollapsingHeader("Main")) {
		CHECK(ImGui::DragFloat3("Position", ViewState.m_Position, 1.0f));                //座標
		CHECK(ImGui::DragInt3("Angle", ViewState.m_Angle, 1.0f));                        //放出角度
		CHECK(ImGui::DragInt("AngleRange", &ViewState.m_AngleRange, 1, 1, 360));         //放出角度範囲
		CHECK(ImGui::InputInt("ParticleNum", &ViewState.m_ParticleNum, 5, 1000));        //パーティクルの個数
		CHECK(ImGui::InputFloat("Size", &ViewState.m_Size, 1.0f, 100.0f));               //粒子の大きさ
	}

	if (ImGui::CollapsingHeader("Time")) {
		CHECK(ImGui::InputFloat("Duaring", &ViewState.m_DuaringTime, 1, 10));            //周期
		CHECK(ImGui::InputFloat("StartDelayTime", &ViewState.m_StartDelayTime, 1, 10));  //開始遅延時間
		CHECK(ImGui::InputFloat("MaxLifeTime", &ViewState.m_MaxLifeTime, 1.0f, 10.0f));  //最大生存時間
	}

	if (ImGui::CollapsingHeader("Move")) {
		CHECK(ImGui::InputFloat("Speed", &ViewState.m_Speed, 1.0f, 100.0f));             //移動速度
		CHECK(ImGui::InputFloat("Accel", &ViewState.m_Accel, 0.1f, 10.0f));              //加速度
		if (ViewState.m_Accel != 0.0f) {
			CHECK(ImGui::DragFloat("minSpeed", &ViewState.m_MinSpeed));                  //最小速度
			CHECK(ImGui::DragFloat("maxSpeed", &ViewState.m_MaxSpeed));                  //最大速度
		}
		CHECK(ImGui::InputInt("RotateSpeed", &ViewState.m_RotateSpeed, 1, 100));         //回転速度
	}
	//GPUパーティクルOnOffチェックボックス
	if (ImGui::Checkbox("GPUParticle", &ViewState.isGPUParticle)){
		pParticleSystem_->ChangeGPUParticleMode(ViewState.isGPUParticle);
		pParticleSystem_->Start();
		CheckDataChange += 1;
	}
	
	CHECK(ImGui::Checkbox("isChaser", &ViewState.isChaser));                    //オブジェクトを追いかけるか

	if (ViewState.isChaser) {
		//#TODO 効果がわかりづらい
		CHECK(ImGui::SliderInt("minAngle", &ViewState.m_MinChaseAngle, 0, 360));//最小角度
		CHECK(ImGui::SliderInt("maxAngle", &ViewState.m_MaxChaseAngle, 0, 360));//最大角度
	}

	CHECK(ImGui::Checkbox("useGravity", &ViewState.UseGravity));                //重力利用
	if (ViewState.UseGravity) {
		CHECK(ImGui::InputFloat3("Gravity", ViewState.m_Gravity));              //重力加速度設定
	}

	CHECK(ImGui::Checkbox("Looping", &ViewState.isLooping));                    //ループさせるかどうか

	if (ImGui::Checkbox("isEmitter", &ViewState.isEmitting)) {                  //他のパーティクルからの発生に限らせるか
		pParticleSystem_->SetEmitte(ViewState.isEmitting);
		CheckDataChange += 1;
	}
	if (ImGui::Checkbox("isActive", &ViewState.isActive)) {                     //有効無効
		pParticleSystem_->SetActive(ViewState.isActive);
		pParticleSystem_->Start();
	}

	CHECK(ImGui::ColorEdit4("Color", ViewState.m_Color, 0));//色

	//監視したい設定項目はここより上の行に書く
	//監視結果-----------------------------------------------------------------------------
	//数値が変更されていたら反映
	if (CHECK_RESULT) {
		//更新
		pParticleSystem_->SetParticleSystemState(&ViewState);//パーティクルシステム設定構造体
		pParticleSystem_->UpdateSRV();                       //GPGPUで利用するシェーダーリソースビュー
	}
	//-----------------------------------------------------------------------------------


	//エミッター

	//エミッター対象パーティクルシステム名表示
	std::vector<int> NextSystemNumbers = pParticleSystem_->getNextSystemNumbers();
	if (NextSystemNumbers.size() < 1) {
		ImGui::Text("null");
	}
	else {
		for (int num = 0; num < NextSystemNumbers.size(); num++) {
			ImGui::Text(m_ParticleSystems.getParticleSystem()[NextSystemNumbers[num]]->getName());
		}
	}

	//エミッターの設定
	//すべてのパーティクルシステムのリストをボタンとして表示する
	if (ImGui::TreeNode("Emitter")) {

		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			if (ImGui::Button(iParticleSystem.second->getName())) {//ボタンが押されたら対象をエミッターに追加
				pParticleSystem_->SetNextParticleSystem(iParticleSystem.second->getSystemNumber());
			}
		}
		if (ImGui::Button("Null")) {
			pParticleSystem_->SetNextParticleSystem(-1);//-1を入れるとリスト初期化
		}
		ImGui::TreePop();
	}
	
	//ファイルアクセス関連-----------------------------------------------------------------------
	//テクスチャ関連
	{

		ImGui::Text(ViewState.m_TextureName);                      //テクスチャパス表示
		
		WIN32_FIND_DATA win32fd;
		HANDLE hFind;                                              //ファイル操作ハンドル

		std::string DirectoryName = ".\\InPutData\\*.png";
		if (ImGui::TreeNode("inputTexture")) {                     //Imguiのツリーが開かれたら
			hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//ファイルが存在するか確認
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (ImGui::Button(win32fd.cFileName)) {        //発見したファイルをボタンとして表示
						                                           //ボタンが押されたらそのファイルを読み込み反映
						pParticleSystem_->FInTex(win32fd.cFileName);
					}
				} while (FindNextFile(hFind, &win32fd));           //次のファイルを捜索
			}
			ImGui::TreePop();
			FindClose(hFind);

		}
	}
	//---------------------------------------------------------------------------------------
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 500, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	ImGui::Begin("ParticleOparate");

	//それぞれの処理を有効無効させる操作ウィンドウ
	ImGui::Checkbox("Update", pParticleSystem_->getisUpdateActive());
	ImGui::Checkbox("Draw", pParticleSystem_->getisDrawActive());
	if (ImGui::Button("ReStart")) {
		pParticleSystem_->Start();//再スタート(単体)
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

//警告表示
ParticleEditor::WARNING_REACTION ParticleEditor::ImGuiWarningText(const char* text) {
	WARNING_REACTION Reaction = WARNING_REACTION::DONT_PUSH;//返り値

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X / 2 - 75, SCREEN_Y / 2 - 40), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(150, 80), ImGuiSetCond_Once);

	ImGui::Begin("Watning");

	//表示
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

//全削除
void ParticleEditor::DeleteParticleSystems() {
	m_ViewParticleSystem = nullptr;
	m_ParticleSystems.DeleteParticleSystem();
}

//パーティクルシステム追加
void ParticleEditor::AddParticleSystem(t_ParticleSystemState* setState) {
	ParticleSystem* CreatedParticleSystem;
	if (setState != nullptr) {
		CreatedParticleSystem = m_ParticleSystems.AddParticleSystem(setState);//追加	
	}
	else {
		CreatedParticleSystem = m_ParticleSystems.AddParticleSystem();//追加
	}

	//名前設定
	std::string Name = "Particle" + std::to_string(m_ParticleSystems.getParticleSystemCount());
	CreatedParticleSystem->SetName(Name.c_str());
	
	//編集中にする
	m_ViewParticleSystem = CreatedParticleSystem;
}

bool ParticleEditor::InputData(const char* FileName_) {
	DeleteParticleSystems();//編集中のパーティクルシステムを削除

	//パーティクルシステムの情報をファイルから読み込む

	//カレントディレクトリを操作する
	TCHAR crDir[MAX_PATH + 1];
	t_ParticleSystemState GetState;


	GetCurrentDirectory(MAX_PATH + 1, crDir);//操作前のディレクトリを取得
	SetCurrentDirectory(".\\ParticleData");//読み込み先フォルダへカレントディレクトリを変更

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

void ParticleEditor::OutputData(char* FileName_) {
	//パーティクルシステムの情報をファイルへ書き出し
	if (m_ParticleSystems.getParticleSystemCount() == 0) {
		return;
	}
	//カレントディレクトリを操作する
	TCHAR crDir[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH + 1, crDir);//操作前のディレクトリを取得
	SetCurrentDirectory(".\\ParticleData");//書き出し先のフォルダへカレントディレクトリを変更

	//ファイル書き出し
	FILE *Fp;

	char ExtName[128];
	strcpy_s(ExtName, sizeof(ExtName), FileName_);
	strcat_s(ExtName, sizeof(ExtName), ".ptc");

	fopen_s(&Fp, ExtName, "wb");

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