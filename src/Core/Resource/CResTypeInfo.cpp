#include "CResTypeInfo.h"
#include <Common/AssertMacro.h>
#include <algorithm>

std::unordered_map<EResType, CResTypeInfo*> CResTypeInfo::smTypeMap;

CResTypeInfo::CResTypeInfo(EResType Type, const TString& rkTypeName)
    : mType(Type)
    , mTypeName(rkTypeName)
    , mHidden(false)
    , mCanBeSerialized(false)
{
#if !PUBLIC_RELEASE
    ASSERT(smTypeMap.find(Type) == smTypeMap.end());
#endif
    smTypeMap[Type] = this;
}

CResTypeInfo::~CResTypeInfo()
{
    // shouldn't happen - we want to just create these at launch and keep them around forever
    ASSERT(false);
}

bool CResTypeInfo::IsInGame(EGame Game) const
{
    for (u32 iGame = 0; iGame < mCookedExtensions.size(); iGame++)
    {
        if (mCookedExtensions[iGame].Game == Game)
            return true;
    }
    return false;
}

CFourCC CResTypeInfo::CookedExtension(EGame Game) const
{
    for (u32 iGame = 0; iGame < mCookedExtensions.size(); iGame++)
    {
        if (mCookedExtensions[iGame].Game == Game)
            return mCookedExtensions[iGame].CookedExt;
    }

    return "NONE";
}

// ************ STATIC ************
void CResTypeInfo::GetAllTypesInGame(EGame Game, std::list<CResTypeInfo*>& rOut)
{
    for (auto Iter = smTypeMap.begin(); Iter != smTypeMap.end(); Iter++)
    {
        CResTypeInfo *pType = Iter->second;

        if (pType->IsInGame(Game))
            rOut.push_back(pType);
    }
}

CResTypeInfo* CResTypeInfo::TypeForCookedExtension(EGame Game, CFourCC Ext)
{
    // Extensions can vary between games, but we're not likely to be calling this function for different games very often.
    // So, to speed things up a little, cache the lookup results in a map.
    static EGame sCachedGame = eUnknownGame;
    static std::map<CFourCC, CResTypeInfo*> sCachedTypeMap;
    Ext = Ext.ToUpper();

    // When the game changes, our cache is invalidated, so clear it
    if (sCachedGame != Game)
    {
        sCachedGame = Game;
        sCachedTypeMap.clear();
    }

    // Is this type cached?
    auto Iter = sCachedTypeMap.find(Ext);
    if (Iter != sCachedTypeMap.end())
        return Iter->second;

    // Not cached - do a slow lookup
    for (auto Iter = smTypeMap.begin(); Iter != smTypeMap.end(); Iter++)
    {
        CResTypeInfo *pType = Iter->second;

        if (pType->CookedExtension(Game) == Ext)
        {
            sCachedTypeMap[Ext] = pType;
            return pType;
        }
    }

    // Haven't found it; caller gave us an invalid type
    Log::Error("Failed to find resource type for cooked extension: " + Ext.ToString());
    DEBUG_BREAK;
    sCachedTypeMap[Ext] = nullptr;
    return nullptr;
}

// ************ CREATION ************
CResTypeInfo::CResTypeInfoFactory CResTypeInfo::smTypeInfoFactory;

CResTypeInfo::CResTypeInfoFactory::CResTypeInfoFactory()
{
    InitTypes();
}

void CResTypeInfo::CResTypeInfoFactory::AddExtension(CResTypeInfo *pType, CFourCC Ext, EGame FirstGame, EGame LastGame)
{
    ASSERT(FirstGame >= ePrimeDemo && LastGame <= eReturns && FirstGame <= LastGame);
    ASSERT(FirstGame != eUnknownGame && LastGame != eUnknownGame);

    for (int iGame = FirstGame; iGame <= LastGame; iGame++)
    {
#if !PUBLIC_RELEASE
        ASSERT(!pType->IsInGame((EGame) iGame));
#endif

        CResTypeInfo::SGameExtension Info { (EGame) iGame, Ext };
        pType->mCookedExtensions.push_back(Info);
    }

    std::sort(pType->mCookedExtensions.begin(), pType->mCookedExtensions.end(), [](const CResTypeInfo::SGameExtension& rkLeft, const CResTypeInfo::SGameExtension& rkRight) -> bool {
        return rkLeft.Game < rkRight.Game;
    });
}

void CResTypeInfo::CResTypeInfoFactory::InitTypes()
{
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimation, "Animation");
        AddExtension(pType, "ANIM", ePrimeDemo, eReturns);
        pType->mRawExtension = "ani";
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimCollisionPrimData, "Animation Collision Primitive Data");
        AddExtension(pType, "CPRM", eReturns, eReturns);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimEventData, "Animation Event Data");
        AddExtension(pType, "EVNT", ePrimeDemo, ePrime);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimSet, "Animation Character Set");
        AddExtension(pType, "ANCS", ePrimeDemo, eEchoes);
        pType->mRawExtension = "acs";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eArea, "Area");
        AddExtension(pType, "MREA", ePrimeDemo, eReturns);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioGroup, "Audio Group");
        AddExtension(pType, "AGSC", ePrimeDemo, eEchoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioMacro, "Audio Macro");
        AddExtension(pType, "CAUD", eCorruptionProto, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioSample, "Audio Sample");
        AddExtension(pType, "CSMP", eCorruptionProto, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioLookupTable, "Audio Lookup Table");
        AddExtension(pType, "ATBL", ePrimeDemo, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eBinaryData, "Generic Data");
        AddExtension(pType, "DUMB", ePrimeDemo, eCorruption);
        pType->mRawExtension = "dat";
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eBurstFireData, "Burst Fire Data");
        AddExtension(pType, "BFRC", eCorruptionProto, eCorruption);
        pType->mRawExtension = "bfre";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eCharacter, "Character");
        AddExtension(pType, "CHAR", eCorruptionProto, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eDependencyGroup, "Dependency Group");
        AddExtension(pType, "DGRP", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eDynamicCollision, "Dynamic Collision");
        AddExtension(pType, "DCLN", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eFont, "Font");
        AddExtension(pType, "FONT", ePrimeDemo, eReturns);
        pType->mRawExtension = "rpff";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eGuiFrame, "Gui Frame");
        AddExtension(pType, "FRME", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eGuiKeyFrame, "Gui Keyframe");
        AddExtension(pType, "KFAM", ePrimeDemo, ePrimeDemo);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eHintSystem, "Hint System Data");
        AddExtension(pType, "HINT", ePrime, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMapArea, "Area Map");
        AddExtension(pType, "MAPA", ePrimeDemo, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMapWorld, "World Map");
        AddExtension(pType, "MAPW", ePrimeDemo, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMapUniverse, "Universe Map");
        AddExtension(pType, "MAPU", ePrimeDemo, eEchoes);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMidi, "MIDI");
        AddExtension(pType, "CSNG", ePrimeDemo, eEchoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eModel, "Model");
        AddExtension(pType, "CMDL", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticle, "Particle System");
        AddExtension(pType, "PART", ePrimeDemo, eReturns);
        pType->mRawExtension = "gpsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleCollisionResponse, "Collision Response Particle System");
        AddExtension(pType, "CRSC", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleDecal, "Decal Particle System");
        AddExtension(pType, "DPSC", ePrimeDemo, eCorruption);
        pType->mRawExtension = "dpsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleElectric, "Electric Particle System");
        AddExtension(pType, "ELSC", ePrimeDemo, eCorruption);
        pType->mRawExtension = "elsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleSorted, "Sorted Particle System");
        AddExtension(pType, "SRSC", eEchoesDemo, eEchoes);
        pType->mRawExtension = "srsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleSpawn, "Spawn Particle System");
        AddExtension(pType, "SPSC", eEchoesDemo, eReturns);
        pType->mRawExtension = "spsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleSwoosh, "Swoosh Particle System");
        AddExtension(pType, "SWHC", ePrimeDemo, eReturns);
        pType->mRawExtension = "swsh";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleTransform, "Transform Particle System");
        AddExtension(pType, "XFSC", eReturns, eReturns);
        pType->mRawExtension = "xfsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleWeapon, "Weapon Particle System");
        AddExtension(pType, "WPSC", ePrimeDemo, eCorruption);
        pType->mRawExtension = "wpsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(ePathfinding, "Pathfinding Mesh");
        AddExtension(pType, "PATH", ePrimeDemo, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(ePortalArea, "Portal Area");
        AddExtension(pType, "PTLA", eEchoesDemo, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eRuleSet, "Rule Set");
        AddExtension(pType, "RULE", eEchoesDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSaveArea, "Area Save Info");
        AddExtension(pType, "SAVA", eCorruptionProto, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSaveWorld, "World Save Info");
        AddExtension(pType, "SAVW", ePrime, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eScan, "Scan");
        AddExtension(pType, "SCAN", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSkeleton, "Skeleton");
        AddExtension(pType, "CINF", ePrimeDemo, eReturns);
        pType->mRawExtension = "cin";
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSkin, "Skin");
        AddExtension(pType, "CSKR", ePrimeDemo, eReturns);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSourceAnimData, "Source Animation Data");
        AddExtension(pType, "SAND", eCorruptionProto, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSpatialPrimitive, "Spatial Primitive");
        AddExtension(pType, "CSPP", eEchoesDemo, eEchoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStateMachine, "State Machine");
        AddExtension(pType, "AFSM", ePrimeDemo, eEchoes);
        AddExtension(pType, "FSM2", eCorruptionProto, eCorruption);
        AddExtension(pType, "FSMC", eReturns, eReturns);
        pType->mRawExtension = "fsm";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStateMachine2, "State Machine 2");
        AddExtension(pType, "FSM2", eEchoesDemo, eEchoes);
        pType->mRawExtension = "fsm2";
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStaticGeometryMap, "Static Geometry Map");
        AddExtension(pType, "EGMC", eEchoesDemo, eCorruption);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStreamedAudio, "Streamed Audio");
        AddExtension(pType, "STRM", eCorruptionProto, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStringList, "String List");
        AddExtension(pType, "STLC", eEchoesDemo, eCorruptionProto);
        pType->mHidden = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStringTable, "String Table");
        AddExtension(pType, "STRG", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eTexture, "Texture");
        AddExtension(pType, "TXTR", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eTweak, "Tweak Data");
        AddExtension(pType, "CTWK", ePrimeDemo, ePrime);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eUnknown_CAAD, "CAAD");
        AddExtension(pType, "CAAD", eCorruption, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eUserEvaluatorData, "User Evaluator Data");
        AddExtension(pType, "USRC", eCorruptionProto, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eWorld, "World");
        AddExtension(pType, "MLVL", ePrimeDemo, eReturns);
        pType->mRawExtension = "mwld";
        pType->mHidden = true;
        pType->mCanBeSerialized = true;
    }
}