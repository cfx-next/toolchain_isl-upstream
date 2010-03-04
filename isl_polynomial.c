#include <isl_seq.h>
#include <isl_polynomial_private.h>
#include <isl_point_private.h>
#include <isl_dim_private.h>
#include <isl_map_private.h>

int isl_upoly_is_cst(__isl_keep struct isl_upoly *up)
{
	if (!up)
		return -1;

	return up->var < 0;
}

__isl_keep struct isl_upoly_cst *isl_upoly_as_cst(__isl_keep struct isl_upoly *up)
{
	if (!up)
		return NULL;

	isl_assert(up->ctx, up->var < 0, return NULL);

	return (struct isl_upoly_cst *)up;
}

__isl_keep struct isl_upoly_rec *isl_upoly_as_rec(__isl_keep struct isl_upoly *up)
{
	if (!up)
		return NULL;

	isl_assert(up->ctx, up->var >= 0, return NULL);

	return (struct isl_upoly_rec *)up;
}

int isl_upoly_is_zero(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;

	if (!up)
		return -1;
	if (!isl_upoly_is_cst(up))
		return 0;

	cst = isl_upoly_as_cst(up);
	if (!cst)
		return -1;

	return isl_int_is_zero(cst->n) && isl_int_is_pos(cst->d);
}

int isl_upoly_is_nan(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;

	if (!up)
		return -1;
	if (!isl_upoly_is_cst(up))
		return 0;

	cst = isl_upoly_as_cst(up);
	if (!cst)
		return -1;

	return isl_int_is_zero(cst->n) && isl_int_is_zero(cst->d);
}

int isl_upoly_is_infty(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;

	if (!up)
		return -1;
	if (!isl_upoly_is_cst(up))
		return 0;

	cst = isl_upoly_as_cst(up);
	if (!cst)
		return -1;

	return isl_int_is_pos(cst->n) && isl_int_is_zero(cst->d);
}

int isl_upoly_is_neginfty(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;

	if (!up)
		return -1;
	if (!isl_upoly_is_cst(up))
		return 0;

	cst = isl_upoly_as_cst(up);
	if (!cst)
		return -1;

	return isl_int_is_neg(cst->n) && isl_int_is_zero(cst->d);
}

int isl_upoly_is_one(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;

	if (!up)
		return -1;
	if (!isl_upoly_is_cst(up))
		return 0;

	cst = isl_upoly_as_cst(up);
	if (!cst)
		return -1;

	return isl_int_eq(cst->n, cst->d) && isl_int_is_pos(cst->d);
}

int isl_upoly_is_negone(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;

	if (!up)
		return -1;
	if (!isl_upoly_is_cst(up))
		return 0;

	cst = isl_upoly_as_cst(up);
	if (!cst)
		return -1;

	return isl_int_is_negone(cst->n) && isl_int_is_one(cst->d);
}

__isl_give struct isl_upoly_cst *isl_upoly_cst_alloc(struct isl_ctx *ctx)
{
	struct isl_upoly_cst *cst;

	cst = isl_alloc_type(ctx, struct isl_upoly_cst);
	if (!cst)
		return NULL;

	cst->up.ref = 1;
	cst->up.ctx = ctx;
	isl_ctx_ref(ctx);
	cst->up.var = -1;

	isl_int_init(cst->n);
	isl_int_init(cst->d);

	return cst;
}

__isl_give struct isl_upoly *isl_upoly_zero(struct isl_ctx *ctx)
{
	struct isl_upoly_cst *cst;

	cst = isl_upoly_cst_alloc(ctx);
	if (!cst)
		return NULL;

	isl_int_set_si(cst->n, 0);
	isl_int_set_si(cst->d, 1);

	return &cst->up;
}

__isl_give struct isl_upoly *isl_upoly_infty(struct isl_ctx *ctx)
{
	struct isl_upoly_cst *cst;

	cst = isl_upoly_cst_alloc(ctx);
	if (!cst)
		return NULL;

	isl_int_set_si(cst->n, 1);
	isl_int_set_si(cst->d, 0);

	return &cst->up;
}

__isl_give struct isl_upoly *isl_upoly_nan(struct isl_ctx *ctx)
{
	struct isl_upoly_cst *cst;

	cst = isl_upoly_cst_alloc(ctx);
	if (!cst)
		return NULL;

	isl_int_set_si(cst->n, 0);
	isl_int_set_si(cst->d, 0);

	return &cst->up;
}

__isl_give struct isl_upoly *isl_upoly_rat_cst(struct isl_ctx *ctx,
	isl_int n, isl_int d)
{
	struct isl_upoly_cst *cst;

	cst = isl_upoly_cst_alloc(ctx);
	if (!cst)
		return NULL;

	isl_int_set(cst->n, n);
	isl_int_set(cst->d, d);

	return &cst->up;
}

__isl_give struct isl_upoly_rec *isl_upoly_alloc_rec(struct isl_ctx *ctx,
	int var, int size)
{
	struct isl_upoly_rec *rec;

	isl_assert(ctx, var >= 0, return NULL);
	isl_assert(ctx, size >= 0, return NULL);
	rec = isl_calloc(dim->ctx, struct isl_upoly_rec,
			sizeof(struct isl_upoly_rec) +
			(size - 1) * sizeof(struct isl_upoly *));
	if (!rec)
		return NULL;

	rec->up.ref = 1;
	rec->up.ctx = ctx;
	isl_ctx_ref(ctx);
	rec->up.var = var;

	rec->n = 0;
	rec->size = size;

	return rec;
error:
	isl_upoly_free(&rec->up);
	return NULL;
}

int isl_qpolynomial_is_zero(__isl_keep isl_qpolynomial *qp)
{
	struct isl_upoly_cst *cst;

	if (!qp)
		return -1;

	return isl_upoly_is_zero(qp->upoly);
}

int isl_qpolynomial_is_one(__isl_keep isl_qpolynomial *qp)
{
	struct isl_upoly_cst *cst;

	if (!qp)
		return -1;

	return isl_upoly_is_one(qp->upoly);
}

static void upoly_free_cst(__isl_take struct isl_upoly_cst *cst)
{
	isl_int_clear(cst->n);
	isl_int_clear(cst->d);
}

static void upoly_free_rec(__isl_take struct isl_upoly_rec *rec)
{
	int i;

	for (i = 0; i < rec->n; ++i)
		isl_upoly_free(rec->p[i]);
}

__isl_give struct isl_upoly *isl_upoly_copy(__isl_keep struct isl_upoly *up)
{
	if (!up)
		return NULL;

	up->ref++;
	return up;
}

__isl_give struct isl_upoly *isl_upoly_dup_cst(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;
	struct isl_upoly_cst *dup;

	cst = isl_upoly_as_cst(up);
	if (!cst)
		return NULL;

	dup = isl_upoly_as_cst(isl_upoly_zero(up->ctx));
	if (!dup)
		return NULL;
	isl_int_set(dup->n, cst->n);
	isl_int_set(dup->d, cst->d);

	return &dup->up;
}

__isl_give struct isl_upoly *isl_upoly_dup_rec(__isl_keep struct isl_upoly *up)
{
	int i;
	struct isl_upoly_rec *rec;
	struct isl_upoly_rec *dup;

	rec = isl_upoly_as_rec(up);
	if (!rec)
		return NULL;

	dup = isl_upoly_alloc_rec(up->ctx, up->var, rec->n);
	if (!dup)
		return NULL;

	for (i = 0; i < rec->n; ++i) {
		dup->p[i] = isl_upoly_copy(rec->p[i]);
		if (!dup->p[i])
			goto error;
		dup->n++;
	}

	return &dup->up;
error:
	isl_upoly_free(&dup->up);
	return NULL;
}

__isl_give struct isl_upoly *isl_upoly_dup(__isl_keep struct isl_upoly *up)
{
	struct isl_upoly *dup;

	if (!up)
		return NULL;

	if (isl_upoly_is_cst(up))
		return isl_upoly_dup_cst(up);
	else
		return isl_upoly_dup_rec(up);
}

__isl_give struct isl_upoly *isl_upoly_cow(__isl_take struct isl_upoly *up)
{
	if (!up)
		return NULL;

	if (up->ref == 1)
		return up;
	up->ref--;
	return isl_upoly_dup(up);
}

void isl_upoly_free(__isl_take struct isl_upoly *up)
{
	if (!up)
		return;

	if (--up->ref > 0)
		return;

	if (up->var < 0)
		upoly_free_cst((struct isl_upoly_cst *)up);
	else
		upoly_free_rec((struct isl_upoly_rec *)up);

	isl_ctx_deref(up->ctx);
	free(up);
}

static void isl_upoly_cst_reduce(__isl_keep struct isl_upoly_cst *cst)
{
	isl_int gcd;

	isl_int_init(gcd);
	isl_int_gcd(gcd, cst->n, cst->d);
	if (!isl_int_is_zero(gcd) && !isl_int_is_one(gcd)) {
		isl_int_divexact(cst->n, cst->n, gcd);
		isl_int_divexact(cst->d, cst->d, gcd);
	}
	isl_int_clear(gcd);
}

__isl_give struct isl_upoly *isl_upoly_sum_cst(__isl_take struct isl_upoly *up1,
	__isl_take struct isl_upoly *up2)
{
	struct isl_upoly_cst *cst1;
	struct isl_upoly_cst *cst2;

	up1 = isl_upoly_cow(up1);
	if (!up1 || !up2)
		goto error;

	cst1 = isl_upoly_as_cst(up1);
	cst2 = isl_upoly_as_cst(up2);

	if (isl_int_eq(cst1->d, cst2->d))
		isl_int_add(cst1->n, cst1->n, cst2->n);
	else {
		isl_int_mul(cst1->n, cst1->n, cst2->d);
		isl_int_addmul(cst1->n, cst2->n, cst1->d);
		isl_int_mul(cst1->d, cst1->d, cst2->d);
	}

	isl_upoly_cst_reduce(cst1);

	isl_upoly_free(up2);
	return up1;
error:
	isl_upoly_free(up1);
	isl_upoly_free(up2);
	return NULL;
}

static __isl_give struct isl_upoly *replace_by_zero(
	__isl_take struct isl_upoly *up)
{
	struct isl_ctx *ctx;

	if (!up)
		return NULL;
	ctx = up->ctx;
	isl_upoly_free(up);
	return isl_upoly_zero(ctx);
}

__isl_give struct isl_upoly *isl_upoly_sum(__isl_take struct isl_upoly *up1,
	__isl_take struct isl_upoly *up2)
{
	int i;
	struct isl_upoly_rec *rec1, *rec2;

	if (!up1 || !up2)
		goto error;

	if (isl_upoly_is_nan(up1)) {
		isl_upoly_free(up2);
		return up1;
	}

	if (isl_upoly_is_nan(up2)) {
		isl_upoly_free(up1);
		return up2;
	}

	if (isl_upoly_is_zero(up1)) {
		isl_upoly_free(up1);
		return up2;
	}

	if (isl_upoly_is_zero(up2)) {
		isl_upoly_free(up2);
		return up1;
	}

	if (up1->var < up2->var)
		return isl_upoly_sum(up2, up1);

	if (up2->var < up1->var) {
		struct isl_upoly_rec *rec;
		if (isl_upoly_is_infty(up2) || isl_upoly_is_neginfty(up2)) {
			isl_upoly_free(up1);
			return up2;
		}
		up1 = isl_upoly_cow(up1);
		rec = isl_upoly_as_rec(up1);
		if (!rec)
			goto error;
		rec->p[0] = isl_upoly_sum(rec->p[0], up2);
		if (rec->n == 1 && isl_upoly_is_zero(rec->p[0]))
			up1 = replace_by_zero(up1);
		return up1;
	}

	if (isl_upoly_is_cst(up1))
		return isl_upoly_sum_cst(up1, up2);

	rec1 = isl_upoly_as_rec(up1);
	rec2 = isl_upoly_as_rec(up2);
	if (!rec1 || !rec2)
		goto error;

	if (rec1->n < rec2->n)
		return isl_upoly_sum(up2, up1);

	up1 = isl_upoly_cow(up1);
	rec1 = isl_upoly_as_rec(up1);
	if (!rec1)
		goto error;

	for (i = rec2->n - 1; i >= 0; --i) {
		rec1->p[i] = isl_upoly_sum(rec1->p[i],
					    isl_upoly_copy(rec2->p[i]));
		if (!rec1->p[i])
			goto error;
		if (i == rec1->n - 1 && isl_upoly_is_zero(rec1->p[i])) {
			isl_upoly_free(rec1->p[i]);
			rec1->n--;
		}
	}

	if (rec1->n == 0)
		up1 = replace_by_zero(up1);

	isl_upoly_free(up2);

	return up1;
error:
	isl_upoly_free(up1);
	isl_upoly_free(up2);
	return NULL;
}

__isl_give struct isl_upoly *isl_upoly_neg_cst(__isl_take struct isl_upoly *up)
{
	struct isl_upoly_cst *cst;

	if (isl_upoly_is_zero(up))
		return up;

	up = isl_upoly_cow(up);
	if (!up)
		return NULL;

	cst = isl_upoly_as_cst(up);

	isl_int_neg(cst->n, cst->n);

	return up;
}

__isl_give struct isl_upoly *isl_upoly_neg(__isl_take struct isl_upoly *up)
{
	int i;
	struct isl_upoly_rec *rec;

	if (!up)
		return NULL;

	if (isl_upoly_is_cst(up))
		return isl_upoly_neg_cst(up);

	up = isl_upoly_cow(up);
	rec = isl_upoly_as_rec(up);
	if (!rec)
		goto error;

	for (i = 0; i < rec->n; ++i) {
		rec->p[i] = isl_upoly_neg(rec->p[i]);
		if (!rec->p[i])
			goto error;
	}

	return up;
error:
	isl_upoly_free(up);
	return NULL;
}

__isl_give struct isl_upoly *isl_upoly_mul_cst(__isl_take struct isl_upoly *up1,
	__isl_take struct isl_upoly *up2)
{
	struct isl_upoly_cst *cst1;
	struct isl_upoly_cst *cst2;

	up1 = isl_upoly_cow(up1);
	if (!up1 || !up2)
		goto error;

	cst1 = isl_upoly_as_cst(up1);
	cst2 = isl_upoly_as_cst(up2);

	isl_int_mul(cst1->n, cst1->n, cst2->n);
	isl_int_mul(cst1->d, cst1->d, cst2->d);

	isl_upoly_cst_reduce(cst1);

	isl_upoly_free(up2);
	return up1;
error:
	isl_upoly_free(up1);
	isl_upoly_free(up2);
	return NULL;
}

__isl_give struct isl_upoly *isl_upoly_mul_rec(__isl_take struct isl_upoly *up1,
	__isl_take struct isl_upoly *up2)
{
	struct isl_upoly_rec *rec1;
	struct isl_upoly_rec *rec2;
	struct isl_upoly_rec *res;
	int i, j;
	int size;

	rec1 = isl_upoly_as_rec(up1);
	rec2 = isl_upoly_as_rec(up2);
	if (!rec1 || !rec2)
		goto error;
	size = rec1->n + rec2->n - 1;
	res = isl_upoly_alloc_rec(up1->ctx, up1->var, size);
	if (!res)
		goto error;

	for (i = 0; i < rec1->n; ++i) {
		res->p[i] = isl_upoly_mul(isl_upoly_copy(rec2->p[0]),
					    isl_upoly_copy(rec1->p[i]));
		if (!res->p[i])
			goto error;
		res->n++;
	}
	for (; i < size; ++i) {
		res->p[i] = isl_upoly_zero(up1->ctx);
		if (!res->p[i])
			goto error;
		res->n++;
	}
	for (i = 0; i < rec1->n; ++i) {
		for (j = 1; j < rec2->n; ++j) {
			struct isl_upoly *up;
			up = isl_upoly_mul(isl_upoly_copy(rec2->p[j]),
					    isl_upoly_copy(rec1->p[i]));
			res->p[i + j] = isl_upoly_sum(res->p[i + j], up);
			if (!res->p[i + j])
				goto error;
		}
	}

	isl_upoly_free(up1);
	isl_upoly_free(up2);

	return &res->up;
error:
	isl_upoly_free(up1);
	isl_upoly_free(up2);
	isl_upoly_free(&res->up);
	return NULL;
}

__isl_give struct isl_upoly *isl_upoly_mul(__isl_take struct isl_upoly *up1,
	__isl_take struct isl_upoly *up2)
{
	if (!up1 || !up2)
		goto error;

	if (isl_upoly_is_nan(up1)) {
		isl_upoly_free(up2);
		return up1;
	}

	if (isl_upoly_is_nan(up2)) {
		isl_upoly_free(up1);
		return up2;
	}

	if (isl_upoly_is_zero(up1)) {
		isl_upoly_free(up2);
		return up1;
	}

	if (isl_upoly_is_zero(up2)) {
		isl_upoly_free(up1);
		return up2;
	}

	if (isl_upoly_is_one(up1)) {
		isl_upoly_free(up1);
		return up2;
	}

	if (isl_upoly_is_one(up2)) {
		isl_upoly_free(up2);
		return up1;
	}

	if (up1->var < up2->var)
		return isl_upoly_mul(up2, up1);

	if (up2->var < up1->var) {
		int i;
		struct isl_upoly_rec *rec;
		if (isl_upoly_is_infty(up2) || isl_upoly_is_neginfty(up2)) {
			isl_ctx *ctx = up1->ctx;
			isl_upoly_free(up1);
			isl_upoly_free(up2);
			return isl_upoly_nan(ctx);
		}
		up1 = isl_upoly_cow(up1);
		rec = isl_upoly_as_rec(up1);
		if (!rec)
			goto error;

		for (i = 0; i < rec->n; ++i) {
			rec->p[i] = isl_upoly_mul(rec->p[i],
						    isl_upoly_copy(up2));
			if (!rec->p[i])
				goto error;
		}
		isl_upoly_free(up2);
		return up1;
	}

	if (isl_upoly_is_cst(up1))
		return isl_upoly_mul_cst(up1, up2);

	return isl_upoly_mul_rec(up1, up2);
error:
	isl_upoly_free(up1);
	isl_upoly_free(up2);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_alloc(__isl_take isl_dim *dim,
	unsigned n_div)
{
	struct isl_qpolynomial *qp;
	unsigned total;

	if (!dim)
		return NULL;

	total = isl_dim_total(dim);

	qp = isl_calloc_type(dim->ctx, struct isl_qpolynomial);
	if (!qp)
		return NULL;

	qp->ref = 1;
	qp->div = isl_mat_alloc(dim->ctx, n_div, 1 + 1 + total + n_div);
	if (!qp->div)
		goto error;

	qp->dim = dim;

	return qp;
error:
	isl_dim_free(dim);
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_copy(__isl_keep isl_qpolynomial *qp)
{
	if (!qp)
		return NULL;

	qp->ref++;
	return qp;
}

__isl_give isl_qpolynomial *isl_qpolynomial_dup(__isl_keep isl_qpolynomial *qp)
{
	struct isl_qpolynomial *dup;

	if (!qp)
		return NULL;

	dup = isl_qpolynomial_alloc(isl_dim_copy(qp->dim), qp->div->n_row);
	if (!dup)
		return NULL;
	isl_mat_free(dup->div);
	dup->div = isl_mat_copy(qp->div);
	if (!dup->div)
		goto error;
	dup->upoly = isl_upoly_copy(qp->upoly);
	if (!dup->upoly)
		goto error;

	return dup;
error:
	isl_qpolynomial_free(dup);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_cow(__isl_take isl_qpolynomial *qp)
{
	if (!qp)
		return NULL;

	if (qp->ref == 1)
		return qp;
	qp->ref--;
	return isl_qpolynomial_dup(qp);
}

void isl_qpolynomial_free(__isl_take isl_qpolynomial *qp)
{
	if (!qp)
		return;

	if (--qp->ref > 0)
		return;

	isl_dim_free(qp->dim);
	isl_mat_free(qp->div);
	isl_upoly_free(qp->upoly);

	free(qp);
}

static int compatible_divs(__isl_keep isl_mat *div1, __isl_keep isl_mat *div2)
{
	int n_row, n_col;
	int equal;

	isl_assert(div1->ctx, div1->n_row >= div2->n_row &&
				div1->n_col >= div2->n_col, return -1);

	if (div1->n_row == div2->n_row)
		return isl_mat_is_equal(div1, div2);

	n_row = div1->n_row;
	n_col = div1->n_col;
	div1->n_row = div2->n_row;
	div1->n_col = div2->n_col;

	equal = isl_mat_is_equal(div1, div2);

	div1->n_row = n_row;
	div1->n_col = n_col;

	return equal;
}

static void expand_row(__isl_keep isl_mat *dst, int d,
	__isl_keep isl_mat *src, int s, int *exp)
{
	int i;
	unsigned c = src->n_col - src->n_row;

	isl_seq_cpy(dst->row[d], src->row[s], c);
	isl_seq_clr(dst->row[d] + c, dst->n_col - c);

	for (i = 0; i < s; ++i)
		isl_int_set(dst->row[d][c + exp[i]], src->row[s][c + i]);
}

static int cmp_row(__isl_keep isl_mat *div, int i, int j)
{
	int li, lj;

	li = isl_seq_last_non_zero(div->row[i], div->n_col);
	lj = isl_seq_last_non_zero(div->row[j], div->n_col);

	if (li != lj)
		return li - lj;

	return isl_seq_cmp(div->row[i], div->row[j], div->n_col);
}

static __isl_give isl_mat *merge_divs(__isl_keep isl_mat *div1,
	__isl_keep isl_mat *div2, int *exp1, int *exp2)
{
	int i, j, k;
	isl_mat *div = NULL;
	unsigned d = div1->n_col - div1->n_row;

	div = isl_mat_alloc(div1->ctx, 1 + div1->n_row + div2->n_row,
				d + div1->n_row + div2->n_row);
	if (!div)
		return NULL;

	for (i = 0, j = 0, k = 0; i < div1->n_row && j < div2->n_row; ++k) {
		int cmp;

		expand_row(div, k, div1, i, exp1);
		expand_row(div, k + 1, div2, j, exp2);

		cmp = cmp_row(div, k, k + 1);
		if (cmp == 0) {
			exp1[i++] = k;
			exp2[j++] = k;
		} else if (cmp < 0) {
			exp1[i++] = k;
		} else {
			exp2[j++] = k;
			isl_seq_cpy(div->row[k], div->row[k + 1], div->n_col);
		}
	}
	for (; i < div1->n_row; ++i, ++k) {
		expand_row(div, k, div1, i, exp1);
		exp1[i] = k;
	}
	for (; j < div2->n_row; ++j, ++k) {
		expand_row(div, k, div2, j, exp2);
		exp2[j] = k;
	}

	div->n_row = k;
	div->n_col = d + k;

	return div;
}

static __isl_give struct isl_upoly *expand(__isl_take struct isl_upoly *up,
	int *exp, int first)
{
	int i;
	struct isl_upoly_rec *rec;

	if (isl_upoly_is_cst(up))
		return up;

	if (up->var < first)
		return up;

	if (exp[up->var - first] == up->var - first)
		return up;

	up = isl_upoly_cow(up);
	if (!up)
		goto error;

	up->var = exp[up->var - first] + first;

	rec = isl_upoly_as_rec(up);
	if (!rec)
		goto error;

	for (i = 0; i < rec->n; ++i) {
		rec->p[i] = expand(rec->p[i], exp, first);
		if (!rec->p[i])
			goto error;
	}

	return up;
error:
	isl_upoly_free(up);
	return NULL;
}

static __isl_give isl_qpolynomial *with_merged_divs(
	__isl_give isl_qpolynomial *(*fn)(__isl_take isl_qpolynomial *qp1,
					  __isl_take isl_qpolynomial *qp2),
	__isl_take isl_qpolynomial *qp1, __isl_take isl_qpolynomial *qp2)
{
	int *exp1 = NULL;
	int *exp2 = NULL;
	isl_mat *div = NULL;

	qp1 = isl_qpolynomial_cow(qp1);
	qp2 = isl_qpolynomial_cow(qp2);

	if (!qp1 || !qp2)
		goto error;

	isl_assert(qp1->div->ctx, qp1->div->n_row >= qp2->div->n_row &&
				qp1->div->n_col >= qp2->div->n_col, goto error);

	exp1 = isl_alloc_array(qp1->div->ctx, int, qp1->div->n_row);
	exp2 = isl_alloc_array(qp2->div->ctx, int, qp2->div->n_row);
	if (!exp1 || !exp2)
		goto error;

	div = merge_divs(qp1->div, qp2->div, exp1, exp2);
	if (!div)
		goto error;

	isl_mat_free(qp1->div);
	qp1->div = isl_mat_copy(div);
	isl_mat_free(qp2->div);
	qp2->div = isl_mat_copy(div);

	qp1->upoly = expand(qp1->upoly, exp1, div->n_col - div->n_row - 2);
	qp2->upoly = expand(qp2->upoly, exp2, div->n_col - div->n_row - 2);

	if (!qp1->upoly || !qp2->upoly)
		goto error;

	isl_mat_free(div);
	free(exp1);
	free(exp2);

	return fn(qp1, qp2);
error:
	isl_mat_free(div);
	free(exp1);
	free(exp2);
	isl_qpolynomial_free(qp1);
	isl_qpolynomial_free(qp2);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_add(__isl_take isl_qpolynomial *qp1,
	__isl_take isl_qpolynomial *qp2)
{
	qp1 = isl_qpolynomial_cow(qp1);

	if (!qp1 || !qp2)
		goto error;

	if (qp1->div->n_row < qp2->div->n_row)
		return isl_qpolynomial_add(qp2, qp1);

	isl_assert(qp1->dim->ctx, isl_dim_equal(qp1->dim, qp2->dim), goto error);
	if (!compatible_divs(qp1->div, qp2->div))
		return with_merged_divs(isl_qpolynomial_add, qp1, qp2);

	qp1->upoly = isl_upoly_sum(qp1->upoly, isl_upoly_copy(qp2->upoly));
	if (!qp1->upoly)
		goto error;

	isl_qpolynomial_free(qp2);

	return qp1;
error:
	isl_qpolynomial_free(qp1);
	isl_qpolynomial_free(qp2);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_sub(__isl_take isl_qpolynomial *qp1,
	__isl_take isl_qpolynomial *qp2)
{
	return isl_qpolynomial_add(qp1, isl_qpolynomial_neg(qp2));
}

__isl_give isl_qpolynomial *isl_qpolynomial_neg(__isl_take isl_qpolynomial *qp)
{
	qp = isl_qpolynomial_cow(qp);

	if (!qp)
		return NULL;

	qp->upoly = isl_upoly_neg(qp->upoly);
	if (!qp->upoly)
		goto error;

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_mul(__isl_take isl_qpolynomial *qp1,
	__isl_take isl_qpolynomial *qp2)
{
	qp1 = isl_qpolynomial_cow(qp1);

	if (!qp1 || !qp2)
		goto error;

	if (qp1->div->n_row < qp2->div->n_row)
		return isl_qpolynomial_mul(qp2, qp1);

	isl_assert(qp1->dim->ctx, isl_dim_equal(qp1->dim, qp2->dim), goto error);
	if (!compatible_divs(qp1->div, qp2->div))
		return with_merged_divs(isl_qpolynomial_mul, qp1, qp2);

	qp1->upoly = isl_upoly_mul(qp1->upoly, isl_upoly_copy(qp2->upoly));
	if (!qp1->upoly)
		goto error;

	isl_qpolynomial_free(qp2);

	return qp1;
error:
	isl_qpolynomial_free(qp1);
	isl_qpolynomial_free(qp2);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_zero(__isl_take isl_dim *dim)
{
	struct isl_qpolynomial *qp;
	struct isl_upoly_cst *cst;

	qp = isl_qpolynomial_alloc(dim, 0);
	if (!qp)
		return NULL;

	qp->upoly = isl_upoly_zero(dim->ctx);
	if (!qp->upoly)
		goto error;

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_infty(__isl_take isl_dim *dim)
{
	struct isl_qpolynomial *qp;
	struct isl_upoly_cst *cst;

	qp = isl_qpolynomial_alloc(dim, 0);
	if (!qp)
		return NULL;

	qp->upoly = isl_upoly_infty(dim->ctx);
	if (!qp->upoly)
		goto error;

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_nan(__isl_take isl_dim *dim)
{
	struct isl_qpolynomial *qp;
	struct isl_upoly_cst *cst;

	qp = isl_qpolynomial_alloc(dim, 0);
	if (!qp)
		return NULL;

	qp->upoly = isl_upoly_nan(dim->ctx);
	if (!qp->upoly)
		goto error;

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_cst(__isl_take isl_dim *dim,
	isl_int v)
{
	struct isl_qpolynomial *qp;
	struct isl_upoly_cst *cst;

	qp = isl_qpolynomial_alloc(dim, 0);
	if (!qp)
		return NULL;

	qp->upoly = isl_upoly_zero(dim->ctx);
	if (!qp->upoly)
		goto error;
	cst = isl_upoly_as_cst(qp->upoly);
	isl_int_set(cst->n, v);

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

int isl_qpolynomial_is_cst(__isl_keep isl_qpolynomial *qp,
	isl_int *n, isl_int *d)
{
	struct isl_upoly_cst *cst;

	if (!qp)
		return -1;

	if (!isl_upoly_is_cst(qp->upoly))
		return 0;

	cst = isl_upoly_as_cst(qp->upoly);
	if (!cst)
		return -1;

	if (n)
		isl_int_set(*n, cst->n);
	if (d)
		isl_int_set(*d, cst->d);

	return 1;
}

__isl_give isl_qpolynomial *isl_qpolynomial_pow(__isl_take isl_dim *dim,
	int pos, int power)
{
	int i;
	struct isl_qpolynomial *qp;
	struct isl_upoly_rec *rec;
	struct isl_upoly_cst *cst;
	struct isl_ctx *ctx;

	if (!dim)
		return NULL;

	ctx = dim->ctx;

	qp = isl_qpolynomial_alloc(dim, 0);
	if (!qp)
		return NULL;

	qp->upoly = &isl_upoly_alloc_rec(ctx, pos, 1 + power)->up;
	if (!qp->upoly)
		goto error;
	rec = isl_upoly_as_rec(qp->upoly);
	for (i = 0; i < 1 + power; ++i) {
		rec->p[i] = isl_upoly_zero(ctx);
		if (!rec->p[i])
			goto error;
		rec->n++;
	}
	cst = isl_upoly_as_cst(rec->p[power]);
	isl_int_set_si(cst->n, 1);

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_var(__isl_take isl_dim *dim,
	enum isl_dim_type type, unsigned pos)
{
	if (!dim)
		return NULL;

	isl_assert(dim->ctx, isl_dim_size(dim, isl_dim_in) == 0, goto error);
	isl_assert(dim->ctx, pos < isl_dim_size(dim, type), goto error);

	if (type == isl_dim_set)
		pos += isl_dim_size(dim, isl_dim_param);

	return isl_qpolynomial_pow(dim, pos, 1);
error:
	isl_dim_free(dim);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_div_pow(__isl_take isl_div *div,
	int power)
{
	struct isl_qpolynomial *qp = NULL;
	struct isl_upoly_rec *rec;
	struct isl_upoly_cst *cst;
	int i;
	int pos;

	if (!div)
		return NULL;
	isl_assert(div->ctx, div->bmap->n_div == 1, goto error);

	qp = isl_qpolynomial_alloc(isl_basic_map_get_dim(div->bmap), 1);
	if (!qp)
		goto error;

	isl_seq_cpy(qp->div->row[0], div->line[0], qp->div->n_col - 1);
	isl_int_set_si(qp->div->row[0][qp->div->n_col - 1], 0);

	pos = isl_dim_total(qp->dim);
	qp->upoly = &isl_upoly_alloc_rec(div->ctx, pos, 1 + power)->up;
	if (!qp->upoly)
		goto error;
	rec = isl_upoly_as_rec(qp->upoly);
	for (i = 0; i < 1 + power; ++i) {
		rec->p[i] = isl_upoly_zero(div->ctx);
		if (!rec->p[i])
			goto error;
		rec->n++;
	}
	cst = isl_upoly_as_cst(rec->p[power]);
	isl_int_set_si(cst->n, 1);

	isl_div_free(div);

	return qp;
error:
	isl_qpolynomial_free(qp);
	isl_div_free(div);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_div(__isl_take isl_div *div)
{
	return isl_qpolynomial_div_pow(div, 1);
}

__isl_give isl_qpolynomial *isl_qpolynomial_rat_cst(__isl_take isl_dim *dim,
	const isl_int n, const isl_int d)
{
	struct isl_qpolynomial *qp;
	struct isl_upoly_cst *cst;

	qp = isl_qpolynomial_alloc(dim, 0);
	if (!qp)
		return NULL;

	qp->upoly = isl_upoly_zero(dim->ctx);
	if (!qp->upoly)
		goto error;
	cst = isl_upoly_as_cst(qp->upoly);
	isl_int_set(cst->n, n);
	isl_int_set(cst->d, d);

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

static __isl_give isl_pw_qpolynomial *pw_qpolynomial_alloc(__isl_take isl_dim *dim,
	int n)
{
	struct isl_pw_qpolynomial *pwqp;

	if (!dim)
		return NULL;
	isl_assert(dim->ctx, n >= 0, return NULL);
	pwqp = isl_alloc(dim->ctx, struct isl_pw_qpolynomial,
			sizeof(struct isl_pw_qpolynomial) +
			(n - 1) * sizeof(struct isl_pw_qpolynomial_piece));
	if (!pwqp)
		goto error;

	pwqp->ref = 1;
	pwqp->size = n;
	pwqp->n = 0;
	pwqp->dim = dim;
	return pwqp;
error:
	isl_dim_free(dim);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_add_piece(
	__isl_take isl_pw_qpolynomial *pwqp,
	__isl_take isl_set *set, __isl_take isl_qpolynomial *qp)
{
	if (!pwqp || !set || !qp)
		goto error;

	if (isl_set_fast_is_empty(set) || isl_qpolynomial_is_zero(qp)) {
		isl_set_free(set);
		isl_qpolynomial_free(qp);
		return pwqp;
	}

	isl_assert(set->ctx, isl_dim_equal(pwqp->dim, qp->dim), goto error);
	isl_assert(set->ctx, pwqp->n < pwqp->size, goto error);

	pwqp->p[pwqp->n].set = set;
	pwqp->p[pwqp->n].qp = qp;
	pwqp->n++;
	
	return pwqp;
error:
	isl_pw_qpolynomial_free(pwqp);
	isl_set_free(set);
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_alloc(__isl_take isl_set *set,
	__isl_take isl_qpolynomial *qp)
{
	struct isl_pw_qpolynomial *pwqp;

	if (!set || !qp)
		goto error;

	pwqp = pw_qpolynomial_alloc(isl_set_get_dim(set), 1);

	return isl_pw_qpolynomial_add_piece(pwqp, set, qp);
error:
	isl_set_free(set);
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_zero(__isl_take isl_dim *dim)
{
	return pw_qpolynomial_alloc(dim, 0);
}

int isl_pw_qpolynomial_is_zero(__isl_keep isl_pw_qpolynomial *pwqp)
{
	if (!pwqp)
		return -1;

	return pwqp->n == 0;
}

int isl_pw_qpolynomial_is_one(__isl_keep isl_pw_qpolynomial *pwqp)
{
	if (!pwqp)
		return -1;

	if (pwqp->n != -1)
		return 0;

	if (!isl_set_fast_is_universe(pwqp->p[0].set))
		return 0;

	return isl_qpolynomial_is_one(pwqp->p[0].qp);
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_copy(
	__isl_keep isl_pw_qpolynomial *pwqp)
{
	if (!pwqp)
		return;

	pwqp->ref++;
	return pwqp;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_dup(
	__isl_keep isl_pw_qpolynomial *pwqp)
{
	int i;
	struct isl_pw_qpolynomial *dup;

	if (!pwqp)
		return NULL;

	dup = pw_qpolynomial_alloc(isl_dim_copy(pwqp->dim), pwqp->n);
	if (!dup)
		return NULL;

	for (i = 0; i < pwqp->n; ++i)
		dup = isl_pw_qpolynomial_add_piece(dup,
					    isl_set_copy(pwqp->p[i].set),
					    isl_qpolynomial_copy(pwqp->p[i].qp));

	return dup;
error:
	isl_pw_qpolynomial_free(dup);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_cow(
	__isl_take isl_pw_qpolynomial *pwqp)
{
	if (!pwqp)
		return NULL;

	if (pwqp->ref == 1)
		return pwqp;
	pwqp->ref--;
	return isl_pw_qpolynomial_dup(pwqp);
}

void isl_pw_qpolynomial_free(__isl_take isl_pw_qpolynomial *pwqp)
{
	int i;

	if (!pwqp)
		return;
	if (--pwqp->ref > 0)
		return;

	for (i = 0; i < pwqp->n; ++i) {
		isl_set_free(pwqp->p[i].set);
		isl_qpolynomial_free(pwqp->p[i].qp);
	}
	isl_dim_free(pwqp->dim);
	free(pwqp);
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_add(
	__isl_take isl_pw_qpolynomial *pwqp1,
	__isl_take isl_pw_qpolynomial *pwqp2)
{
	int i, j, n;
	struct isl_pw_qpolynomial *res;
	isl_set *set;

	if (!pwqp1 || !pwqp2)
		goto error;

	isl_assert(pwqp1->dim->ctx, isl_dim_equal(pwqp1->dim, pwqp2->dim),
			goto error);

	if (isl_pw_qpolynomial_is_zero(pwqp1)) {
		isl_pw_qpolynomial_free(pwqp1);
		return pwqp2;
	}

	if (isl_pw_qpolynomial_is_zero(pwqp2)) {
		isl_pw_qpolynomial_free(pwqp2);
		return pwqp1;
	}

	n = (pwqp1->n + 1) * (pwqp2->n + 1);
	res = pw_qpolynomial_alloc(isl_dim_copy(pwqp1->dim), n);

	for (i = 0; i < pwqp1->n; ++i) {
		set = isl_set_copy(pwqp1->p[i].set);
		for (j = 0; j < pwqp2->n; ++j) {
			struct isl_set *common;
			struct isl_qpolynomial *sum;
			set = isl_set_subtract(set,
					isl_set_copy(pwqp2->p[j].set));
			common = isl_set_intersect(isl_set_copy(pwqp1->p[i].set),
						isl_set_copy(pwqp2->p[j].set));
			if (isl_set_fast_is_empty(common)) {
				isl_set_free(common);
				continue;
			}

			sum = isl_qpolynomial_add(
				isl_qpolynomial_copy(pwqp1->p[i].qp),
				isl_qpolynomial_copy(pwqp2->p[j].qp));

			res = isl_pw_qpolynomial_add_piece(res, common, sum);
		}
		res = isl_pw_qpolynomial_add_piece(res, set,
			isl_qpolynomial_copy(pwqp1->p[i].qp));
	}

	for (j = 0; j < pwqp2->n; ++j) {
		set = isl_set_copy(pwqp2->p[j].set);
		for (i = 0; i < pwqp1->n; ++i)
			set = isl_set_subtract(set,
					isl_set_copy(pwqp1->p[i].set));
		res = isl_pw_qpolynomial_add_piece(res, set,
			isl_qpolynomial_copy(pwqp2->p[j].qp));
	}

	isl_pw_qpolynomial_free(pwqp1);
	isl_pw_qpolynomial_free(pwqp2);

	return res;
error:
	isl_pw_qpolynomial_free(pwqp1);
	isl_pw_qpolynomial_free(pwqp2);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_add_disjoint(
	__isl_take isl_pw_qpolynomial *pwqp1,
	__isl_take isl_pw_qpolynomial *pwqp2)
{
	int i;
	struct isl_pw_qpolynomial *res;

	if (!pwqp1 || !pwqp2)
		goto error;

	isl_assert(pwqp1->dim->ctx, isl_dim_equal(pwqp1->dim, pwqp2->dim),
			goto error);

	if (isl_pw_qpolynomial_is_zero(pwqp1)) {
		isl_pw_qpolynomial_free(pwqp1);
		return pwqp2;
	}

	if (isl_pw_qpolynomial_is_zero(pwqp2)) {
		isl_pw_qpolynomial_free(pwqp2);
		return pwqp1;
	}

	res = pw_qpolynomial_alloc(isl_dim_copy(pwqp1->dim), pwqp1->n + pwqp2->n);

	for (i = 0; i < pwqp1->n; ++i)
		res = isl_pw_qpolynomial_add_piece(res,
				isl_set_copy(pwqp1->p[i].set),
				isl_qpolynomial_copy(pwqp1->p[i].qp));

	for (i = 0; i < pwqp2->n; ++i)
		res = isl_pw_qpolynomial_add_piece(res,
				isl_set_copy(pwqp2->p[i].set),
				isl_qpolynomial_copy(pwqp2->p[i].qp));

	isl_pw_qpolynomial_free(pwqp1);
	isl_pw_qpolynomial_free(pwqp2);

	return res;
error:
	isl_pw_qpolynomial_free(pwqp1);
	isl_pw_qpolynomial_free(pwqp2);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_mul(
	__isl_take isl_pw_qpolynomial *pwqp1,
	__isl_take isl_pw_qpolynomial *pwqp2)
{
	int i, j, n;
	struct isl_pw_qpolynomial *res;
	isl_set *set;

	if (!pwqp1 || !pwqp2)
		goto error;

	isl_assert(pwqp1->dim->ctx, isl_dim_equal(pwqp1->dim, pwqp2->dim),
			goto error);

	if (isl_pw_qpolynomial_is_zero(pwqp1)) {
		isl_pw_qpolynomial_free(pwqp2);
		return pwqp1;
	}

	if (isl_pw_qpolynomial_is_zero(pwqp2)) {
		isl_pw_qpolynomial_free(pwqp1);
		return pwqp2;
	}

	if (isl_pw_qpolynomial_is_one(pwqp1)) {
		isl_pw_qpolynomial_free(pwqp1);
		return pwqp2;
	}

	if (isl_pw_qpolynomial_is_one(pwqp2)) {
		isl_pw_qpolynomial_free(pwqp2);
		return pwqp1;
	}

	n = pwqp1->n * pwqp2->n;
	res = pw_qpolynomial_alloc(isl_dim_copy(pwqp1->dim), n);

	for (i = 0; i < pwqp1->n; ++i) {
		for (j = 0; j < pwqp2->n; ++j) {
			struct isl_set *common;
			struct isl_qpolynomial *prod;
			common = isl_set_intersect(isl_set_copy(pwqp1->p[i].set),
						isl_set_copy(pwqp2->p[j].set));
			if (isl_set_fast_is_empty(common)) {
				isl_set_free(common);
				continue;
			}

			prod = isl_qpolynomial_mul(
				isl_qpolynomial_copy(pwqp1->p[i].qp),
				isl_qpolynomial_copy(pwqp2->p[j].qp));

			res = isl_pw_qpolynomial_add_piece(res, common, prod);
		}
	}

	isl_pw_qpolynomial_free(pwqp1);
	isl_pw_qpolynomial_free(pwqp2);

	return res;
error:
	isl_pw_qpolynomial_free(pwqp1);
	isl_pw_qpolynomial_free(pwqp2);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_neg(
	__isl_take isl_pw_qpolynomial *pwqp)
{
	int i, j, n;
	struct isl_pw_qpolynomial *res;
	isl_set *set;

	if (!pwqp)
		return NULL;

	if (isl_pw_qpolynomial_is_zero(pwqp))
		return pwqp;

	pwqp = isl_pw_qpolynomial_cow(pwqp);

	for (i = 0; i < pwqp->n; ++i) {
		pwqp->p[i].qp = isl_qpolynomial_neg(pwqp->p[i].qp);
		if (!pwqp->p[i].qp)
			goto error;
	}

	return pwqp;
error:
	isl_pw_qpolynomial_free(pwqp);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_sub(
	__isl_take isl_pw_qpolynomial *pwqp1,
	__isl_take isl_pw_qpolynomial *pwqp2)
{
	return isl_pw_qpolynomial_add(pwqp1, isl_pw_qpolynomial_neg(pwqp2));
}

__isl_give struct isl_upoly *isl_upoly_eval(
	__isl_take struct isl_upoly *up, __isl_take isl_vec *vec)
{
	int i;
	struct isl_upoly_rec *rec;
	struct isl_upoly *res;
	struct isl_upoly *base;

	if (isl_upoly_is_cst(up)) {
		isl_vec_free(vec);
		return up;
	}

	rec = isl_upoly_as_rec(up);
	if (!rec)
		goto error;

	isl_assert(up->ctx, rec->n >= 1, goto error);

	base = isl_upoly_rat_cst(up->ctx, vec->el[1 + up->var], vec->el[0]);

	res = isl_upoly_eval(isl_upoly_copy(rec->p[rec->n - 1]),
				isl_vec_copy(vec));

	for (i = rec->n - 2; i >= 0; --i) {
		res = isl_upoly_mul(res, isl_upoly_copy(base));
		res = isl_upoly_sum(res, 
			    isl_upoly_eval(isl_upoly_copy(rec->p[i]),
							    isl_vec_copy(vec)));
	}

	isl_upoly_free(base);
	isl_upoly_free(up);
	isl_vec_free(vec);
	return res;
error:
	isl_upoly_free(up);
	isl_vec_free(vec);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_eval(
	__isl_take isl_qpolynomial *qp, __isl_take isl_point *pnt)
{
	isl_vec *ext;
	struct isl_upoly *up;

	if (!qp || !pnt)
		goto error;
	isl_assert(pnt->dim->ctx, isl_dim_equal(pnt->dim, qp->dim), goto error);

	if (qp->div->n_row == 0)
		ext = isl_vec_copy(pnt->vec);
	else {
		int i;
		unsigned dim = isl_dim_total(qp->dim);
		ext = isl_vec_alloc(qp->dim->ctx, 1 + dim + qp->div->n_row);
		if (!ext)
			goto error;

		isl_seq_cpy(ext->el, pnt->vec->el, pnt->vec->size);
		for (i = 0; i < qp->div->n_row; ++i) {
			isl_seq_inner_product(qp->div->row[i] + 1, ext->el,
						1 + dim + i, &ext->el[1+dim+i]);
			isl_int_fdiv_q(ext->el[1+dim+i], ext->el[1+dim+i],
					qp->div->row[i][0]);
		}
	}

	up = isl_upoly_eval(isl_upoly_copy(qp->upoly), ext);
	if (!up)
		goto error;

	isl_qpolynomial_free(qp);
	isl_point_free(pnt);

	qp = isl_qpolynomial_alloc(isl_dim_set_alloc(up->ctx, 0, 0), 0);
	if (!qp)
		isl_upoly_free(up);
	else
		qp->upoly = up;

	return qp;
error:
	isl_qpolynomial_free(qp);
	isl_point_free(pnt);
	return NULL;
}

__isl_give isl_qpolynomial *isl_pw_qpolynomial_eval(
	__isl_take isl_pw_qpolynomial *pwqp, __isl_take isl_point *pnt)
{
	int i;
	int found;
	isl_qpolynomial *qp;

	if (!pwqp || !pnt)
		goto error;
	isl_assert(pnt->dim->ctx, isl_dim_equal(pnt->dim, pwqp->dim), goto error);

	for (i = 0; i < pwqp->n; ++i) {
		found = isl_set_contains_point(pwqp->p[i].set, pnt);
		if (found < 0)
			goto error;
		if (found)
			break;
	}
	if (found)
		qp = isl_qpolynomial_eval(isl_qpolynomial_copy(pwqp->p[i].qp),
					    isl_point_copy(pnt));
	else
		qp = isl_qpolynomial_zero(isl_dim_copy(pwqp->dim));
	isl_pw_qpolynomial_free(pwqp);
	isl_point_free(pnt);
	return qp;
error:
	isl_pw_qpolynomial_free(pwqp);
	isl_point_free(pnt);
	return NULL;
}

__isl_give isl_qpolynomial *isl_qpolynomial_move(__isl_take isl_qpolynomial *qp,
	enum isl_dim_type dst_type, unsigned dst_pos,
	enum isl_dim_type src_type, unsigned src_pos, unsigned n)
{
	if (!qp)
		return NULL;

	qp->dim = isl_dim_move(qp->dim, dst_type, dst_pos, src_type, src_pos, n);
	if (!qp->dim)
		goto error;
	
	/* Do something to polynomials when needed; later */

	return qp;
error:
	isl_qpolynomial_free(qp);
	return NULL;
}

__isl_give isl_pw_qpolynomial *isl_pw_qpolynomial_move(
	__isl_take isl_pw_qpolynomial *pwqp,
	enum isl_dim_type dst_type, unsigned dst_pos,
	enum isl_dim_type src_type, unsigned src_pos, unsigned n)
{
	int i;

	if (!pwqp)
		return NULL;

	pwqp->dim = isl_dim_move(pwqp->dim,
				    dst_type, dst_pos, src_type, src_pos, n);
	if (!pwqp->dim)
		goto error;

	for (i = 0; i < pwqp->n; ++i) {
		pwqp->p[i].set = isl_set_move(pwqp->p[i].set, dst_type, dst_pos,
						src_type, src_pos, n);
		if (!pwqp->p[i].set)
			goto error;
		pwqp->p[i].qp = isl_qpolynomial_move(pwqp->p[i].qp,
					dst_type, dst_pos, src_type, src_pos, n);
		if (!pwqp->p[i].qp)
			goto error;
	}

	return pwqp;
error:
	isl_pw_qpolynomial_free(pwqp);
	return NULL;
}