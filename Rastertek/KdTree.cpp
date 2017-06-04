#include "KdTree.h"
#include <iostream>
#include <algorithm>

KdTree::Triangle::Triangle()
{
}

void KdTree::Triangle::CalculateSmallest()
{
	smallest.x = std::min<float>({vertices[0].x,vertices[1].x,vertices[2].x});
	smallest.y = std::min<float>({vertices[0].y,vertices[1].y,vertices[2].y});
	smallest.z = std::min<float>({vertices[0].z,vertices[1].z,vertices[2].z});
}

void KdTree::Triangle::CalculateGreatest()
{
	greatest.x = std::max<float>({vertices[0].x, vertices[1].x, vertices[2].x});
	greatest.y = std::max<float>({vertices[0].y,vertices[1].y,vertices[2].y});
	greatest.z = std::max<float>({vertices[0].z, vertices[1].z, vertices[2].z});
}

void KdTree::AddTriangles(const std::vector<Triangle*> newTriangles)
{
	treeTriangles->insert(treeTriangles->end(), newTriangles.begin(), newTriangles.end());
	std::cout << treeTriangles->size() << std::endl;
}

void KdTree::AddTriangle(Triangle* tri)
{
	treeTriangles->push_back(tri);
}

bool KdTree::hitCheckAll(const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, RayHitStruct& rayhit)
{
	if(tree)
	{
		return tree->hitCheckAll(tree, ray, t, tmin, rayhit);
	}

	return false;
}

bool KdTree::hit(const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, RayHitStruct& rayhit)
{
	if (tree)
	{
		return tree->hit(tree, ray, t, tmin, rayhit);
	}

	return false;
}

void KdTree::MarkKDTreeDirty()
{
	isDirty = true;
}

void KdTree::UpdateKDTree()
{
	if(isDirty)
	{
		printf("Updating KD-Tree\n\r");
		delete tree;
		tree = KdNode::build(treeTriangles, 0);
		isDirty = false;
	}
}

bool KdTree::MyBoundingBox::SmallestX(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float small1 = std::min<float>({ t1->vertices[0].x,t1->vertices[1].x,t1->vertices[2].x });
	float small2 = std::min<float>({ t2->vertices[0].x,t2->vertices[1].x,t2->vertices[2].x });

	return small1 < small2;
}

bool KdTree::MyBoundingBox::SmallestY(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float small1 = std::min<float>({ t1->vertices[0].y,t1->vertices[1].y,t1->vertices[2].y });
	float small2 = std::min<float>({ t2->vertices[0].y,t2->vertices[1].y,t2->vertices[2].y });

	return small1 < small2;
}

bool KdTree::MyBoundingBox::SmallestZ(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float small1 = std::min<float>({ t1->vertices[0].z,t1->vertices[1].z,t1->vertices[2].z });
	float small2 = std::min<float>({ t2->vertices[0].z,t2->vertices[1].z,t2->vertices[2].z });

	return small1 < small2;
}

bool KdTree::MyBoundingBox::GreatestX(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float great1 = std::max<float>({ t1->vertices[0].x,t1->vertices[1].x,t1->vertices[2].x });
	float great2 = std::max<float>({ t2->vertices[0].x,t2->vertices[1].x,t2->vertices[2].x });

	return great1 > great2;
}

bool KdTree::MyBoundingBox::GreatestY(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float great1 = std::max<float>({ t1->vertices[0].y,t1->vertices[1].y,t1->vertices[2].y });
	float great2 = std::max<float>({ t2->vertices[0].y,t2->vertices[1].y,t2->vertices[2].y });

	return great1 > great2;
}

bool KdTree::MyBoundingBox::GreatestZ(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float great1 = std::max<float>({ t1->vertices[0].z,t1->vertices[1].z,t1->vertices[2].z });
	float great2 = std::max<float>({ t2->vertices[0].z,t2->vertices[1].z,t2->vertices[2].z });

	return great1 > great2;
}

KdTree::MyBoundingBox::MyBoundingBox(const std::vector<KdTree::Triangle*>& tris)
{
	size_t size = tris.size();

	auto xExtremes = std::minmax_element(tris.begin(), tris.end(), SmallestX);
	auto yExtremes = std::minmax_element(tris.begin(), tris.end(), SmallestY);
	auto zExtremes = std::minmax_element(tris.begin(), tris.end(), SmallestZ);

	smallest.x = std::min<float>({ (*xExtremes.first)->vertices[0].x,(*xExtremes.first)->vertices[1].x,(*xExtremes.first)->vertices[2].x });
	smallest.y = std::min<float>({ (*yExtremes.first)->vertices[0].y,(*yExtremes.first)->vertices[1].y,(*yExtremes.first)->vertices[2].y });
	smallest.z = std::min<float>({ (*zExtremes.first)->vertices[0].z,(*zExtremes.first)->vertices[1].z,(*zExtremes.first)->vertices[2].z });

	greatest.x = std::max<float>({ (*xExtremes.second)->vertices[0].x,(*xExtremes.second)->vertices[1].x,(*xExtremes.second)->vertices[2].x });
	greatest.y = std::max<float>({ (*yExtremes.second)->vertices[0].y,(*yExtremes.second)->vertices[1].y,(*yExtremes.second)->vertices[2].y });
	greatest.z = std::max<float>({ (*zExtremes.second)->vertices[0].z,(*zExtremes.second)->vertices[1].z,(*zExtremes.second)->vertices[2].z });

	Center.x = (smallest.x + greatest.x) / 2.0f;
	Center.y = (smallest.y + greatest.y) / 2.0f;
	Center.z = (smallest.z + greatest.z) / 2.0f;

	Extents.x = greatest.x - Center.x;
	Extents.y = greatest.y - Center.y;
	Extents.z = greatest.z - Center.z;
}

void KdTree::MyBoundingBox::Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, DirectX::XMVECTORF32 color)
{
	std::vector<DirectX::SimpleMath::Vector3> corners;
	corners.resize(CORNER_COUNT);
	GetCorners(&corners[0]);
	if (!corners.empty())
	{
		batch->DrawLine(DirectX::VertexPositionColor(corners[0], color), DirectX::VertexPositionColor(corners[1], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[1], color), DirectX::VertexPositionColor(corners[2], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[2], color), DirectX::VertexPositionColor(corners[3], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[3], color), DirectX::VertexPositionColor(corners[0], color));

		batch->DrawLine(DirectX::VertexPositionColor(corners[4], color), DirectX::VertexPositionColor(corners[5], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[5], color), DirectX::VertexPositionColor(corners[6], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[6], color), DirectX::VertexPositionColor(corners[7], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[7], color), DirectX::VertexPositionColor(corners[4], color));

		batch->DrawLine(DirectX::VertexPositionColor(corners[0], color), DirectX::VertexPositionColor(corners[4], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[1], color), DirectX::VertexPositionColor(corners[5], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[2], color), DirectX::VertexPositionColor(corners[6], color));
		batch->DrawLine(DirectX::VertexPositionColor(corners[3], color), DirectX::VertexPositionColor(corners[7], color));
	}
}

int KdTree::MyBoundingBox::GetLongestAxis() const
{
	float longest = std::max<float>({ Extents.x, Extents.y, Extents.z });

	if (longest == Extents.x)
	{
		return 0;
	}
	if (longest == Extents.y)
	{
		return 1;
	}
	if (longest == Extents.z)
	{
		return 2;
	}

	return 0;
}

bool KdTree::KdNode::SmallestX(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float small1 = (t1->vertices[0].x + t1->vertices[1].x + t1->vertices[2].x) / 3.0f;
	float small2 = (t2->vertices[0].x + t2->vertices[1].x + t2->vertices[2].x) / 3.0f;

	return small1 < small2;
}

bool KdTree::KdNode::SmallestY(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float small1 = (t1->vertices[0].y + t1->vertices[1].y + t1->vertices[2].y) / 3.0f;
	float small2 = (t2->vertices[0].y + t2->vertices[1].y + t2->vertices[2].y) / 3.0f;

	return small1 < small2;
}

bool KdTree::KdNode::SmallestZ(const KdTree::Triangle* t1, const KdTree::Triangle* t2)
{
	float small1 = (t1->vertices[0].z + t1->vertices[1].z + t1->vertices[2].z) / 3.0f;
	float small2 = (t2->vertices[0].z + t2->vertices[1].z + t2->vertices[2].z) / 3.0f;

	return small1 < small2;
}

KdTree::KdNode::KdNode()
{
	left = nullptr;
	right = nullptr;
	bbox = nullptr;
}

KdTree::KdNode* KdTree::KdNode::build(std::vector<KdTree::Triangle*>* tris, int depth)
{
	KdNode* node = new KdNode();
	node->triangles = tris;
	node->left = nullptr;
	node->right = nullptr;
	node->bbox = nullptr;

	if (tris->empty())
	{
		return node;
	}

	if (tris->size() <= 100)
	{
		node->bbox = new MyBoundingBox(*tris);
		node->left = new KdNode();
		node->right = new KdNode();
		node->left->triangles = new std::vector<KdTree::Triangle*>();
		node->right->triangles = new std::vector<KdTree::Triangle*>();
		return node;
	}

	node->bbox = new MyBoundingBox(*tris);

	std::vector<KdTree::Triangle*>* leftTris = new std::vector<KdTree::Triangle*>();
	std::vector<KdTree::Triangle*>* rightTris = new std::vector<KdTree::Triangle*>();;
	KdTree::Triangle* medTri = nullptr;
	float axisBaryCenter = 0.0f;
	int axis = node->bbox->GetLongestAxis();

	if (axis == 0) {
		std::nth_element(tris->begin(), tris->begin() + tris->size() / 2, tris->end(), SmallestX);
		medTri = (*(tris->begin() + tris->size() / 2));
		axisBaryCenter = medTri->getBarycenter().x;
	}
	else if (axis == 1) {
		std::nth_element(tris->begin(), tris->begin() + tris->size() / 2, tris->end(), SmallestY);
		medTri = (*(tris->begin() + tris->size() / 2));
		axisBaryCenter = medTri->getBarycenter().y;
	}
	else {
		std::nth_element(tris->begin(), tris->begin() + tris->size() / 2, tris->end(), SmallestZ);
		medTri = (*(tris->begin() + tris->size() / 2));
		axisBaryCenter = medTri->getBarycenter().z;
	}

	if (medTri->alreadyCut) {
		node->bbox = new MyBoundingBox(*tris);
		node->left = new KdNode();
		node->right = new KdNode();
		node->left->triangles = new std::vector<KdTree::Triangle*>();
		node->right->triangles = new std::vector<KdTree::Triangle*>();
		return node;
	}
	medTri->alreadyCut = true;

	for (int i = 0; i < tris->size(); ++i)
	{
		switch (axis)
		{
		case 0:
			if ((*tris)[i]->smallest.x < axisBaryCenter)
			{
				leftTris->push_back((*tris)[i]);
			}
			if ((*tris)[i]->greatest.x >= axisBaryCenter)
			{
				rightTris->push_back((*tris)[i]);
			}
			break;
		case 1:
			if ((*tris)[i]->smallest.y < axisBaryCenter)
			{
				leftTris->push_back((*tris)[i]);
			}
			if ((*tris)[i]->greatest.y >= axisBaryCenter)
			{
				rightTris->push_back((*tris)[i]);
			}
			break;
		case 2:
			if ((*tris)[i]->smallest.z < axisBaryCenter)
			{
				leftTris->push_back((*tris)[i]);
			}
			if ((*tris)[i]->greatest.z >= axisBaryCenter)
			{
				rightTris->push_back((*tris)[i]);
			}
			break;
		default: break;
		}
	}

	node->left = build(leftTris, depth + 1);
	node->right = build(rightTris, depth + 1);

	return node;
}

bool KdTree::KdNode::hit(KdNode* node, const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, KdTree::RayHitStruct& rayhit)
{
	float f;
	if (ray->Intersects(*node->bbox, f))
	{
		if (node->left->triangles->size() > 0 || node->right->triangles->size() > 0)
		{
			bool hitleft = false;
			bool hitright = false;

			if (node->left->triangles->size() > 0)
			{
				if (hit(node->left, ray, t, tmin, rayhit))
				{
					hitleft = true;
				}
			}

			if (node->right->triangles->size() > 0)
			{
				if (hit(node->right, ray, t, tmin, rayhit))
				{
					hitright = true;
				}
			}

			return hitleft || hitright;
		}
		else
		{
			bool hitBool = false;
			for (const auto tri : *node->triangles)
			{
				if (ray->Intersects(tri->vertices[0], tri->vertices[1], tri->vertices[2], t))
				{
					if (t < tmin)
					{
						tmin = t;
						rayhit.hitDistance = t;
						rayhit.hitTriangle = tri;
						rayhit.hitray = *ray;
						rayhit.hitBox = node->bbox;
						rayhit.hitPoint = rayhit.hitray.position + rayhit.hitray.direction * rayhit.hitDistance;
						hitBool = true;
					}
				}
			}

			return hitBool;
		}

	}

	return false;
}

bool KdTree::KdNode::hitCheckAll(KdNode* node, const DirectX::SimpleMath::Ray* ray, float& t, float& tmin, RayHitStruct& rayhit)
{
	bool hitSomething = false;
	for (const auto tri : *(node->triangles))
	{
		if (ray->Intersects(tri->vertices[0], tri->vertices[1], tri->vertices[2], t))
		{
			if (t < tmin)
			{
				tmin = t;
				rayhit.hitDistance = t;
				rayhit.hitTriangle = tri;
				rayhit.hitray = *ray;
				rayhit.hitBox = node->bbox;
				rayhit.hitPoint = rayhit.hitray.position + rayhit.hitray.direction * rayhit.hitDistance; 
				hitSomething = true;
			}
		}
	}

	return hitSomething;
}

void KdTree::KdNode::Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, DirectX::XMVECTORF32 color)
{
	if (this->bbox != nullptr)
	{
		this->bbox->Draw(batch, color);
	}if (this->left != nullptr)
	{
		this->left->Draw(batch, color);
	}
	if (this->right != nullptr)
	{
		this->right->Draw(batch, color);
	}
}

void KdTree::Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, DirectX::XMVECTORF32 color)
{
	if(tree)
	{
		tree->Draw(batch, color);
	}
	
}

void KdTree::PurgeTriangles()
{
	for(size_t i = 0u; i < treeTriangles->size(); ++i)
	{
		delete (*treeTriangles)[i];
	}

	treeTriangles->clear();
}
