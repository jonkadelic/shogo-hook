#pragma once

#include <stdint.h>

#include <lithtech/lithtech.h>

#include "render/object/polygrid.h"

typedef struct object_data {
    uint16_t object_id;
    ObjectType_t object_type;
    uint64_t last_used;
    union {
        polygrid_t as_polygrid;
    }; 
} object_data_t;

typedef struct object_manager {
    tessellator_t* tessellator;

    size_t objects_data_capacity;
    object_data_t** objects_data;
} object_manager_t;

bool object_manager__init(object_manager_t* self, tessellator_t* tessellator);
void object_manager__cleanup(object_manager_t* self);

void object_manager__update(object_manager_t* self);

void object_manager__draw(object_manager_t* self, DObject_t const* object);
