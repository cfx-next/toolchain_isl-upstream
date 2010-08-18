/*
 * Copyright 2010      INRIA Saclay
 *
 * Use of this software is governed by the GNU LGPLv2.1 license
 *
 * Written by Sven Verdoolaege, INRIA Saclay - Ile-de-France,
 * Parc Club Orsay Universite, ZAC des vignes, 4 rue Jacques Monod,
 * 91893 Orsay, France 
 */

#include <isl_ctx.h>
#include <isl_hash.h>
#include <isl_map.h>
#include <isl_set.h>
#include <isl_dim_private.h>
#include <isl_union_map_private.h>
#include <isl_union_set.h>

static __isl_give isl_union_map *isl_union_map_alloc(__isl_take isl_dim *dim,
	int size)
{
	isl_union_map *umap;

	if (!dim)
		return NULL;

	umap = isl_calloc_type(dim->ctx, isl_union_map);
	if (!umap)
		return NULL;

	umap->ref = 1;
	umap->dim = dim;
	if (isl_hash_table_init(dim->ctx, &umap->table, size) < 0)
		goto error;

	return umap;
error:
	isl_dim_free(dim);
	isl_union_map_free(umap);
	return NULL;
}

__isl_give isl_union_map *isl_union_map_empty(__isl_take isl_dim *dim)
{
	return isl_union_map_alloc(dim, 16);
}

__isl_give isl_union_set *isl_union_set_empty(__isl_take isl_dim *dim)
{
	return isl_union_map_empty(dim);
}

isl_ctx *isl_union_map_get_ctx(__isl_keep isl_union_map *umap)
{
	return umap ? umap->dim->ctx : NULL;
}

isl_ctx *isl_union_set_get_ctx(__isl_keep isl_union_set *uset)
{
	return uset ? uset->dim->ctx : NULL;
}

__isl_give isl_dim *isl_union_map_get_dim(__isl_keep isl_union_map *umap)
{
	if (!umap)
		return NULL;
	return isl_dim_copy(umap->dim);
}

__isl_give isl_dim *isl_union_set_get_dim(__isl_keep isl_union_set *uset)
{
	return isl_union_map_get_dim(uset);
}

static int free_umap_entry(void **entry, void *user)
{
	isl_map *map = *entry;
	isl_map_free(map);
	return 0;
}

static int add_map(__isl_take isl_map *map, void *user)
{
	isl_union_map **umap = (isl_union_map **)user;

	*umap = isl_union_map_add_map(*umap, map);

	return 0;
}

__isl_give isl_union_map *isl_union_map_dup(__isl_keep isl_union_map *umap)
{
	isl_union_map *dup;

	if (!umap)
		return NULL;

	dup = isl_union_map_empty(isl_dim_copy(umap->dim));
	if (isl_union_map_foreach_map(umap, &add_map, &dup) < 0)
		goto error;
	return dup;
error:
	isl_union_map_free(dup);
	return NULL;
}

__isl_give isl_union_map *isl_union_map_cow(__isl_take isl_union_map *umap)
{
	if (!umap)
		return NULL;

	if (umap->ref == 1)
		return umap;
	umap->ref--;
	return isl_union_map_dup(umap);
}

__isl_give isl_union_map *isl_union_map_union(__isl_take isl_union_map *umap1,
	__isl_take isl_union_map *umap2)
{
	umap1 = isl_union_map_cow(umap1);

	if (!umap1 || !umap2)
		goto error;

	if (isl_union_map_foreach_map(umap2, &add_map, &umap1) < 0)
		goto error;

	isl_union_map_free(umap2);

	return umap1;
error:
	isl_union_map_free(umap1);
	isl_union_map_free(umap2);
	return NULL;
}

__isl_give isl_union_set *isl_union_set_union(__isl_take isl_union_set *uset1,
	__isl_take isl_union_set *uset2)
{
	return isl_union_map_union(uset1, uset2);
}

__isl_give isl_union_map *isl_union_map_copy(__isl_keep isl_union_map *umap)
{
	if (!umap)
		return NULL;

	umap->ref++;
	return umap;
}

__isl_give isl_union_set *isl_union_set_copy(__isl_keep isl_union_set *uset)
{
	return isl_union_map_copy(uset);
}

void isl_union_map_free(__isl_take isl_union_map *umap)
{
	if (!umap)
		return;

	if (--umap->ref > 0)
		return;

	isl_hash_table_foreach(umap->dim->ctx, &umap->table,
			       &free_umap_entry, NULL);
	isl_hash_table_clear(&umap->table);
	isl_dim_free(umap->dim);
	free(umap);
}

void isl_union_set_free(__isl_take isl_union_set *uset)
{
	isl_union_map_free(uset);
}

static int has_dim(const void *entry, const void *val)
{
	isl_map *map = (isl_map *)entry;
	isl_dim *dim = (isl_dim *)val;

	return isl_dim_equal(map->dim, dim);
}

__isl_give isl_union_map *isl_union_map_add_map(__isl_take isl_union_map *umap,
	__isl_take isl_map *map)
{
	uint32_t hash;
	struct isl_hash_table_entry *entry;

	if (isl_map_fast_is_empty(map)) {
		isl_map_free(map);
		return umap;
	}

	umap = isl_union_map_cow(umap);

	if (!map || !umap)
		goto error;

	isl_assert(map->ctx, isl_dim_match(map->dim, isl_dim_param, umap->dim,
					   isl_dim_param), goto error);

	hash = isl_dim_get_hash(map->dim);
	entry = isl_hash_table_find(umap->dim->ctx, &umap->table, hash,
				    &has_dim, map->dim, 1);
	if (!entry)
		goto error;

	if (!entry->data)
		entry->data = map;
	else {
		entry->data = isl_map_union(entry->data, isl_map_copy(map));
		if (!entry->data)
			goto error;
		isl_map_free(map);
	}

	return umap;
error:
	isl_map_free(map);
	isl_union_map_free(umap);
	return NULL;
}

__isl_give isl_union_set *isl_union_set_add_set(__isl_take isl_union_set *uset,
	__isl_take isl_set *set)
{
	return isl_union_map_add_map(uset, (isl_map *)set);
}

__isl_give isl_union_map *isl_union_map_from_map(__isl_take isl_map *map)
{
	isl_dim *dim;
	isl_union_map *umap;

	if (!map)
		return NULL;

	dim = isl_map_get_dim(map);
	dim = isl_dim_drop(dim, isl_dim_in, 0, isl_dim_size(dim, isl_dim_in));
	dim = isl_dim_drop(dim, isl_dim_out, 0, isl_dim_size(dim, isl_dim_out));
	umap = isl_union_map_empty(dim);
	umap = isl_union_map_add_map(umap, map);

	return umap;
}

__isl_give isl_union_set *isl_union_set_from_set(__isl_take isl_set *set)
{
	return isl_union_map_from_map((isl_map *)set);
}

struct isl_union_map_foreach_data
{
	int (*fn)(__isl_take isl_map *map, void *user);
	void *user;
};

static int call_on_copy(void **entry, void *user)
{
	isl_map *map = *entry;
	struct isl_union_map_foreach_data *data;
	data = (struct isl_union_map_foreach_data *)user;

	return data->fn(isl_map_copy(map), data->user);
}

int isl_union_map_foreach_map(__isl_keep isl_union_map *umap,
	int (*fn)(__isl_take isl_map *map, void *user), void *user)
{
	struct isl_union_map_foreach_data data = { fn, user };

	if (!umap)
		return -1;

	return isl_hash_table_foreach(umap->dim->ctx, &umap->table,
				      &call_on_copy, &data);
}

int isl_union_set_foreach_set(__isl_keep isl_union_set *uset,
	int (*fn)(__isl_take isl_set *set, void *user), void *user)
{
	return isl_union_map_foreach_map(uset,
		(int(*)(__isl_take isl_map *, void*))fn, user);
}

struct isl_union_set_foreach_point_data {
	int (*fn)(__isl_take isl_point *pnt, void *user);
	void *user;
};

static int foreach_point(__isl_take isl_set *set, void *user)
{
	struct isl_union_set_foreach_point_data *data = user;
	int r;

	r = isl_set_foreach_point(set, data->fn, data->user);
	isl_set_free(set);

	return r;
}

int isl_union_set_foreach_point(__isl_keep isl_union_set *uset,
	int (*fn)(__isl_take isl_point *pnt, void *user), void *user)
{
	struct isl_union_set_foreach_point_data data = { fn, user };
	return isl_union_set_foreach_set(uset, &foreach_point, &data);
}

struct isl_union_map_gen_bin_data {
	isl_union_map *umap2;
	isl_union_map *res;
};

static int subtract_entry(void **entry, void *user)
{
	struct isl_union_map_gen_bin_data *data = user;
	uint32_t hash;
	struct isl_hash_table_entry *entry2;
	isl_map *map = *entry;

	hash = isl_dim_get_hash(map->dim);
	entry2 = isl_hash_table_find(data->umap2->dim->ctx, &data->umap2->table,
				     hash, &has_dim, map->dim, 0);
	map = isl_map_copy(map);
	if (entry2) {
		int empty;
		map = isl_map_subtract(map, isl_map_copy(entry2->data));

		empty = isl_map_is_empty(map);
		if (empty < 0) {
			isl_map_free(map);
			return -1;
		}
		if (empty) {
			isl_map_free(map);
			return 0;
		}
	}
	data->res = isl_union_map_add_map(data->res, map);

	return 0;
}

static __isl_give isl_union_map *gen_bin_op(__isl_take isl_union_map *umap1,
	__isl_take isl_union_map *umap2, int (*fn)(void **, void *))
{
	struct isl_union_map_gen_bin_data data = { umap2, NULL };

	if (!umap1 || !umap2)
		goto error;

	data.res = isl_union_map_alloc(isl_dim_copy(umap1->dim),
				       umap1->table.n);
	if (isl_hash_table_foreach(umap1->dim->ctx, &umap1->table,
				   fn, &data) < 0)
		goto error;

	isl_union_map_free(umap1);
	isl_union_map_free(umap2);
	return data.res;
error:
	isl_union_map_free(umap1);
	isl_union_map_free(umap2);
	isl_union_map_free(data.res);
	return NULL;
}

__isl_give isl_union_map *isl_union_map_subtract(
	__isl_take isl_union_map *umap1, __isl_take isl_union_map *umap2)
{
	return gen_bin_op(umap1, umap2, &subtract_entry);
}

__isl_give isl_union_set *isl_union_set_subtract(
	__isl_take isl_union_set *uset1, __isl_take isl_union_set *uset2)
{
	return isl_union_map_subtract(uset1, uset2);
}

struct isl_union_map_match_bin_data {
	isl_union_map *umap2;
	isl_union_map *res;
	__isl_give isl_map *(*fn)(__isl_take isl_map*, __isl_take isl_map*);
};

static int match_bin_entry(void **entry, void *user)
{
	struct isl_union_map_match_bin_data *data = user;
	uint32_t hash;
	struct isl_hash_table_entry *entry2;
	isl_map *map = *entry;
	int empty;

	hash = isl_dim_get_hash(map->dim);
	entry2 = isl_hash_table_find(data->umap2->dim->ctx, &data->umap2->table,
				     hash, &has_dim, map->dim, 0);
	if (!entry2)
		return 0;

	map = isl_map_copy(map);
	map = data->fn(map, isl_map_copy(entry2->data));

	empty = isl_map_is_empty(map);
	if (empty < 0) {
		isl_map_free(map);
		return -1;
	}
	if (empty) {
		isl_map_free(map);
		return 0;
	}

	data->res = isl_union_map_add_map(data->res, map);

	return 0;
}

static __isl_give isl_union_map *match_bin_op(__isl_take isl_union_map *umap1,
	__isl_take isl_union_map *umap2,
	__isl_give isl_map *(*fn)(__isl_take isl_map*, __isl_take isl_map*))
{
	struct isl_union_map_match_bin_data data = { umap2, NULL, fn };

	if (!umap1 || !umap2)
		goto error;

	data.res = isl_union_map_alloc(isl_dim_copy(umap1->dim),
				       umap1->table.n);
	if (isl_hash_table_foreach(umap1->dim->ctx, &umap1->table,
				   &match_bin_entry, &data) < 0)
		goto error;

	isl_union_map_free(umap1);
	isl_union_map_free(umap2);
	return data.res;
error:
	isl_union_map_free(umap1);
	isl_union_map_free(umap2);
	isl_union_map_free(data.res);
	return NULL;
}

__isl_give isl_union_map *isl_union_map_intersect(
	__isl_take isl_union_map *umap1, __isl_take isl_union_map *umap2)
{
	return match_bin_op(umap1, umap2, &isl_map_intersect);
}

__isl_give isl_union_set *isl_union_set_intersect(
	__isl_take isl_union_set *uset1, __isl_take isl_union_set *uset2)
{
	return isl_union_map_intersect(uset1, uset2);
}

__isl_give isl_union_map *isl_union_map_gist(__isl_take isl_union_map *umap,
	__isl_take isl_union_map *context)
{
	return match_bin_op(umap, context, &isl_map_gist);
}

__isl_give isl_union_set *isl_union_set_gist(__isl_take isl_union_set *uset,
	__isl_take isl_union_set *context)
{
	return isl_union_map_gist(uset, context);
}

static int intersect_domain_entry(void **entry, void *user)
{
	struct isl_union_map_gen_bin_data *data = user;
	uint32_t hash;
	struct isl_hash_table_entry *entry2;
	isl_dim *dim;
	isl_map *map = *entry;
	int empty;

	dim = isl_map_get_dim(map);
	dim = isl_dim_domain(dim);
	hash = isl_dim_get_hash(dim);
	entry2 = isl_hash_table_find(data->umap2->dim->ctx, &data->umap2->table,
				     hash, &has_dim, dim, 0);
	isl_dim_free(dim);
	if (!entry2)
		return 0;

	map = isl_map_copy(map);
	map = isl_map_intersect_domain(map, isl_set_copy(entry2->data));

	empty = isl_map_is_empty(map);
	if (empty < 0) {
		isl_map_free(map);
		return -1;
	}
	if (empty) {
		isl_map_free(map);
		return 0;
	}

	data->res = isl_union_map_add_map(data->res, map);

	return 0;
}

__isl_give isl_union_map *isl_union_map_intersect_domain(
	__isl_take isl_union_map *umap, __isl_take isl_union_set *uset)
{
	return gen_bin_op(umap, uset, &intersect_domain_entry);
}

struct isl_union_map_bin_data {
	isl_union_map *umap2;
	isl_union_map *res;
	isl_map *map;
	int (*fn)(void **entry, void *user);
};

static int apply_range_entry(void **entry, void *user)
{
	struct isl_union_map_bin_data *data = user;
	isl_map *map2 = *entry;
	int empty;

	if (!isl_dim_tuple_match(data->map->dim, isl_dim_out,
				 map2->dim, isl_dim_in))
		return 0;

	map2 = isl_map_apply_range(isl_map_copy(data->map), isl_map_copy(map2));

	empty = isl_map_is_empty(map2);
	if (empty < 0) {
		isl_map_free(map2);
		return -1;
	}
	if (empty) {
		isl_map_free(map2);
		return 0;
	}

	data->res = isl_union_map_add_map(data->res, map2);

	return 0;
}

static int bin_entry(void **entry, void *user)
{
	struct isl_union_map_bin_data *data = user;
	isl_map *map = *entry;

	data->map = map;
	if (isl_hash_table_foreach(data->umap2->dim->ctx, &data->umap2->table,
				   data->fn, data) < 0)
		return -1;

	return 0;
}

static __isl_give isl_union_map *bin_op(__isl_take isl_union_map *umap1,
	__isl_take isl_union_map *umap2, int (*fn)(void **entry, void *user))
{
	struct isl_union_map_bin_data data = { umap2, NULL, NULL, fn };

	if (!umap1 || !umap2)
		goto error;

	data.res = isl_union_map_alloc(isl_dim_copy(umap1->dim),
				       umap1->table.n);
	if (isl_hash_table_foreach(umap1->dim->ctx, &umap1->table,
				   &bin_entry, &data) < 0)
		goto error;

	isl_union_map_free(umap1);
	isl_union_map_free(umap2);
	return data.res;
error:
	isl_union_map_free(umap1);
	isl_union_map_free(umap2);
	isl_union_map_free(data.res);
	return NULL;
}

__isl_give isl_union_map *isl_union_map_apply_range(
	__isl_take isl_union_map *umap1, __isl_take isl_union_map *umap2)
{
	return bin_op(umap1, umap2, &apply_range_entry);
}

__isl_give isl_union_set *isl_union_set_apply(
	__isl_take isl_union_set *uset, __isl_take isl_union_map *umap)
{
	return isl_union_map_apply_range(uset, umap);
}

static int product_entry(void **entry, void *user)
{
	struct isl_union_map_bin_data *data = user;
	isl_map *map2 = *entry;

	map2 = isl_map_product(isl_map_copy(data->map), isl_map_copy(map2));

	data->res = isl_union_map_add_map(data->res, map2);

	return 0;
}

__isl_give isl_union_map *isl_union_map_product(__isl_take isl_union_map *umap1,
	__isl_take isl_union_map *umap2)
{
	return bin_op(umap1, umap2, &product_entry);
}

__isl_give isl_union_set *isl_union_set_product(__isl_take isl_union_set *uset1,
	__isl_take isl_union_set *uset2)
{
	return isl_union_map_product(uset1, uset2);
}

__isl_give isl_union_map *isl_union_map_from_range(
	__isl_take isl_union_set *uset)
{
	return uset;
}

__isl_give isl_union_map *isl_union_map_from_domain(
	__isl_take isl_union_set *uset)
{
	return isl_union_map_reverse(isl_union_map_from_range(uset));
}

__isl_give isl_union_map *isl_union_map_from_domain_and_range(
	__isl_take isl_union_set *domain, __isl_take isl_union_set *range)
{
	return isl_union_map_apply_range(isl_union_map_from_domain(domain),
				         isl_union_map_from_range(range));
}

static __isl_give isl_union_map *un_op(__isl_take isl_union_map *umap,
	int (*fn)(void **, void *), int cow)
{
	if (cow)
		umap = isl_union_map_cow(umap);
	if (!umap)
		return NULL;

	if (isl_hash_table_foreach(umap->dim->ctx, &umap->table, fn, NULL) < 0)
		goto error;

	return umap;
error:
	isl_union_map_free(umap);
	return NULL;
}

static int affine_entry(void **entry, void *user)
{
	isl_map **map = (isl_map **)entry;

	*map = isl_map_from_basic_map(isl_map_affine_hull(*map));

	return *map ? 0 : -1;
}

__isl_give isl_union_map *isl_union_map_affine_hull(
	__isl_take isl_union_map *umap)
{
	return un_op(umap, &affine_entry, 1);
}

__isl_give isl_union_set *isl_union_set_affine_hull(
	__isl_take isl_union_set *uset)
{
	return isl_union_map_affine_hull(uset);
}

static int coalesce_entry(void **entry, void *user)
{
	isl_map **map = (isl_map **)entry;

	*map = isl_map_coalesce(*map);

	return *map ? 0 : -1;
}

__isl_give isl_union_map *isl_union_map_coalesce(
	__isl_take isl_union_map *umap)
{
	return un_op(umap, &coalesce_entry, 0);
}

__isl_give isl_union_set *isl_union_set_coalesce(
	__isl_take isl_union_set *uset)
{
	return isl_union_map_coalesce(uset);
}

static int compute_divs_entry(void **entry, void *user)
{
	isl_map **map = (isl_map **)entry;

	*map = isl_map_compute_divs(*map);

	return *map ? 0 : -1;
}

__isl_give isl_union_map *isl_union_map_compute_divs(
	__isl_take isl_union_map *umap)
{
	return un_op(umap, &compute_divs_entry, 0);
}

__isl_give isl_union_set *isl_union_set_compute_divs(
	__isl_take isl_union_set *uset)
{
	return isl_union_map_compute_divs(uset);
}

static int lexmin_entry(void **entry, void *user)
{
	isl_map **map = (isl_map **)entry;

	*map = isl_map_lexmin(*map);

	return *map ? 0 : -1;
}

__isl_give isl_union_map *isl_union_map_lexmin(
	__isl_take isl_union_map *umap)
{
	return un_op(umap, &lexmin_entry, 1);
}

__isl_give isl_union_set *isl_union_set_lexmin(
	__isl_take isl_union_set *uset)
{
	return isl_union_map_lexmin(uset);
}

static int lexmax_entry(void **entry, void *user)
{
	isl_map **map = (isl_map **)entry;

	*map = isl_map_lexmax(*map);

	return *map ? 0 : -1;
}

__isl_give isl_union_map *isl_union_map_lexmax(
	__isl_take isl_union_map *umap)
{
	return un_op(umap, &lexmax_entry, 1);
}

__isl_give isl_union_set *isl_union_set_lexmax(
	__isl_take isl_union_set *uset)
{
	return isl_union_map_lexmax(uset);
}

static __isl_give isl_union_set *cond_un_op(__isl_take isl_union_map *umap,
	int (*fn)(void **, void *))
{
	isl_union_set *res;

	if (!umap)
		return NULL;

	res = isl_union_map_alloc(isl_dim_copy(umap->dim), umap->table.n);
	if (isl_hash_table_foreach(umap->dim->ctx, &umap->table, fn, &res) < 0)
		goto error;

	isl_union_map_free(umap);
	return res;
error:
	isl_union_map_free(umap);
	isl_union_set_free(res);
	return NULL;
}

static int reverse_entry(void **entry, void *user)
{
	isl_map *map = *entry;
	isl_union_map **res = user;

	*res = isl_union_map_add_map(*res, isl_map_reverse(isl_map_copy(map)));

	return 0;
}

__isl_give isl_union_map *isl_union_map_reverse(__isl_take isl_union_map *umap)
{
	return cond_un_op(umap, &reverse_entry);
}

static int domain_entry(void **entry, void *user)
{
	isl_map *map = *entry;
	isl_union_set **res = user;

	*res = isl_union_set_add_set(*res, isl_map_domain(isl_map_copy(map)));

	return 0;
}

__isl_give isl_union_set *isl_union_map_domain(__isl_take isl_union_map *umap)
{
	return cond_un_op(umap, &domain_entry);
}

static int range_entry(void **entry, void *user)
{
	isl_map *map = *entry;
	isl_union_set **res = user;

	*res = isl_union_set_add_set(*res, isl_map_range(isl_map_copy(map)));

	return 0;
}

__isl_give isl_union_set *isl_union_map_range(__isl_take isl_union_map *umap)
{
	return cond_un_op(umap, &range_entry);
}

static int deltas_entry(void **entry, void *user)
{
	isl_map *map = *entry;
	isl_union_set **res = user;

	if (!isl_dim_tuple_match(map->dim, isl_dim_in, map->dim, isl_dim_out))
		return 0;

	*res = isl_union_set_add_set(*res, isl_map_deltas(isl_map_copy(map)));

	return 0;
}

__isl_give isl_union_set *isl_union_map_deltas(__isl_take isl_union_map *umap)
{
	return cond_un_op(umap, &deltas_entry);
}

static int unwrap_entry(void **entry, void *user)
{
	isl_set *set = *entry;
	isl_union_set **res = user;

	if (!isl_set_is_wrapping(set))
		return 0;

	*res = isl_union_map_add_map(*res, isl_set_unwrap(isl_set_copy(set)));

	return 0;
}

__isl_give isl_union_map *isl_union_set_unwrap(__isl_take isl_union_set *uset)
{
	return cond_un_op(uset, &unwrap_entry);
}

static int wrap_entry(void **entry, void *user)
{
	isl_map *map = *entry;
	isl_union_set **res = user;

	*res = isl_union_set_add_set(*res, isl_map_wrap(isl_map_copy(map)));

	return 0;
}

__isl_give isl_union_set *isl_union_map_wrap(__isl_take isl_union_map *umap)
{
	return cond_un_op(umap, &wrap_entry);
}

struct isl_union_map_is_subset_data {
	isl_union_map *umap2;
	int is_subset;
};

static int is_subset_entry(void **entry, void *user)
{
	struct isl_union_map_is_subset_data *data = user;
	uint32_t hash;
	struct isl_hash_table_entry *entry2;
	isl_map *map = *entry;

	hash = isl_dim_get_hash(map->dim);
	entry2 = isl_hash_table_find(data->umap2->dim->ctx, &data->umap2->table,
				     hash, &has_dim, map->dim, 0);
	if (!entry2) {
		data->is_subset = 0;
		return -1;
	}

	data->is_subset = isl_map_is_subset(map, entry2->data);
	if (data->is_subset < 0 || !data->is_subset)
		return -1;

	return 0;
}

int isl_union_map_is_subset(__isl_keep isl_union_map *umap1,
	__isl_keep isl_union_map *umap2)
{
	struct isl_union_map_is_subset_data data = { umap2, 1 };

	if (!umap1 || !umap2)
		return -1;

	if (isl_hash_table_foreach(umap1->dim->ctx, &umap1->table,
				   &is_subset_entry, &data) < 0 &&
	    data.is_subset)
		return -1;

	return data.is_subset;
}

int isl_union_map_is_equal(__isl_keep isl_union_map *umap1,
	__isl_keep isl_union_map *umap2)
{
	int is_subset;

	if (!umap1 || !umap2)
		return -1;
	is_subset = isl_union_map_is_subset(umap1, umap2);
	if (is_subset != 1)
		return is_subset;
	is_subset = isl_union_map_is_subset(umap2, umap1);
	return is_subset;
}

int isl_union_map_is_strict_subset(__isl_keep isl_union_map *umap1,
	__isl_keep isl_union_map *umap2)
{
	int is_subset;

	if (!umap1 || !umap2)
		return -1;
	is_subset = isl_union_map_is_subset(umap1, umap2);
	if (is_subset != 1)
		return is_subset;
	is_subset = isl_union_map_is_subset(umap2, umap1);
	if (is_subset == -1)
		return is_subset;
	return !is_subset;
}

static int sample_entry(void **entry, void *user)
{
	isl_basic_map **sample = (isl_basic_map **)user;
	isl_map *map = *entry;

	*sample = isl_map_sample(isl_map_copy(map));
	if (!*sample)
		return -1;
	if (!isl_basic_map_fast_is_empty(*sample))
		return -1;
	return 0;
}

__isl_give isl_basic_map *isl_union_map_sample(__isl_take isl_union_map *umap)
{
	isl_basic_map *sample = NULL;

	if (!umap)
		return NULL;

	if (isl_hash_table_foreach(umap->dim->ctx, &umap->table,
				   &sample_entry, &sample) < 0 &&
	    !sample)
		goto error;

	isl_union_map_free(umap);

	return sample;
error:
	isl_union_map_free(umap);
	return NULL;
}

__isl_give isl_basic_set *isl_union_set_sample(__isl_take isl_union_set *uset)
{
	return (isl_basic_set *)isl_union_map_sample(uset);
}

static int empty_entry(void **entry, void *user)
{
	int *empty = user;
	isl_map *map = *entry;

	if (isl_map_is_empty(map))
		return 0;

	*empty = 0;

	return -1;
}

__isl_give int isl_union_map_is_empty(__isl_keep isl_union_map *umap)
{
	int empty = 1;

	if (!umap)
		return -1;

	if (isl_hash_table_foreach(umap->dim->ctx, &umap->table,
				   &empty_entry, &empty) < 0 && empty)
		return -1;

	return empty;
}

int isl_union_set_is_empty(__isl_keep isl_union_set *uset)
{
	return isl_union_map_is_empty(uset);
}