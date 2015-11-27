#ifndef CCOLLISIONNODE_H
#define CCOLLISIONNODE_H

#include "CSceneNode.h"
#include <Resource/CCollisionMeshGroup.h>

class CCollisionNode : public CSceneNode
{
    CCollisionMeshGroup *mpCollision;
    CToken mCollisionToken;

public:
    CCollisionNode(CSceneManager *pScene, CSceneNode *pParent = 0, CCollisionMeshGroup *pCollision = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
    void SetCollision(CCollisionMeshGroup *pCollision);
};

#endif // CCOLLISIONNODE_H
