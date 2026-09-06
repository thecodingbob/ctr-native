#include <common.h>


/// @brief Link Hierarchal Matrix (like weapon relative to kart)
/// @param pDst - destination instance
/// @param pSrc - source instance
/// @param transVec - transform vector (x,y,z)
/// 0x800313c8
void LHMatrix_Parent(struct Instance *pDst, struct Instance *pSrc, const SVec3 *transVec)
{
	memcpy(&pDst->matrix, &pSrc->matrix, sizeof(pSrc->matrix));
	SetRotMatrix(&pDst->matrix);
	SetTransMatrix(&pDst->matrix);
	CTR_GteLoadSVec3V0(transVec);
	gte_rt();
	CTR_GteStoreMAC(pDst->matrix.t);
}
