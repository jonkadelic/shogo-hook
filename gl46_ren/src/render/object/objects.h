#pragma once

#include <stdint.h>

#include <lithtech/lithtech.h>

#include "render/object/object_polygrid.h"
#include "render/object/object_worldmodel.h"
#include "render/object/object_model.h"

typedef struct object_data {
    uint16_t object_id;
    DObject_t const* owner;
    ObjectType_t object_type;
    uint64_t last_used;
    union {
        object_polygrid_t as_polygrid;
        object_worldmodel_t as_worldmodel;
        object_model_t as_model;
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

void object_manager__draw(object_manager_t* self, SceneDesc_t const* scene_desc, DObject_t const* object);
