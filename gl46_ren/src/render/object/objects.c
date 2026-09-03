#include "./objects.h"

#include <stddef.h>

#include <SDL3/SDL.h>
#include <stdio.h>

#include "logger.h"
#include "render/tessellator.h"
#include "util/util.h"

#define OBJECT_TIMEOUT  (30 * 1000) // 30 seconds

#define OBJECT_DATA_ALLOC_LEN   (4)

// object_id must be first member for `compare_object_ptrs_by_id` to work
static_assert(offsetof(object_data_t, object_id) == 0);

static int compare_object_ptrs_by_id(void const* a, void const* b);

typedef struct object_vtable {
    void (*draw)(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object);
    void (*cleanup)(object_data_t* self);
} object_vtable_t;

static object_vtable_t const OBJECT_VTABLE[NumObjectTypes] = {
    [ObjectType_PolyGrid] = {
        .draw = object_polygrid__draw,
        .cleanup = object_polygrid__cleanup,
    },
    [ObjectType_WorldModel] = {
        .draw = object_worldmodel__draw,
        .cleanup = object_worldmodel__cleanup,
    },
    [ObjectType_Model] = {
        .draw = object_model__draw,
        .cleanup = object_model__cleanup,
    }
};

bool object_manager__init(object_manager_t* self, tessellator_t* tessellator) {
    OBJECT_ZERO_INIT(self);

    self->tessellator = tessellator;

    return true;
}

void object_manager__cleanup(object_manager_t* self) {
    for (size_t i = 0; i < self->objects_data_capacity; i++) {
        object_data_t* object_data = self->objects_data[i];

        if (object_data != nullptr) {
            SDL_assert(object_data->object_type < NumObjectTypes);

            if (OBJECT_VTABLE[object_data->object_type].cleanup != nullptr) {
                OBJECT_VTABLE[object_data->object_type].cleanup(object_data);
            }

            SDL_free(object_data);
            self->objects_data[i] = nullptr;
        }
    }

    SDL_free(self->objects_data);
    self->objects_data = nullptr;
    self->objects_data_capacity = 0;
}

void object_manager__update(object_manager_t* self) {
    uint64_t ticks = SDL_GetTicks();

    bool removed = false;
    for (size_t i = 0; i < self->objects_data_capacity; i++) {
        object_data_t* object_data = self->objects_data[i];

        if (object_data != nullptr) {
            if (object_data->last_used + OBJECT_TIMEOUT < ticks) {
                // Free object as it has timed out
                if (OBJECT_VTABLE[object_data->object_type].cleanup != nullptr) {
                    OBJECT_VTABLE[object_data->object_type].cleanup(object_data);
                }

                SDL_free(object_data);
                self->objects_data[i] = nullptr;
                removed = true;
            }
        }
    }

    if (removed) {
        SDL_qsort(
            self->objects_data,
            self->objects_data_capacity,
            sizeof(object_data_t*),
            compare_object_ptrs_by_id
        );
    }
}

void object_manager__draw(object_manager_t* self, SceneDesc_t const* scene_desc, DObject_t const* object) {
    if (object == nullptr || object->m_ObjectType >= NumObjectTypes) {
        return;
    }

    if (object->m_pClass != nullptr) return;
    if ((object->m_Flags & 0x01) == 0) return; // !visible

    uint16_t const* object_id = &object->m_ObjectID;

    if (OBJECT_VTABLE[object->m_ObjectType].draw == nullptr) {
        return;
    }

    object_data_t** object_data_ptr = nullptr;
    object_data_t* object_data = nullptr;

    if (object->m_ObjectID == 0xFFFF) {
        // client-only objects all share this sentinel ID, so id-based lookup can't disambiguate them
        for (size_t i = 0; i < self->objects_data_capacity; i++) {
            if (self->objects_data[i] != nullptr && self->objects_data[i]->owner == object) {
                object_data_ptr = &self->objects_data[i];
                break;
            }
        }
    } else {
        object_data_ptr = SDL_bsearch(
            &object_id,
            self->objects_data,
            self->objects_data_capacity,
            sizeof(object_data_t*),
            compare_object_ptrs_by_id
        );
    }

    // Add new object if no data
    if (object_data_ptr == nullptr) {
        // Try to find existing empty slot
        for (size_t i = 0; i < self->objects_data_capacity; i++) {
            if (self->objects_data[i] == nullptr) {
                object_data_ptr = &self->objects_data[i];
                break;
            }
        }

        // If no slot, grow array
        if (object_data_ptr == nullptr) {
            size_t new_objects_data_capacity = self->objects_data_capacity + OBJECT_DATA_ALLOC_LEN;
            object_data_t** new_objects_data = SDL_realloc(self->objects_data, new_objects_data_capacity * sizeof(object_data_t*));
            if (new_objects_data == nullptr) {
                LOG_ERROR("Failed to allocate %zu object data pointers", new_objects_data_capacity);
                return;
            }

            for (size_t i = self->objects_data_capacity; i < new_objects_data_capacity; i++) {
                new_objects_data[i] = nullptr;
            }

            self->objects_data = new_objects_data;
            object_data_ptr = &self->objects_data[self->objects_data_capacity];
            self->objects_data_capacity = new_objects_data_capacity;
        }

        // Allocate new object data
        *object_data_ptr = SDL_calloc(1, sizeof(object_data_t));
        if (*object_data_ptr == nullptr) {
            LOG_ERROR("Failed to allocate new object data");
            return;
        }

        // Set up object data
        object_data = *object_data_ptr;
        object_data->object_id = object->m_ObjectID;
        object_data->object_type = object->m_ObjectType;
        object_data->owner = object;

        // Re-sort object data by ID
        SDL_qsort(
            self->objects_data,
            self->objects_data_capacity,
            sizeof(object_data_t*),
            compare_object_ptrs_by_id
        );
    } else {
        object_data = *object_data_ptr;
        object_data->owner = object;

        if (object_data->object_type != object->m_ObjectType) {
            if (OBJECT_VTABLE[object_data->object_type].cleanup != nullptr) {
                OBJECT_VTABLE[object_data->object_type].cleanup(object_data);
            }

            SDL_memset(&object_data->as_polygrid, 0, sizeof(object_data->as_polygrid));
            SDL_memset(&object_data->as_worldmodel, 0, sizeof(object_data->as_worldmodel));
            SDL_memset(&object_data->as_model, 0, sizeof(object_data->as_model));
            object_data->object_type = object->m_ObjectType;
        }
    }

    SDL_assert(object_data != nullptr);

    object_data->last_used = SDL_GetTicks();

    SDL_assert(object->m_ObjectType < NumObjectTypes);

    if (OBJECT_VTABLE[object->m_ObjectType].draw != nullptr) {
        OBJECT_VTABLE[object->m_ObjectType].draw(object_data, scene_desc, object);
    }
}

static int compare_object_ptrs_by_id(void const* a, void const* b) {
    uint16_t const* id_a = *(uint16_t const**) a;
    uint16_t const* id_b = *(uint16_t const**) b;

    if (id_a == nullptr && id_b == nullptr) {
        return 0;
    }
    if (id_a == nullptr) {
        return -1;
    }
    if (id_b == nullptr) {
        return 1;
    }

    return (*id_a > *id_b) - (*id_a < *id_b);
}
