#include "quad_tree.h"
#include "coord.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include <raylib.h>

static bool aabb_contains(quad_aabb_t bounds, coord_t pos) {
    assert(bounds.x_min <= bounds.x_max);
    assert(bounds.y_min <= bounds.y_max);
    return bounds.x_min <= pos.x && pos.x < bounds.x_max &&
           bounds.y_min <= pos.y && pos.y < bounds.y_max;
}

static bool aabb_overlaps(quad_aabb_t a, quad_aabb_t b) {
    bool x_overlap = false;
    bool y_overlap = false;

    // TODO: simplify

    x_overlap = (b.x_min <= a.x_min && a.x_min <= b.x_max) ||
                (b.x_min <= a.x_max && a.x_max <= b.x_max) ||
                (a.x_min <= b.x_min && b.x_min <= a.x_max) ||
                (a.x_min <= b.x_max && b.x_max <= a.x_max);

    y_overlap = (b.y_min <= a.y_min && a.y_min <= b.y_max) ||
                (b.y_min <= a.y_max && a.y_max <= b.y_max) ||
                (a.y_min <= b.y_min && b.y_min <= a.y_max) ||
                (a.y_min <= b.y_max && b.y_max <= a.y_max);

    return x_overlap && y_overlap;
}

static void *arena_alloc(quad_tree_t *tree, size_t size) {
    assert(tree);
    assert(size);

    if (tree->arena_pos + size >= tree->arena_size) {
        printf("arena full\n");
        assert(0);
        return NULL;
    }

    void *ptr = tree->arena_buffer + tree->arena_pos;
    tree->arena_pos += size;
    return ptr;
}

static quad_node_t *create_quad_node(quad_tree_t *tree) {
    assert(tree);

    quad_node_t *node = arena_alloc(tree, sizeof(quad_node_t));
    memset(node, 0, sizeof(quad_node_t));
    return node;
}

static void quad_node_add_item(quad_tree_t *tree, quad_node_t *node, size_t item) {
    assert(tree);
    assert(node);
    assert(item);

    size_t old_count = node->item_count;
    size_t *old_items = node->items;

    size_t new_count = node->item_count + 1;
    size_t *new_items = arena_alloc(tree, sizeof(size_t) * new_count);

    if (old_count && old_items)
        memcpy(new_items, old_items, sizeof(size_t) * old_count);
    new_items[new_count-1] = item;

    node->item_count = new_count;
    node->items = new_items;
}

// -----

quad_tree_t *quad_tree_create() {

    quad_tree_t *tree = malloc(sizeof(quad_tree_t));
    memset(tree, 0, sizeof(quad_tree_t));

    tree->arena_size = 1024 * 1024 * 100;
    tree->arena_pos = 0;
    tree->arena_buffer = malloc(tree->arena_size);
    assert(tree->arena_buffer);

    return tree;
}

void quad_tree_destroy(quad_tree_t *tree) {
    assert(tree);

    free(tree->arena_buffer);
    free(tree);
}

void quad_tree_reset(quad_tree_t *tree) {
    assert(tree);
    //memset(tree, 0, sizeof(quad_tree_t));

    tree->arena_pos = 0;

    const int32_t s = 1 << 16;
    tree->root = create_quad_node(tree);
    tree->root->bounds = (quad_aabb_t) { -s,-s,s,s };

    // Notes:
    // - aabb describes finite point, not cell
    // - pos describes entire cell
}

static uint32_t quad_tree_child_index(int32_t row, int32_t col) {
    assert(row >= 0 && row <= 1);
    assert(col >= 0 && col <= 1);

    return row*2 + col;
}

void quad_tree_insert(quad_tree_t *tree, coord_t pos, size_t item) {
    //printf("insert (%d,%d) -> %lu\n", pos.x, pos.y, item);

    if (!aabb_contains(tree->root->bounds, pos)) { // ~3 ms ???
        printf("outside bounds of root: %d %d\n", pos.x, pos.y);
        assert(0);
        return;
    }

    quad_node_t *current_node = tree->root;

    while (1) {
        quad_aabb_t b = current_node->bounds;
        int32_t w = b.x_max - b.x_min;
        int32_t h = b.y_max - b.y_min;

        if (w == 1 || h == 1) {
            quad_node_add_item(tree, current_node, item);
            break;
        }

        int32_t row = (pos.y - b.y_min) / (h / 2);
        int32_t col = (pos.x - b.x_min) / (w / 2);
        uint32_t idx = quad_tree_child_index(row, col);

        if (!current_node->children[idx]) {
            int32_t x_min = b.x_min + col * w/2;
            int32_t y_min = b.y_min + row * h/2;
            int32_t x_max = x_min + w/2;
            int32_t y_max = y_min + h/2;

            current_node->children[idx] = create_quad_node(tree);
            current_node->children[idx]->bounds = (quad_aabb_t) {
                x_min, y_min, x_max, y_max,
            };
        }

        current_node = current_node->children[idx];
    }
}

void quad_node_query(quad_node_t *node, quad_aabb_t bounds, quad_tree_query_result_t *query_result) {
    assert(node);
    assert(query_result);

    for (size_t i=0; i<node->item_count; i++) {
        if (query_result->count < query_result->capacity) {
            query_result->items[query_result->count++] = node->items[i];
        } else {
            printf("query result full\n");
        }
    }

    for (size_t i=0; i<ARRAY_LENGTH(node->children); i++) {
        quad_node_t *c = node->children[i];

        if (c && aabb_overlaps(c->bounds, bounds)) {
            quad_node_query(c, bounds, query_result);
        }
    }
}

void quad_tree_query(const quad_tree_t *tree, quad_aabb_t bounds, quad_tree_query_result_t *query_result) {
    assert(tree);
    assert(query_result);

    quad_node_query(tree->root, bounds, query_result);
}

static void print_pad(int pad) {
    for (int i=0; i<pad; i++) {
        printf(" ");
    }
}

static void dump_node(const quad_node_t *node, int indent) {
    assert(node);
    quad_aabb_t b = node->bounds;

    print_pad(indent);
    printf("n (%d,%d) (%d,%d) \n", b.x_min, b.y_min, b.x_max, b.y_max);

    for (size_t i=0; i<ARRAY_LENGTH(node->children); i++) {
        if (node->children[i]) {
            dump_node(node->children[i], indent+2);
        } else {
            print_pad(indent+2);
            printf("-\n");
        }
    }
}

void quad_tree_dump(const quad_tree_t *tree) {
    assert(tree);
    printf("*** quad tree ***\n");
    assert(tree->root);
    dump_node(tree->root, 0);
}


static void quad_node_render(const quad_node_t *node) {
    assert(node);

    const quad_aabb_t b = node->bounds;
    const float s = 100.0f;
    const Rectangle r = {
        .x = b.x_min * s + 10,
        .y = b.y_min * s + 10,
        .width = (b.x_max - b.x_min) * s - 20,
        .height = (b.y_max - b.y_min) * s - 20,
    };

    DrawRectangleLinesEx(r, 5.0f, YELLOW);

    int text_y = r.y + 5;
    for (size_t i=0; i<node->item_count; i++) {
        DrawText(TextFormat("[%lu]", node->items[i]), r.x+5, text_y+=20, 20, YELLOW);
    }
   
    for (size_t i=0; i<4; i++) {
        quad_node_t *c = node->children[i];
        if (c) quad_node_render(c);
    }
}

void quad_tree_render(const quad_tree_t *tree) {
    assert(tree);
    quad_node_render(tree->root);
}

