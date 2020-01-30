#pragma once

void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float *m16);
void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float *m16);