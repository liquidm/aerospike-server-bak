/*
 * hist.h
 *
 * Copyright (C) 2009-2014 Aerospike, Inc.
 *
 * Portions may be licensed to Aerospike, Inc. under one or more contributor
 * license agreements.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include "citrusleaf/cf_atomic.h"
#include "dynbuf.h"


//==========================================================
// Histogram with logarithmic buckets, used for all our
// timing metrics.
//

#define N_BUCKETS 64
#define HISTOGRAM_NAME_SIZE 128

// DO NOT access this member data directly - use the API!
// (Except for cf_hist_track, for which histogram is a base class.)
typedef struct histogram_s {
	char name[HISTOGRAM_NAME_SIZE];
	cf_atomic64 counts[N_BUCKETS];
} histogram;

extern histogram *histogram_create(const char *name);
extern void histogram_clear(histogram *h);
extern void histogram_dump(histogram *h );

extern void histogram_insert_raw(histogram *h, uint64_t value);
extern void histogram_insert_ms_since(histogram *h, uint64_t start_ns);
extern void histogram_insert_us_since(histogram *h, uint64_t start_ns);


//==========================================================
// Histogram with linear buckets, used by the eviction
// algorithm, and for various statistics, e.g. record sizes.
//

#define MAX_LINEAR_BUCKETS 100
#define INFO_SNAPSHOT_SIZE 2048

// DO NOT access this member data directly - use the API!
typedef struct linear_histogram_s {
	char name[HISTOGRAM_NAME_SIZE];
	int num_buckets;
	uint64_t start;
	uint64_t bucket_width;
	cf_atomic64 counts[MAX_LINEAR_BUCKETS];
	pthread_mutex_t info_lock;
	char info_snapshot[INFO_SNAPSHOT_SIZE];
} linear_histogram;

extern linear_histogram *linear_histogram_create(char *name, uint64_t start,
		uint64_t max_offset, int num_buckets);
extern void linear_histogram_destroy(linear_histogram *h);
extern void linear_histogram_clear(linear_histogram *h, uint64_t start,
		uint64_t max_offset); // Note: not thread-safe!
extern void linear_histogram_dump(linear_histogram *h);

extern uint64_t linear_histogram_get_total(linear_histogram *h);
extern void linear_histogram_insert_data_point(linear_histogram *h,
		uint64_t point);
extern bool linear_histogram_get_thresholds_for_fraction(linear_histogram *h,
		uint32_t tenths_pct, uint64_t *p_low, uint64_t *p_high,
		uint32_t *p_mid_tenths_pct); // Note: not thread-safe!
extern bool linear_histogram_get_thresholds_for_subtotal(linear_histogram *h,
		uint64_t subtotal, uint64_t *p_low, uint64_t *p_high,
		uint32_t *p_mid_tenths_pct); // Note: not thread-safe!

extern void linear_histogram_save_info(linear_histogram *h);
extern void linear_histogram_get_info(linear_histogram *h, cf_dyn_buf *db);
