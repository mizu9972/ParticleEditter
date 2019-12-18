#include <math.h>
#include "imconfig.h"
//#include "ImGuizmo.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float *m16)
{
	float temp, temp2, temp3, temp4;
	temp = 2.0f * znear;
	temp2 = right - left;
	temp3 = top - bottom;
	temp4 = zfar - znear;
	m16[0] = temp / temp2;
	m16[1] = 0.0;
	m16[2] = 0.0;
	m16[3] = 0.0;
	m16[4] = 0.0;
	m16[5] = temp / temp3;
	m16[6] = 0.0;
	m16[7] = 0.0;
	m16[8] = (right + left) / temp2;
	m16[9] = (top + bottom) / temp3;
	m16[10] = (-zfar - znear) / temp4;
	m16[11] = -1.0f;
	m16[12] = 0.0;
	m16[13] = 0.0;
	m16[14] = (-temp * zfar) / temp4;
	m16[15] = 0.0;
}

void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float *m16)
{
	float ymax, xmax;
	ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
	xmax = ymax * aspectRatio;
	Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
}

//void EditTransform(const float *cameraView, float *cameraProjection, float* matrix)
//{
//	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
//	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
//	static bool useSnap = false;
//	static float snap[3] = { 1.f, 1.f, 1.f };
//	static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
//	static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
//	static bool boundSizing = false;
//	static bool boundSizingSnap = false;
//
//	if (ImGui::IsKeyPressed(90))
//		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
//	if (ImGui::IsKeyPressed(69))
//		mCurrentGizmoOperation = ImGuizmo::ROTATE;
//	if (ImGui::IsKeyPressed(82)) // r Key
//		mCurrentGizmoOperation = ImGuizmo::SCALE;
//	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
//		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
//	ImGui::SameLine();
//	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
//		mCurrentGizmoOperation = ImGuizmo::ROTATE;
//	ImGui::SameLine();
//	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
//		mCurrentGizmoOperation = ImGuizmo::SCALE;
//	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
//	ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
//	ImGui::InputFloat3("Tr", matrixTranslation, 3);
//	ImGui::InputFloat3("Rt", matrixRotation, 3);
//	ImGui::InputFloat3("Sc", matrixScale, 3);
//	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);
//
//	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
//	{
//		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
//			mCurrentGizmoMode = ImGuizmo::LOCAL;
//		ImGui::SameLine();
//		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
//			mCurrentGizmoMode = ImGuizmo::WORLD;
//	}
//	if (ImGui::IsKeyPressed(83))
//		useSnap = !useSnap;
//	ImGui::Checkbox("", &useSnap);
//	ImGui::SameLine();
//
//	switch (mCurrentGizmoOperation)
//	{
//	case ImGuizmo::TRANSLATE:
//		ImGui::InputFloat3("Snap", &snap[0]);
//		break;
//	case ImGuizmo::ROTATE:
//		ImGui::InputFloat("Angle Snap", &snap[0]);
//		break;
//	case ImGuizmo::SCALE:
//		ImGui::InputFloat("Scale Snap", &snap[0]);
//		break;
//	}
//	ImGui::Checkbox("Bound Sizing", &boundSizing);
//	if (boundSizing)
//	{
//		ImGui::PushID(3);
//		ImGui::Checkbox("", &boundSizingSnap);
//		ImGui::SameLine();
//		ImGui::InputFloat3("Snap", boundsSnap);
//		ImGui::PopID();
//	}
//
//	ImGuiIO& io = ImGui::GetIO();
//	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
//	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
//}