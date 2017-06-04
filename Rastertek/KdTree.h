#pragma once

#include <string>
#include <d3d11.h>
#include <unordered_map>
#include <directxmath.h>
#include <SimpleMath.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>

class KdTree 
{
public:
	class Triangle
	{
	public:
		Triangle();

		inline DirectX::SimpleMath::Vector3 getBarycenter() const
		{
			return DirectX::SimpleMath::Vector3((vertices[0].x + vertices[1].x + vertices[2].x) / 3.0f, (vertices[0].y + vertices[1].y + vertices[2].y) / 3.0f, (vertices[0].z + vertices[1].z + vertices[2].z) / 3.0f);
		}

		void CalculateSmallest();
		void CalculateGreatest();

		DirectX::XMFLOAT3 vertices[3];
		DirectX::SimpleMath::Vector3 smallest, greatest;
		bool alreadyCut = false;
	};

	class MyBoundingBox : public DirectX::BoundingBox
	{
	public:
		static bool SmallestX(const Triangle* t1, const Triangle* t2);
		static bool SmallestY(const Triangle* t1, const Triangle* t2);
		static bool SmallestZ(const Triangle* t1, const Triangle* t2);
		static bool GreatestX(const Triangle* t1, const Triangle* t2);
		static bool GreatestY(const Triangle* t1, const Triangle* t2);
		static bool GreatestZ(const Triangle* t1, const Triangle* t2);

		MyBoundingBox(const std::vector<Triangle*>& tris);

		DirectX::SimpleMath::Vector3 smallest;
		DirectX::SimpleMath::Vector3 greatest;

		void Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, DirectX::XMVECTORF32 color);
		int GetLongestAxis() const;
	};

	struct RayHitStruct
	{
		Triangle* hitTriangle = nullptr;
		float hitDistance = 0.0f;
		DirectX::SimpleMath::Ray hitray;
		MyBoundingBox* hitBox = nullptr;
		DirectX::SimpleMath::Vector3 hitPoint = DirectX::SimpleMath::Vector3::Zero;
	};

	class KdNode
	{
	public:
		static bool SmallestX(const Triangle* t1, const Triangle* t2);
		static bool SmallestY(const Triangle* t1, const Triangle* t2);
		static bool SmallestZ(const Triangle* t1, const Triangle* t2);

		KdNode();
		static KdNode* build(std::vector<Triangle*>* tris, int depth);

		static bool hit(KdNode* node, const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, RayHitStruct& rayhit);
		static bool hitCheckAll(KdNode* node, const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, RayHitStruct& rayhit);

		void Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, DirectX::XMVECTORF32 color);

		MyBoundingBox* bbox;
		KdNode *left, *right;
		std::vector<Triangle*>* triangles;
	};

	bool hitCheckAll(const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, RayHitStruct& rayhit);
	bool hit(const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, RayHitStruct& rayhit);
	void MarkKDTreeDirty();
	void UpdateKDTree();
	void AddTriangles(const std::vector<Triangle*> newTriangles);
	void AddTriangle(Triangle* tri);
	void Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, DirectX::XMVECTORF32 color);
	void PurgeTriangles();

private:
	std::vector<Triangle*>* treeTriangles = new std::vector<Triangle*>();
	KdNode*	tree = nullptr;
	bool isDirty = false;
};
