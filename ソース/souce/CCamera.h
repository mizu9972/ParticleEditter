#include	<directxmath.h>
#include	"Memory.h"
#include "CDirectInput.h"
#include <math.h>

#define MAX_ELEVATION -45
#define MIN_ELEVATION -135

#define MAX_RADIUS 300
#define MIN_RADIUS 10
using namespace DirectX;

class CCamera{
	CCamera(){
	}

	//角度をラジアンに変換
	inline float ToRad(float angle) {
		return (angle * DirectX::XM_PI) / 180.0f;
	}

	void MoveCamera() {
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_UP)) {
			m_Elevation += 1;
			m_Elevation = min(m_Elevation, MAX_ELEVATION);
		}
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_DOWN)) {
			m_Elevation -= 1;
			m_Elevation = max(m_Elevation, MIN_ELEVATION);
		}

		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RIGHT)) {
			m_Azimuth += 1;
		}
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_LEFT)) {
			m_Azimuth -= 1;
		}
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
			m_Radius -= 1;
			m_Radius = max(m_Radius, MIN_RADIUS);
		}
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
			m_Radius += 1;
			m_Radius = min(m_Radius, MAX_RADIUS);
		}
	}
public:
	CCamera(const CCamera&) = delete;
	CCamera& operator=(const CCamera&) = delete;
	CCamera(CCamera&&) = delete;
	CCamera& operator=(CCamera&&) = delete;

	static CCamera* GetInstance(){
		static CCamera instance;
		return &instance;
	}

	void Init(float nearclip, float farclip, float fov, 
		float width, float height,
		XMFLOAT3 eye, XMFLOAT3 lookat, XMFLOAT3 up){

		SetProjection(nearclip,farclip,fov,width,height);
		SetCamera(eye,lookat,up);
	}

	void SetNear(float nearclip){
		m_near = nearclip;
	}

	void SetFar(float farclip){
		m_far = farclip;
	}

	void SetFov(float fov){
		m_Fov = fov;
	}

	void SetAspect(float width,float height){
		m_Aspect = width/height;
	}

	void SetProjection(float nearclip,float farclip,float fov,float width,float height){
		SetNear(nearclip);
		SetFar(farclip);
		SetFov(fov);
		SetAspect(width,height);
		CreateProjectionMatrix();
	}
		
	void SetCamera(const XMFLOAT3& eye, const XMFLOAT3& lookat, const XMFLOAT3& up){

		SetEye(eye);
		SetLookat(lookat);
		SetUp(up);
		CreateCameraMatrix();
	}

	void SetEye(const XMFLOAT3& eye){
		m_Eye = eye;
	}

	void SetLookat(const XMFLOAT3& lookat){
		m_Lookat = lookat;
	}

	void SetUp(const XMFLOAT3& up){
		m_Up = up;
	}

	void SetRadius(float radius) {
		m_Radius = radius;
	}

	void SetElevation(float elevation) {
		m_Elevation = elevation;
	}

	void SetAzimuth(float azimuth) {
		m_Azimuth = azimuth;
	}

	void CreateCameraMatrix(){
		ALIGN16 XMVECTOR Eye = XMVectorSet(m_Eye.x, m_Eye.y, m_Eye.z, 0.0f);
		ALIGN16 XMVECTOR At = XMVectorSet(m_Lookat.x, m_Lookat.y, m_Lookat.z, 0.0f);
		ALIGN16 XMVECTOR Up = XMVectorSet(m_Up.x, m_Up.y, m_Up.z, 0.0f);

		ALIGN16 XMMATRIX camera;
		camera = XMMatrixLookAtLH(Eye, At, Up);

		XMStoreFloat4x4(&m_Camera, camera);
	}

	void CreateProjectionMatrix(){

		ALIGN16 XMMATRIX projection;

		projection = XMMatrixPerspectiveFovLH(m_Fov, m_Aspect, m_near, m_far);

		XMStoreFloat4x4(&m_Projection, projection);

	}

	void Cross(const float* a, const float* b, float* r)
	{
		r[0] = a[1] * b[2] - a[2] * b[1];
		r[1] = a[2] * b[0] - a[0] * b[2];
		r[2] = a[0] * b[1] - a[1] * b[0];
	}

	float Dot(const float* a, const float* b)
	{
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	}

	void Normalize(const float* a, float *r)
	{
		float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
		r[0] = a[0] * il;
		r[1] = a[1] * il;
		r[2] = a[2] * il;
	}


	void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float *m16)
	{
		float temp, temp2, temp3, temp4;
		temp = 2.0f * znear;
		temp2 = left - right;
		temp3 = top - bottom;
		temp4 = zfar - znear;
		m16[0] = temp / temp2;					m16[1] = 0.0;						m16[2] = 0.0;						m16[3] = 0.0;
		m16[4] = 0.0;							m16[5] = temp / temp3;				m16[6] = 0.0;						m16[7] = 0.0;
		m16[8] = (right + left) / temp2;		m16[9] = (top + bottom) / temp3;	m16[10] = (-zfar - znear) / temp4;	m16[11] = -1.0f;
		m16[12] = 0.0;							m16[13] = 0.0;						m16[14] = (-temp * zfar) / temp4;	m16[15] = 0.0;
	}

	void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float *m16)
	{
		float ymax, xmax;
		ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
		xmax = ymax * aspectRatio;
		Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
	}

	void CreateView(float *m16) {
		float X[3], Y[3], Z[3], tmp[3];

		float eye[3] = { m_Eye.x,m_Eye.y,m_Eye.z };
		float at[3]  = { m_Lookat.x,m_Lookat.y,m_Lookat.z };
		float up[3]  = { m_Up.x,m_Up.y,m_Up.z };

		tmp[0] = eye[0] - at[0];
		tmp[1] = eye[1] - at[1];
		tmp[2] = eye[2] - at[2];
		//Z.normalize(eye - at);
		Normalize(tmp, Z);
		Normalize(up, Y);
		//Y.normalize(up);

		Cross(Y, Z, tmp);
		//tmp.cross(Y, Z);
		Normalize(tmp, X);
		//X.normalize(tmp);

		Cross(Z, X, tmp);
		//tmp.cross(Z, X);
		Normalize(tmp, Y);
		//Y.normalize(tmp);

		m16[0]  = X[2];			m16[1]  = Y[2];			m16[2]  = Z[2];			m16[3]  = 0.0f;
		m16[4]  = X[1];			m16[5]  = Y[1];			m16[6]  = Z[1];			m16[7]  = 0.0f;
		m16[8]  = X[0];			m16[9]  = Y[0];			m16[10] = Z[0];			m16[11] = 0.0f;
		m16[12] = -Dot(X, eye);	m16[13] = -Dot(Y, eye);	m16[14] = -Dot(Z, eye);	m16[15] = 1.0f;
	}

	void UpdateSphereCamera() {
		MoveCamera();
		DirectX::XMFLOAT3 eye;

		float Elevation;
		float Azimuth;
		//仰角、方位角をラジアンに変換
		Elevation = ToRad(m_Elevation);
		Azimuth = ToRad(m_Azimuth);

		//仰角に応じた半径を計算
		float r = m_Radius + sinf(Elevation);

		//視点座標を計算
		eye.x = r * cosf(Azimuth);
		eye.y = m_Radius * cosf(Elevation);
		eye.z = r * sinf(Azimuth);

		//上向きベクトルを計算(仰角を90度回転させるとできる)
		DirectX::XMFLOAT3 up;
		float elevationplus90 = Elevation + ToRad(90.0f);

		r = m_Radius * sinf(elevationplus90);

		up.x = r * cosf(Azimuth);
		up.y = m_Radius * cosf(elevationplus90);
		up.z = r * sinf(Azimuth);
		//視点ベクトルを注視点分平行移動
		eye.x += m_Lookat.x;
		eye.y += m_Lookat.y;
		eye.z += m_Lookat.z;

		//ビュー変換行列を作成
		CCamera::GetInstance()->SetCamera(eye, m_Lookat, up);
	}
	


	const XMFLOAT4X4& GetCameraMatrix(){
		return m_Camera;
	}

	const XMFLOAT4X4& GetProjectionMatrix(){
		return m_Projection;
	}

	float* GetProjectionMatrixf16() {
		float retMat[16] = {
			m_Projection._11,m_Projection._12,m_Projection._13,m_Projection._14,
			m_Projection._21,m_Projection._22,m_Projection._23,m_Projection._24,
			m_Projection._31,m_Projection._32,m_Projection._33,m_Projection._34,
			m_Projection._41,m_Projection._42,m_Projection._43,m_Projection._44
		};
		return retMat;
	}


	float GetFov() const {
		return m_Fov;
	}

	const XMFLOAT3& GetEye() const {
		return m_Eye;
	}

	const XMFLOAT3& GetLookat() const {
		return m_Lookat;
	}

	const XMFLOAT3& GetUp() const {
		return m_Up;
	}

	const float GetElevation() const {
		return m_Elevation;
	}

	const float GetAzimuth()const {
		return m_Azimuth;
	}

private:
	XMFLOAT4X4		m_Projection;

	XMFLOAT4X4		m_Camera;

	XMFLOAT3		m_Eye;				// カメラ位置
	XMFLOAT3		m_Lookat;			// 注視点
	XMFLOAT3		m_Up;				// 上向きベクトル

	float			m_Radius;//注視点からの距離
	float           m_Elevation = -90;//仰角
	float           m_Azimuth   = 0;//回転角

	float			m_near;
	float			m_Aspect;
	float			m_Fov;
	float			m_far;
};