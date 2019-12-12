#ifndef QUO
#define QUO

#define QUATERNION_IDENTITY float4(0, 0, 0, 1)
#define PI 3.14159265359f

//回転クォータニオン生成
float4 CreateRotateQt(float radian, float angleX, float angleY, float angleZ)
{
    float4 retQt;
    radian = radian % 360;
    float hRadian = 0.5f * radian;
    float sinRad = sin(hRadian);

    retQt.w = cos(hRadian);
    retQt.x = sinRad * angleX;
    retQt.y = sinRad * angleY;
    retQt.z = sinRad * angleZ;

    return retQt;
}

float4 CreateQtfromMatrix(matrix setMatrix)
{
    float4 retQt;

    float s;
    float tr = setMatrix._11 + setMatrix._22 + setMatrix._33 + 1.0f;
    if (tr >= 1.0f)
    {
        s = 0.5f / sqrt(tr);
        retQt.w = 0.25f / s;
        retQt.x = (setMatrix._23 - setMatrix._32) * s;
        retQt.y = (setMatrix._31 - setMatrix._13) * s;
        retQt.z = (setMatrix._12 - setMatrix._21) * s;

        return retQt;
    }
    else
    {
        float max;
        if (setMatrix._22 > setMatrix._33)
        {
            max = setMatrix._22;
        }
        else
        {
            max = setMatrix._33;
        }

        if (max < setMatrix._11)
        {
            s = sqrt(setMatrix._11 - (setMatrix._22 + setMatrix._33) + 1.0f);
            float x = s * 0.5f;
            s = 0.5f / s;
            retQt.x = x;
            retQt.y = (setMatrix._12 + setMatrix._21) * s;
            retQt.z = (setMatrix._31 + setMatrix._13) * s;
            retQt.w = (setMatrix._23 - setMatrix._32) * s;
            return retQt;
        }
        else
            if(max == setMatrix._22){
            s = sqrt(setMatrix._22 - (setMatrix._33 + setMatrix._11) + 1.0f);
            float y = s * 0.5f;

            s = 0.5f / s;
            retQt.x = (setMatrix._12 + setMatrix._21) * s;
            retQt.y = y;
            retQt.z = (setMatrix._23 + setMatrix._32) * s;
            retQt.w = (setMatrix._31 - setMatrix._13) * s;
            return retQt;
        }
        else
        {
            s = sqrt(setMatrix._33 - (setMatrix._11 + setMatrix._22) + 1.0f);
            float z = s * 0.5f;
            s = 0.5f / s;
            retQt.x = (setMatrix._31 + setMatrix._13) * s;
            retQt.y = (setMatrix._23 + setMatrix._32) * s;
            retQt.z = z;
            retQt.w = (setMatrix._12 - setMatrix._21) * s;
            return retQt;

        }
    }
        return retQt;
}

float4 RotationArc(float3 v0, float3 v1, float d)
{
    float3 Axis;
    float4 q;

    Axis = cross(v0, v1);

    float s = (float)sqrt((1 + d) * 2);

    if (s == 0.0f)
    {
        //ターゲット追い越し
        q = float4(0.0f, 0.0f, 0.0f, 1.0f);

    }
    else
    {
        q.x = Axis.x / s;
        q.y = Axis.y / s;
        q.z = Axis.z / s;
        q.w = s / 2;
    }

    return q;

}

float4 rotate_angle_axis(float angle, float3 axis)
{
    float sn = sin(angle * 0.5);
    float cs = cos(angle * 0.5);
    return float4(axis * sn, cs);
}

//クォータニオンの掛け算
float4 MulQuot(float4 Quot1, float4 Quot2)
{
    float4 retQt;

    retQt.w = Quot1.w * Quot2.w - Quot1.x * Quot2.x - Quot1.y * Quot2.y - Quot1.z * Quot2.z;
    retQt.x = Quot1.w * Quot2.x + Quot1.x * Quot2.w + Quot1.y * Quot2.z - Quot1.z * Quot2.y;
    retQt.y = Quot1.w * Quot2.y - Quot1.x * Quot2.z + Quot1.y * Quot2.w + Quot1.z * Quot2.x;
    retQt.z = Quot1.w * Quot2.z + Quot1.x * Quot2.y - Quot1.y * Quot2.x + Quot1.z * Quot2.w;

    return retQt;
}

float4 MultiplyQuot(float4 q1, float4 q2)
{
    return float4(
        q2.xyz * q1.w + q1.xyz * q2.w + cross(q1.xyz, q2.xyz),
        q1.w * q2.w - dot(q1.xyz, q2.xyz)
    );
}

//クォータニオンを行列に変換
matrix QttoMatrix(float4 Qt)
{
    matrix retMat;
    float x2, y2, z2;
    float xy, yz, zx;
    float wx, wy, wz;

    x2 = 2.0f * Qt.x * Qt.x;
    y2 = 2.0f * Qt.y * Qt.y;
    z2 = 2.0f * Qt.z * Qt.z;

    xy = 2.0f * Qt.x * Qt.y;
    yz = 2.0f * Qt.y * Qt.z;
    zx = 2.0f * Qt.z * Qt.x;

    wx = 2.0f * Qt.w * Qt.x;
    wy = 2.0f * Qt.w * Qt.y;
    wz = 2.0f * Qt.w * Qt.z;

    retMat._11 = 1.0f - y2 - z2;
    retMat._21 = xy - wz;
    retMat._31 = zx + wy;
    retMat._41 = 0.0f;

    retMat._12 = xy + wz;
    retMat._22 = 1.0f - z2 - x2;
    retMat._32 = yz - wx;
    retMat._42 = 0.0f;

    retMat._13 = zx - wy;
    retMat._23 = yz + wx;
    retMat._33 = 1.0f - x2 - y2;
    retMat._43 = 0.0f;

    retMat._14 = retMat._24 = retMat._34 = 0.0f;
    retMat._44 = 1.0f;

    return retMat;
}

//スラープ補間
float4 SlerpQt(float4 Qt1,float4 Qt2, float t)
{
    //二つのクォータニオンの長さが0なら終了
    if (length(Qt1) == 0.0)
    {
        if (length(Qt2) == 0.0)
        {
            return QUATERNION_IDENTITY;
        }
        return Qt2;

    }
    else if (length(Qt2) == 0.0)
    {
        return Qt1;
    }

    float cosHalfAngle = Qt1.w * Qt2.w + dot(Qt1.xyz, Qt2.xyz);

    if (cosHalfAngle >= 1.0 || cosHalfAngle <= -1.0)
    {
        return Qt1;
    }
    else if (cosHalfAngle < 0.0)
    {
        Qt2.xyz = -Qt2.xyz;
        Qt2.w = -Qt2.w;
        cosHalfAngle = -cosHalfAngle;
    }

    float blendA;
    float blendB;
    if (cosHalfAngle < 0.99)
    {
        //角度が360度より大きいなら補正する
        float halfAngle = acos(cosHalfAngle);
        float sinHalfAngle = sin(halfAngle);
        float oneOverSinHalfAngle = 1.0 / sinHalfAngle;
        blendA = sin(halfAngle * (1.0 - t)) * oneOverSinHalfAngle;
        blendB = sin(halfAngle * t) * oneOverSinHalfAngle;
    }
    else
    {
        blendA = 1.0 - t;
        blendB = t;
    }

    float4 result = float4(blendA * Qt1.xyz + blendB * Qt2.xyz, blendA * Qt1.w + blendB * Qt2.w);
    if (length(result) > 0.0)
    {
        return normalize(result);
    }
    return QUATERNION_IDENTITY;
}
#endif