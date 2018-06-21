////////////////////////////////////////////////////////////////////////////////
// SbModel.cpp

// Includes
#include "BaArchive.h"
#include "MtVector2.h"
#include "RsCamera.h"
#include "RsColour.h"
#include "RsFont.h"
#include "RsShader.h"
#include "RsUtil.h"
#include "RsRenderTarget.h"
#include "SgNode.h"
#include "MtAABB.h"
#include "SgMesh.h"
#include "SgAnimator.h"
#include "BtTime.h"
#include <stdio.h>
#include "UiKeyboard.h"
#include "HlDraw.h"
#include "ApConfig.h"
#include "SgBone.h"
#include "HlModel.h"
#include "SgSkin.h"
#include "ShTouch.h"
#include "ScModel.h"
#include "ScWorld.h"
#include "HlFont.h"
#include "HlModel.h"
#include "ShIMU.h"

////////////////////////////////////////////////////////////////////////////////
// Setup

void ScModel::Setup( BaArchive *pGameArchive )
{
	// Cache our model
	m_pCube1 = pGameArchive->GetNode("cube");
    m_pCube2 = m_pCube1->GetDuplicate();
    m_pCube3 = m_pCube1->GetDuplicate();
    HlModel::SetSortOrders(m_pCube1, ModelSortOrder);
    HlModel::SetSortOrders(m_pCube2, ModelSortOrder);
    HlModel::SetSortOrders(m_pCube3, ModelSortOrder);
    
	// Cache the main shader
	m_pShader = pGameArchive->GetShader( "shader" );
}
	
////////////////////////////////////////////////////////////////////////////////
// Update

void ScModel::Update( RsCamera &camera )
{
    MtMatrix4 m4Translate;
    
	if (m_pCube1)
	{
        MtVector3 v3Position = ShIMU::GetPosition(0);
        v3Position += MtVector3( -10.0f, 0, 10.0f );
        m4Translate.SetTranslation( v3Position );

        MtQuaternion quaternion = ShIMU::GetQuaternion(0);
        //quaternion.SetIdentity();
        MtMatrix4 m4Transform( quaternion );
        m_pCube1->SetLocalTransform( m4Transform * m4Translate );
		m_pCube1->Update();
	}
    if (m_pCube2)
    {
        MtVector3 v3Position = ShIMU::GetPosition(1);
        v3Position += MtVector3( 0, 0, 10.0f );
        m4Translate.SetTranslation( v3Position );
        
        MtQuaternion quaternion = ShIMU::GetQuaternion(1);
        //quaternion.SetIdentity();
        MtMatrix4 m4Transform( quaternion );
        m_pCube2->SetLocalTransform( m4Transform * m4Translate );
        m_pCube2->Update();
    }
    if (m_pCube3)
    {
        MtVector3 v3Position = ShIMU::GetPosition(2);
        v3Position += MtVector3( 10, 0, 10.0f );
        m4Translate.SetTranslation( v3Position );
        
        MtQuaternion quaternion = ShIMU::GetQuaternion(2);
        //quaternion.SetIdentity();
        MtMatrix4 m4Transform( quaternion );
        m_pCube3->SetLocalTransform( m4Transform * m4Translate );
        m_pCube3->Update();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Render

void ScModel::Render( RsCamera &camera )
{
	// Set the light direction
	MtVector3 v3LightDirection( -1, -1, 0 );

	m_pShader->SetDirectionalLight( v3LightDirection );

	// Set the lights ambient level
	m_pShader->SetAmbient( RsColour( 0.5f, 0.4f, 0.3f, 0 ) );

	// Apply the shader
	m_pShader->Apply();

	if (m_pCube1)
	{
		m_pCube1->Render();
	}
    if (m_pCube2)
    {
        m_pCube2->Render();
    }
    if (m_pCube3)
    {
        m_pCube3->Render();
    }
    
    MtVector3 v3Acc = ShIMU::GetAccelerometer(0);
    BtChar text[32];
    MtVector2 v2Position( 0, 0 );
    sprintf( text, "%.2f %.2f %.2f", v3Acc.x, v3Acc.y, v3Acc.z );
    HlFont::Render(v2Position, text, RsColour::BlackColour(), MaxSortOrders - 1);
}
