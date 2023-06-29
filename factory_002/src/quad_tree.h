#pragma once

#include <stddef.h>
#include <stdint.h>

#include "coord.h"

typedef struct {
    int32_t x_min;
    int32_t y_min;
    int32_t x_max;
    int32_t y_max;
} quad_aabb_t;

typedef struct quad_node {
    quad_aabb_t bounds;
    struct quad_node *children[4];

    size_t item_count;
    size_t *items;

} quad_node_t;

typedef struct {
    size_t arena_size;
    size_t arena_pos;
    void *arena_buffer;

    quad_node_t *root;
} quad_tree_t;

typedef struct {
    size_t capacity;
    size_t count;
    size_t *items;
} quad_tree_query_result_t;

quad_tree_t *quad_tree_create();
void quad_tree_destroy(quad_tree_t *tree);

void quad_tree_reset(quad_tree_t *tree);
void quad_tree_insert(quad_tree_t *tree, coord_t pos, size_t item);
void quad_tree_query(const quad_tree_t *tree, quad_aabb_t bounds, quad_tree_query_result_t *result);

void quad_tree_dump(const quad_tree_t *tree);
void quad_tree_render(const quad_tree_t *tree);

