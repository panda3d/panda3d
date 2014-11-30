/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_SensorFilter.cpp
Content     :   Basic filtering of sensor data
Created     :   March 7, 2013
Authors     :   Steve LaValle, Anna Yershova

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_SensorFilter.h"

namespace OVR {

Vector3f SensorFilter::Total() const
{
    Vector3f total = Vector3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < Size; i++)
        total += Elements[i];
    return total;
}

Vector3f SensorFilter::Mean() const
{
    Vector3f total = Vector3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < Size; i++)
        total += Elements[i];
    return total / (float) Size;
}

Vector3f SensorFilter::Median() const
{
    int half_window = (int) Size / 2;
    float sortx[MaxFilterSize];
    float resultx = 0.0f;

    float sorty[MaxFilterSize];
    float resulty = 0.0f;

    float sortz[MaxFilterSize];
    float resultz = 0.0f;

    for (int i = 0; i < Size; i++) 
    {
        sortx[i] = Elements[i].x;
        sorty[i] = Elements[i].y;
        sortz[i] = Elements[i].z;
    }
    for (int j = 0; j <= half_window; j++) 
    {
        int minx = j;
        int miny = j;
        int minz = j;
        for (int k = j + 1; k < Size; k++) 
        {
            if (sortx[k] < sortx[minx]) minx = k;
            if (sorty[k] < sorty[miny]) miny = k;
            if (sortz[k] < sortz[minz]) minz = k;
        }
        const float tempx = sortx[j];
        const float tempy = sorty[j];
        const float tempz = sortz[j];
        sortx[j] = sortx[minx];
        sortx[minx] = tempx;

        sorty[j] = sorty[miny];
        sorty[miny] = tempy;

        sortz[j] = sortz[minz];
        sortz[minz] = tempz;
    }
    resultx = sortx[half_window];
    resulty = sorty[half_window];
    resultz = sortz[half_window];

    return Vector3f(resultx, resulty, resultz);
}

//  Only the diagonal of the covariance matrix.
Vector3f SensorFilter::Variance() const
{
    Vector3f mean = Mean();
    Vector3f total = Vector3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < Size; i++) 
    {
        total.x += (Elements[i].x - mean.x) * (Elements[i].x - mean.x);
        total.y += (Elements[i].y - mean.y) * (Elements[i].y - mean.y);
        total.z += (Elements[i].z - mean.z) * (Elements[i].z - mean.z);
    }
    return total / (float) Size;
}

// Should be a 3x3 matrix returned, but OVR_math.h doesn't have one
Matrix4f SensorFilter::Covariance() const
{
    Vector3f mean = Mean();
    Matrix4f total = Matrix4f(0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);
    for (int i = 0; i < Size; i++) 
    {
        total.M[0][0] += (Elements[i].x - mean.x) * (Elements[i].x - mean.x);
        total.M[1][0] += (Elements[i].y - mean.y) * (Elements[i].x - mean.x);
        total.M[2][0] += (Elements[i].z - mean.z) * (Elements[i].x - mean.x);
        total.M[1][1] += (Elements[i].y - mean.y) * (Elements[i].y - mean.y);
        total.M[2][1] += (Elements[i].z - mean.z) * (Elements[i].y - mean.y);
        total.M[2][2] += (Elements[i].z - mean.z) * (Elements[i].z - mean.z);
    }
    total.M[0][1] = total.M[1][0];
    total.M[0][2] = total.M[2][0];
    total.M[1][2] = total.M[2][1];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            total.M[i][j] *= 1.0f / Size;
    return total;
}

Vector3f SensorFilter::PearsonCoefficient() const
{
    Matrix4f cov = Covariance();
    Vector3f pearson = Vector3f();
    pearson.x = cov.M[0][1]/(sqrt(cov.M[0][0])*sqrt(cov.M[1][1]));
    pearson.y = cov.M[1][2]/(sqrt(cov.M[1][1])*sqrt(cov.M[2][2]));
    pearson.z = cov.M[2][0]/(sqrt(cov.M[2][2])*sqrt(cov.M[0][0]));

    return pearson;
}


Vector3f SensorFilter::SavitzkyGolaySmooth8() const
{
    OVR_ASSERT(Size >= 8);
    return GetPrev(0)*0.41667f +
            GetPrev(1)*0.33333f +
            GetPrev(2)*0.25f +
            GetPrev(3)*0.16667f +
            GetPrev(4)*0.08333f -
            GetPrev(6)*0.08333f -
            GetPrev(7)*0.16667f;
}


Vector3f SensorFilter::SavitzkyGolayDerivative4() const
{
    OVR_ASSERT(Size >= 4);
    return GetPrev(0)*0.3f +
            GetPrev(1)*0.1f -
            GetPrev(2)*0.1f -
            GetPrev(3)*0.3f;
}

Vector3f SensorFilter::SavitzkyGolayDerivative5() const
{
    OVR_ASSERT(Size >= 5);
    return GetPrev(0)*0.2f +
            GetPrev(1)*0.1f -
            GetPrev(3)*0.1f -
            GetPrev(4)*0.2f;
}

Vector3f SensorFilter::SavitzkyGolayDerivative12() const
{
    OVR_ASSERT(Size >= 12);
    return GetPrev(0)*0.03846f +
            GetPrev(1)*0.03147f +
            GetPrev(2)*0.02448f +
            GetPrev(3)*0.01748f +
            GetPrev(4)*0.01049f +
            GetPrev(5)*0.0035f -
            GetPrev(6)*0.0035f -
            GetPrev(7)*0.01049f -
            GetPrev(8)*0.01748f -
            GetPrev(9)*0.02448f -
            GetPrev(10)*0.03147f -
            GetPrev(11)*0.03846f;
}

Vector3f SensorFilter::SavitzkyGolayDerivativeN(int n) const
{    
    OVR_ASSERT(Size >= n);
    int m = (n-1)/2;
    Vector3f result = Vector3f();
    for (int k = 1; k <= m; k++) 
    {
        int ind1 = m - k;
        int ind2 = n - m + k - 1;
        result += (GetPrev(ind1) - GetPrev(ind2)) * (float) k;
    }
    float coef = 3.0f/(m*(m+1.0f)*(2.0f*m+1.0f));
    result = result*coef;
    return result;
}




} //namespace OVR