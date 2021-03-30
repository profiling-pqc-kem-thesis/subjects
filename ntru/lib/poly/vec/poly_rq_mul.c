#include <vecintrin.h>

#include "../../poly.h"

typedef uint16_t vuint16_t __attribute__((vector_size(sizeof(uint16_t) * NTRU_N)));

void poly_Rq_mul(poly *r, const poly *a, const poly *b) {
  vuint16_t *va = (vuint16_t *)a->coeffs;
  vuint16_t *vb = (vuint16_t *)b->coeffs;
  vuint16_t *vr = (vuint16_t *)r->coeffs;

  for (int k = 0; k < NTRU_N; k++) {
    (*vr)[k] = 0;
    for (int i = 1; i < NTRU_N - k; i++)
      (*vr)[k] = (*vr)[k] + (*va)[k + i] * (*vb)[NTRU_N - i];
    for (int i = 0; i < k + 1; i++)
      (*vr)[k] = (*vr)[k] + (*va)[k - i] * (*vb)[i];
  }
}
