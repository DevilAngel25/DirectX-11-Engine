#pragma once
#include "Model.h"

class GameObject
{
public:
	const XMVECTOR & GetPositionVector() const;
	const XMFLOAT3 & GetPositionFloat3() const;

	const XMVECTOR & GetRotationVector() const;
	const XMFLOAT3 & GetRotationFloat3() const;

	const XMVECTOR & GetScaleVector() const;
	const XMFLOAT3 & GetScaleFloat3() const;

	void SetPosition(const XMVECTOR & pos);
	void SetPosition(const XMFLOAT3 & pos);
	void SetPosition(float x, float y, float z);

	void AdjustPosition(const XMVECTOR & pos);
	void AdjustPosition(const XMFLOAT3 & pos);
	void AdjustPosition(float x, float y, float z);

	void SetRotation(const XMVECTOR & rot);
	void SetRotation(const XMFLOAT3 & rot);
	void SetRotation(float x, float y, float z);

	void AdjustRotation(const XMVECTOR & rot);
	void AdjustRotation(const XMFLOAT3 & rot);
	void AdjustRotation(float x, float y, float z);

	void SetScale(const XMVECTOR & scale);
	void SetScale(const XMFLOAT3 & scale);
	void SetScale(float x, float y, float z);

	void AdjustScale(const XMVECTOR & scale);
	void AdjustScale(const XMFLOAT3 & scale);
	void AdjustScale(float x, float y, float z);
	
protected:
	virtual void UpdateMatrix();

	XMVECTOR posVector;
	XMVECTOR rotVector;
	XMVECTOR scaleVector;

	XMFLOAT3 pos;
	XMFLOAT3 rot;
	XMFLOAT3 scale;
};