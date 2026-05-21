#include <common.h>

// TODO(aalhendi): Source-backed helper required by BOTS_ThTick_Drive; audit
// against retail 0x8003d460 before adding an ASM-verified stamp.
void MATH_MatrixMul(MATRIX *output, MATRIX *input, VECTOR *rotate)
{
	MATRIX *rotateMatrix = (MATRIX *)rotate;

	MatrixRotate(output, input, rotateMatrix);
	ApplyMatrixLV(input, (VECTOR *)rotateMatrix->t, (VECTOR *)output->t);

	output->t[0] += input->t[0];
	output->t[1] += input->t[1];
	output->t[2] += input->t[2];
}

void DECOMP_MATH_MatrixMul(MATRIX *output, MATRIX *input, VECTOR *rotate)
{
	MATH_MatrixMul(output, input, rotate);
}
