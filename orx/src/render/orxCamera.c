/**
 * @file orxCamera.c
 * 
 * Camera module
 * 
 */

 /***************************************************************************
 orxCamera.c
 Camera module
 
 begin                : 10/12/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "render/orxCamera.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/** Module flags
 */
#define orxCAMERA_KU32_STATIC_FLAG_NONE       0x00000000  /**< No flags */

#define orxCAMERA_KU32_STATIC_FLAG_READY      0x00000001  /**< Ready flag */
#define orxCAMERA_KU32_STATIC_FLAG_DATA_2D    0x00000010  /**< 2D flag */
#define orxCAMERA_KU32_STATIC_MASK_DEFAULT    0x00000010  /**< Default flag */

#define orxCAMERA_KU32_STATIC_MASK_ALL        0xFFFFFFFF  /**< All mask */


/** orxCAMERA flags / masks
 */
#define orxCAMERA_KU32_MASK_ALL               0xFFFFFFFF  /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal Frustrum Structure
 */
typedef struct __orxCAMERA_FRUSTRUM_t
{
  orxVECTOR vUL;                              /**< Upper left corner : 12 */
  orxVECTOR vBR;                              /**< Bottom right corner : 24 */

} orxCAMERA_FRUSTRUM;


/** Camera structure
 */
struct __orxCAMERA_t
{
  orxSTRUCTURE        stStructure;            /**< Public structure, first structure member : 16 */  
  orxFRAME           *pstFrame;               /**< Frame : 20 */
  orxCAMERA_FRUSTRUM  stFrustrum;             /**< Frustrum : 44 */

  orxPAD(44)
};


/** Static structure
 */
typedef struct __orxCAMERA_STATIC_t
{
  orxU32 u32Flags;                            /**< Control flags : 4 */

} orxCAMERA_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxCAMERA_STATIC sstCamera;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all cameras
 */
orxSTATIC orxVOID orxCamera_DeleteAll()
{
  orxCAMERA *pstCamera;
  
  /* Gets first camera */
  pstCamera = (orxCAMERA *)orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA);

  /* Non empty? */
  while(pstCamera != orxNULL)
  {
    /* Deletes camera */
    orxCamera_Delete(pstCamera);

    /* Gets first remaining camera */
    pstCamera = (orxCAMERA *)orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA);
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Camera module setup
 */
orxVOID orxCamera_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_FRAME);

  return;
}

/** Inits Camera module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxCamera_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if(!(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstCamera, 0, sizeof(orxCAMERA_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(CAMERA, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstCamera.u32Flags = orxCAMERA_KU32_STATIC_FLAG_READY | orxCAMERA_KU32_STATIC_MASK_DEFAULT;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from Camera module
 */
orxVOID orxCamera_Exit()
{
  /* Initialized? */
  if(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY)
  {
    /* Deletes camera list */
    orxCamera_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_CAMERA);

    /* Updates flags */
    sstCamera.u32Flags &= ~orxCAMERA_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates a camera
 * @return      Created orxCAMERA / orxNULL
 */
orxCAMERA *orxCamera_Create()
{
  orxCAMERA *pstCamera = orxNULL;
  orxFRAME  *pstFrame;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);

  /* Creates camera */
  pstCamera = (orxCAMERA *)orxStructure_Create(orxSTRUCTURE_ID_CAMERA);

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Creates frame */
    pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

    /* Valid? */  
    if(pstFrame != orxNULL)
    {
      /* 2D? */
      if(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_DATA_2D)
      {
        /* Stores frame */
        pstCamera->pstFrame = pstFrame;

        /* Increases its reference counter */
        orxStructure_IncreaseCounter(pstFrame);

        /* Updates flags */
        orxStructure_SetFlags(pstCamera, orxCAMERA_KU32_FLAG_2D, orxCAMERA_KU32_FLAG_NONE);
      }
      else
      {
        /* !!! MSG !!! */

        /* Fress partially allocated camera */
        orxFrame_Delete(pstFrame);
        orxStructure_Delete(pstCamera);

        /* Updates result */
        pstCamera = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Fress partially allocated camera */
      orxStructure_Delete(pstCamera);

      /* Updates result */
      pstCamera = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstCamera;
}

/** Deletes a camera
 * @param[in]   _pstCamera      Camera to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_Delete(orxCAMERA *_pstCamera)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstCamera) == 0)
  {
    /* Removes frame reference */
    orxStructure_DecreaseCounter(_pstCamera->pstFrame);

    /* Deletes frame*/
    orxFrame_Delete(_pstCamera->pstFrame);

    /* Deletes structure */
    orxStructure_Delete(_pstCamera);
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets camera frustrum (3D rectangle for 2D camera)
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _pvUL           Upper left corner position
 * @param[in]   _pvBR           Bottom right corner position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetFrustrum(orxCAMERA *_pstCamera, orxFLOAT _fWidth, orxFLOAT _fHeight, orxFLOAT _fNear, orxFLOAT _fFar)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_fFar > _fNear);

  /* Updates internal frustrum */
  orxVector_Set(&_pstCamera->stFrustrum.vUL, orx2F(-0.5f) * _fWidth, orx2F(-0.5f) * _fHeight, _fNear);
  orxVector_Set(&_pstCamera->stFrustrum.vBR, orx2F(0.5f) * _fWidth, orx2F(0.5f) * _fHeight, _fFar);

  /* Done! */
  return eResult;
}

/** Sets camera position
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _pvPosition     Camera position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetPosition(orxCAMERA *_pstCamera, orxCONST orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets camera position */
  orxFrame_SetPosition(_pstCamera->pstFrame, _pvPosition);

  /* Done! */
  return eResult;
}

/** Sets camera rotation
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fRotation      Camera rotation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetRotation(orxCAMERA *_pstCamera, orxFLOAT _fRotation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

   /* Sets camera rotation */
  orxFrame_SetRotation(_pstCamera->pstFrame, _fRotation);

  /* Done! */
  return eResult;
}

/** Sets camera zoom
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fZoom          Camera zoom
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetZoom(orxCAMERA *_pstCamera, orxFLOAT _fZoom)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

   /* Sets camera zoom */
  orxFrame_SetScale(_pstCamera->pstFrame, orxFLOAT_1 / _fZoom, orxFLOAT_1 / _fZoom);

  /* Done! */
  return eResult;
}

/** Gets camera frustrum (3D box for 2D camera)
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pvUL           Upper left corner position
 * @param[out]  _pvBR           Bottom right corner position
 */
orxVOID orxFASTCALL orxCamera_GetFrustrum(orxCONST orxCAMERA *_pstCamera, orxVECTOR *_pvUL, orxVECTOR *_pvBR)
{
  orxVECTOR vPosition;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pvUL != orxNULL);
  orxASSERT(_pvBR != orxNULL);

  /* Gets camera position */
  orxFrame_GetPosition(_pstCamera->pstFrame, orxFRAME_SPACE_GLOBAL, &vPosition);

  /* Stores frustrum */
  orxVector_Add(_pvUL, &(_pstCamera->stFrustrum.vUL), &vPosition);
  orxVector_Add(_pvBR, &(_pstCamera->stFrustrum.vBR), &vPosition);

  return;
}

/** Get camera position
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pvPosition     Camera position
 * @return      orxVECTOR
 */
orxVECTOR *orxFASTCALL orxCamera_GetPosition(orxCONST orxCAMERA *_pstCamera, orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets camera position */
  return(orxFrame_GetPosition(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition));
}

/** Get camera rotation
 * @param[in]   _pstCamera      Concerned camera
 * @return      Rotation value
 */
orxFLOAT orxFASTCALL orxCamera_GetRotation(orxCONST orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera rotation */
  return(orxFrame_GetRotation(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL));
}

/** Gets camera zoom
 * @param[in]   _pstCamera      Concerned camera
 * @return      Zoom value
 */
orxFLOAT orxFASTCALL orxCamera_GetZoom(orxCONST orxCAMERA *_pstCamera)
{
  orxFLOAT fScale, fDummy;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera scale */
  orxFrame_GetScale(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, &fScale, &fDummy);

  /* Done! */
  return(orxFLOAT_1 / fScale);
}

/** Gets camera frame
 * @param[in]   _pstCamera      Concerned camera
 * @return      orxFRAME
 */
orxFRAME *orxFASTCALL orxCamera_GetFrame(orxCONST orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera frame */
  return(_pstCamera->pstFrame);
}
