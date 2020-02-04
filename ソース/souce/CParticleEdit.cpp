#include <list>
#include <vector>
#include <string>
#include "ImGui/ImGuizmoUtil.h"
#include "CParticleEdit.h"
#include "CCamera.h"
#include "dx11mathutil.h"
#include "DX11Settransform.h"

#include "Shader.h"

#include "game.h"

#define CHECK(x) (CheckDataChange += x) //数値変更を感知
#define CHECK_RESULT (CheckDataChange > 0) //変更されたかどうか判定
static int CheckDataChange = 0;//ImGuiで数値が変更されたらtrueが返ってくるメソッドの仕様を利用して、数値変更を管理する変数

extern int g_nCountFPS;//FPSカウンター

//初期化
void ParticleEditor::Init(unsigned int Width, unsigned int Height, ID3D11Device* device, ID3D11DeviceContext* devicecontext) {
	
	//ホーミングターゲット初期化
	m_TargetBillBoard.Init(CDirectXGraphics::GetInstance()->GetDXDevice(), CDirectXGraphics::GetInstance()->GetImmediateContext(),
		m_TargetPosf[0], m_TargetPosf[1], m_TargetPosf[2],
		30, 30,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		"shader/psParticle.fx",
		"shader/vsPrticle.fx"
	);
	strcpy_s(m_FileName, sizeof(m_FileName), "ParticleState");

	m_ParticleSystems.Init(device,devicecontext,CDirectXGraphics::GetInstance()->GetDepthStencilView(),CDirectXGraphics::GetInstance()->GetRenderTargetView(),CDirectXGraphics::GetInstance()->GetSwapChain()/*Width, Height, device, devicecontext, CDirectXGraphics::GetInstance()->GetDepthStencilView(), CDirectXGraphics::GetInstance()->GetRenderTargetView()*/);

	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };

	//スカイボックス初期化
	DX11MtxIdentity(m_SkyboxMatrix);//行列初期化
	DX11MtxIdentity(m_CubeMat);

	//スカイボックスを初期化してmapに登録する処理
	auto SkyboxInit = [this](const char* Keyname,const char* Filename,const char* VertexFilename,const char* PixelFilename) {
		m_SkyBoxes.emplace(Keyname, new CModel());//mapに追加
		m_SkyBoxes[Keyname]->Init(Filename, VertexFilename, PixelFilename);//初期化
	};
	m_Cube = new CModel();
	m_Cube->Init("assets/f1.x.dat", "shader/vs.fx", "shader/ps.fx");

	//スカイボックス初期化
	SkyboxInit("Skydome", "assets/skydome.x.dat", "shader/vs.fx", "shader/psskydome.fx");
	SkyboxInit("DarkSky", "assets/darkskydome.x.dat", "shader/vs.fx", "shader/psskydome.fx");

	//表示用スカイボックス設定
	m_ViewSkybox = m_SkyBoxes.at("Skydome");
}

//終了処理
void ParticleEditor::UnInit() {
	if (m_ViewParticleSystem != nullptr) {
		m_ViewParticleSystem->UnInit();
		m_ViewParticleSystem = nullptr;
	}

	m_ParticleSystems.UnInit();

	//ターゲット終了
	m_Cube->Uninit();
	delete m_Cube;
	m_Cube = nullptr;

	for (auto iSkyboxes : m_SkyBoxes) {
		iSkyboxes.second->Uninit();
		delete iSkyboxes.second;
	}
	m_SkyBoxes.clear();
}

//更新
void ParticleEditor::Update() {

	//ターゲット座標反映
	m_CubeMat._41 = m_TargetPosf[0];
	m_CubeMat._42 = m_TargetPosf[1];
	m_CubeMat._43 = m_TargetPosf[2];
	//ImGui表示
	ImVec4 clear_color = ImVec4(0.25f, 0.5f, 0.35f, 1.00f);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (isActive == true) {
	m_ParticleSystems.Update();//パーティクルシステム更新
	}

	//パーティクルシステムが存在しないなら表示用データリセット
	if (m_ParticleSystems.getParticleSystemCount() <= 0) {
		m_ViewParticleSystem = nullptr;
	}
}
//描画
void ParticleEditor::Draw() {

	// ワールド変換行列
	DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::WORLD, m_SkyboxMatrix);
	m_ViewSkybox->Draw();

	if (isDrawTargetObj) {

		DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::WORLD, m_CubeMat);
		DX11MtxScale(2.0f, 2.0f, 2.0f, m_CubeMat);

		m_Cube->Draw();
	}

	m_ParticleSystems.Draw(CCamera::GetInstance()->GetCameraMatrix());


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

	//スカイボックス設定
	if (ImGui::TreeNode("Skybox")) {
		for (auto iSkyboxes : m_SkyBoxes) {
			if (ImGui::Button(iSkyboxes.first)) {
				m_ViewSkybox = iSkyboxes.second;
			}
		}
		ImGui::TreePop();
	}

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

	//ImGuizmo
	ImGuizmo::BeginFrame();
	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 300, 0), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, SCREEN_Y), ImGuiSetCond_Once);

	char WindowName[256] = "ParticleSystem : ";
	strcat_s(WindowName, sizeof(WindowName), pParticleSystem_->getName());
	ImGui::Begin(WindowName, nullptr, ImGuiWindowFlags_MenuBar);

	CheckDataChange = 0;//数値変更監視変数初期化

	EditTransform(&ViewState);

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
		CHECK(ImGui::Checkbox("Looping", &ViewState.isLooping));                    //ループさせるかどうか
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

	
	if (ImGui::CollapsingHeader("TargetChase")) {
		CHECK(ImGui::Checkbox("UseChaser", &ViewState.isChaser));                    //オブジェクトを追いかけるか

		if (ViewState.isChaser) {
			//#TODO 効果がわかりづらい
			CHECK(ImGui::SliderInt("minAngle", &ViewState.m_MinChaseAngle, 0, 360));//最小角度
			CHECK(ImGui::SliderInt("maxAngle", &ViewState.m_MaxChaseAngle, 0, 360));//最大角度
		}
	}

	if (ImGui::CollapsingHeader("Gravity")) {
		CHECK(ImGui::Checkbox("useGravity", &ViewState.UseGravity));                //重力利用
		if (ViewState.UseGravity) {
			CHECK(ImGui::InputFloat3("Gravity", ViewState.m_Gravity));              //重力加速度設定
		}
	}

	if (ImGui::CollapsingHeader("Option")) {
		//GPUパーティクルOnOffチェックボックス
		if (ImGui::Checkbox("GPUParticle", &ViewState.isGPUParticle)) {
			pParticleSystem_->ChangeGPUParticleMode(ViewState.isGPUParticle);
			pParticleSystem_->Start();
			CheckDataChange += 1;
		}
		if (ImGui::Checkbox("isEmitter", &ViewState.isEmitting)) {                  //他のパーティクルからの発生に限らせるか
			pParticleSystem_->SetEmitte(ViewState.isEmitting);
			CheckDataChange += 1;
		}
		if (ImGui::Checkbox("isActive", &ViewState.isActive)) {                     //有効無効
			pParticleSystem_->SetActive(ViewState.isActive);
			pParticleSystem_->Start();
		}
	}

	if (ImGui::CollapsingHeader("SoftParticle")) {

		if (ImGui::Checkbox("UseSoftParticle", &ViewState.isSoftParticle)) {
			pParticleSystem_->ChangeSoftParticleMode(ViewState.isSoftParticle);
		}
		if (ViewState.isSoftParticle) {
			int ChangeCBSPFlag = 0;
			ConstantBufferSoftParticle setCb = pParticleSystem_->getCBSoftParticleState();
			ChangeCBSPFlag += ImGui::DragFloat("FeedFar", &setCb.iZfar, 0.01f, 0.0f, 10.0f);
			ChangeCBSPFlag += ImGui::DragFloat("FeedVolume", &setCb.iZVolume, 0.1f, 0.0f, 150.0f);
			if (ChangeCBSPFlag > 0) {
				pParticleSystem_->SetSoftPConstantBuffer(&setCb);
			}
		}
	}
	//エミッター

	//連続して発生させるパーティクルシステムの設定
	//すべてのパーティクルシステムのリストをボタンとして表示する
	if (ImGui::CollapsingHeader("NextParticle")) {

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

		for (auto iParticleSystem : m_ParticleSystems.getParticleSystem()) {
			if (ImGui::Button(iParticleSystem.second->getName())) {//ボタンが押されたら対象をエミッターに追加
				pParticleSystem_->SetNextParticleSystem(iParticleSystem.second->getSystemNumber());
			}
		}
		if (ImGui::Button("Null")) {
			pParticleSystem_->SetNextParticleSystem(-1);//-1を入れるとリスト初期化
		}
		
	}
	
	//ファイルアクセス関連-----------------------------------------------------------------------
	//テクスチャ関連
	{

		
		WIN32_FIND_DATA win32fd;
		HANDLE hFind;                                              //ファイル操作ハンドル

		std::string DirectoryName = ".\\InPutData\\*.png";
		if (ImGui::CollapsingHeader("Texture")) {
			ImGui::Text("-");
			ImGui::SameLine();
			ImGui::Text(ViewState.m_TextureName);                      //テクスチャパス表示
			hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//ファイルが存在するか確認
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (ImGui::Button(win32fd.cFileName)) {        //発見したファイルをボタンとして表示
						                                           //ボタンが押されたらそのファイルを読み込み反映
						pParticleSystem_->FInTex(win32fd.cFileName);
					}
				} while (FindNextFile(hFind, &win32fd));           //次のファイルを捜索
			}
			FindClose(hFind);

		}
	}
	//---------------------------------------------------------------------------------------


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


	ImGui::End();

	ImGuizmo::ViewManipulate(cameraView, 8.0f, ImVec2(200, 0), ImVec2(128, 128), 0x101010);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 500, SCREEN_Y - 100), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Once);

	ImGui::Begin("ParticleOparate");
	{
		bool isMesodActice;
		isMesodActice = pParticleSystem_->getisUpdateActive();
		//それぞれの処理を有効無効させる操作ウィンドウ
		if (ImGui::Checkbox("Update", &isMesodActice)) {
			pParticleSystem_->SetisUpdateActive(isMesodActice);
		}
		isMesodActice = pParticleSystem_->getisDrawActive();
		if (ImGui::Checkbox("Draw", &isMesodActice)) {
			pParticleSystem_->SetisDrawActive(isMesodActice);
		}

		if (ImGui::Button("ReStart")) {
			pParticleSystem_->Start();//再スタート(単体)
		}
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
		fread(&ParticleCount, sizeof(int), 1, Fp);//read1 //パーティクルシステム個数
		std::vector<int> nextNumbers;
		int NumbersSize;
		for (int Count = 0; Count < ParticleCount; Count++) {
			//--------------------------------------------------------------------------------------------------------------
			//パーティクルシステム一つ分
			//データを読み込んで追加
			fread(&GetState, sizeof(t_ParticleSystemState), 1, Fp);//read2 //構造体データ
			
			fread(&NumbersSize, sizeof(int), 1, Fp);//read3 要素数読み込み

			nextNumbers.clear();
			for (int num = 0; num < NumbersSize; num++) {
				int setNumber;
				fread(&setNumber, sizeof(int), 1, Fp);//read4 要素読み込み
				nextNumbers.emplace_back(setNumber);
			}

			m_ParticleSystems.AddParticleSystem(&GetState,nextNumbers);

			//--------------------------------------------------------------------------------------------------------------
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
		//--------------------------------------------------------------------------------------------------------------
		//パーティクルシステム一つ分
		fwrite(&iParticleSystem.second->GetState(), sizeof(t_ParticleSystemState), 1, Fp);//write2 //構造体データ書き出し

		//パーティクル番号リスト出力
		nextNumbers = iParticleSystem.second->getNextSystemNumbers();
		NumbersSize = static_cast<int>(nextNumbers.size());
		fwrite(&NumbersSize, sizeof(int), 1, Fp);//write3 //要素数書き出し
		for (int num = 0; num < NumbersSize; num++) {
			fwrite(&nextNumbers[num], sizeof(int), 1, Fp);//write4 //要素を配列で書き出し
		}

		//--------------------------------------------------------------------------------------------------------------
	}
	
	fclose(Fp);

	SetCurrentDirectory(crDir);//カレントディレクトリを元に戻す

}

void ParticleEditor::EditTransform(t_ParticleSystemState* ViewState) {
	ImGuiIO& io = ImGui::GetIO();
	float fov = 27.0f;

	float objectMatrix[16] =
	{ 1.f, 0.f, 0.f, 0.f,
	  0.f, 1.f, 0.f, 0.f,
	  0.f, 0.f, 1.f, 0.f,
	  0.f, 0.f, 0.f, 1.f };

	ImGuizmo::Enable(true);

	//カメラ行列更新
	CCamera::GetInstance()->Perspective(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 100.f, cameraProjection);
	CCamera::GetInstance()->CreateView(cameraView);


	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);//操作モード
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);//座標系

	static bool useSnap = false;
	static float snap[3] = { 1.f, 1.f, 1.f };
	static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
	static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
	static bool boundSizing = false;
	static bool boundSizingSnap = false;

	float matrixTranslation[3];
	float matrixRotation[3];
	float matrixScale[3] = { 1.0f,1.0f,1.0f };


	matrixTranslation[0] = ViewState->m_Position[2] / 2.0f;
	matrixTranslation[1] = ViewState->m_Position[1] / 2.0f;
	matrixTranslation[2] = -1 * ViewState->m_Position[0] / 2.0f;

	matrixRotation[0] = static_cast<float>(ViewState->m_Angle[0]);
	matrixRotation[1] = static_cast<float>(ViewState->m_Angle[1]);
	matrixRotation[2] = static_cast<float>(ViewState->m_Angle[2]);


	//各要素編集
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, objectMatrix);

	if (ImGui::IsKeyPressed(83)) {
		useSnap = !useSnap;
	}

	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);//画面情報をセット

	//マニピュレータ標示
	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, objectMatrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
	ImGuizmo::DecomposeMatrixToComponents(objectMatrix, matrixTranslation, matrixRotation, matrixScale);

	//数値反映
	if (ViewState->m_Position[0] != -1 * matrixTranslation[2] * 2.0f) {
		ViewState->m_Position[0] = -1 * matrixTranslation[2] * 2.0f;
		CheckDataChange += 1;
	}
	if (ViewState->m_Position[1] != matrixTranslation[1] * 2.0f) {
		ViewState->m_Position[1] = matrixTranslation[1] * 2.0f;
		CheckDataChange += 1;
	}
	if (ViewState->m_Position[2] != matrixTranslation[0] * 2.0f) {
		ViewState->m_Position[2] = matrixTranslation[0] * 2.0f;
		CheckDataChange += 1;
	}
}