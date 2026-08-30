#include "./textures.h"

#include "util/util.h"

#define TEXTURE_ALLOC_SIZE  (4)
#define UPLOAD_ALLOC_SIZE   (4)

#define GET_IDX(id)         ((id & 0x000F'FFFF) - 1)
#define GET_GEN(id)         ((id & 0xFFF0'0000) >> 20)
#define MAKE_ID(idx, gen)   (((gen & 0x0FFF) << 20) | ((idx & 0x000F'FFFF) + 1))

static texture_t const* get_texture(texture_manager_t const* self, texture_id_t id);
static texture_t* get_texture_mut(texture_manager_t* self, texture_id_t id);

bool texture_manager__init(texture_manager_t* self, SDL_GPUDevice* device) {
    ZERO_INIT_STRUCT(self);

    self->device = device;

    if (!sampler_manager__init(&self->samplers, device)) {
        goto err;
    }

    return true;

err:
    texture_manager__cleanup(self);
    return false;
}

void texture_manager__cleanup(texture_manager_t* self) {
    // Free upload list
    for (size_t i = 0; i < self->uploads_len; i++) {
        SDL_free(self->uploads[i].data);
    }
    SDL_free(self->uploads);
    self->uploads = nullptr;

    // Free textures list
    for (size_t i = 0; i < self->textures_capacity; i++) {
        SDL_ReleaseGPUTexture(self->device, self->textures[i].texture);
    }
    SDL_free(self->textures);
    self->textures = nullptr;

    // Free transfer buffer
    SDL_ReleaseGPUTransferBuffer(self->device, self->tx_buffer);
    self->tx_buffer = nullptr;

    sampler_manager__cleanup(&self->samplers);

    self->device = nullptr;
}

bool texture_manager__needs_flush(texture_manager_t* self) {
    return self->uploads_len > 0;
}

void texture_manager__flush_to_gpu(texture_manager_t* self, SDL_GPUCommandBuffer* cmd_buffer, SDL_GPUCopyPass* opt_copy_pass) {
    SDL_GPUCopyPass* copy_pass = opt_copy_pass;

    if (self->uploads_len == 0) {
        return;
    }

    // Ensure textures are sufficient
    for (size_t i = 0; i < self->uploads_len; i++) {
        texture_upload_t const* upload = &self->uploads[i];
        texture_t* texture = get_texture_mut(self, upload->texture);
        if (texture == nullptr) {
            continue;
        }

        // Recreate texture if size doesn't match
        if (texture->texture == nullptr || texture->width != upload->width || texture->height != upload->height) {
            SDL_ReleaseGPUTexture(self->device, texture->texture);
            
            SDL_GPUTextureCreateInfo texture_info = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = upload->width, .height = upload->height,
                .layer_count_or_depth = 1,
                .num_levels = 1,
                .sample_count = SDL_GPU_SAMPLECOUNT_1,
            };
            texture->texture = SDL_CreateGPUTexture(self->device, &texture_info);
            if (texture->texture == nullptr) {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create %ux%u texture: %s", upload->width, upload->height, SDL_GetError());
                return;
            }

            texture->width = upload->width;
            texture->height = upload->height;
        }
    }

    // Get required transfer buffer length
    size_t new_tx_buffer_size = 0;
    for (size_t i = 0; i < self->uploads_len; i++) {
        texture_upload_t const* upload = &self->uploads[i];

        new_tx_buffer_size += upload->width * upload->height * 4;
    }

    // Ensure transfer buffer alignment
    if (new_tx_buffer_size % 0x1000 != 0) {
        new_tx_buffer_size = (new_tx_buffer_size - (new_tx_buffer_size % 0x1000)) + 0x1000;
    }

    // Re-create transfer buffer if needed
    if (new_tx_buffer_size > self->tx_buffer_size) {
        auto props = SDL_CreateProperties();
        SDL_SetStringProperty(props, SDL_PROP_GPU_TRANSFERBUFFER_CREATE_NAME_STRING, "TexUploadTxBuffer");
        SDL_GPUTransferBufferCreateInfo info = {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = new_tx_buffer_size,
            .props = props,
        };
        SDL_GPUTransferBuffer* new_tx_buffer = SDL_CreateGPUTransferBuffer(self->device, &info);
        SDL_DestroyProperties(props);
        if (new_tx_buffer == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create transfer buffer of %zu bytes: %s", new_tx_buffer_size, SDL_GetError());
            goto end;
        }

        SDL_ReleaseGPUTransferBuffer(self->device, self->tx_buffer);

        self->tx_buffer_size = new_tx_buffer_size;
        self->tx_buffer = new_tx_buffer;
    }

    // Begin copy pass
    if (copy_pass == nullptr) {
        copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);
        if (copy_pass == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to begin copy pass: %s", SDL_GetError());
            goto end;
        }
    }

    // Map transfer buffer
    uint8_t* tx_buffer_data = SDL_MapGPUTransferBuffer(self->device, self->tx_buffer, false);
    if (tx_buffer_data == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to map transfer buffer: %s", SDL_GetError());
        goto end;
    }

    // Iterate over transfers
    size_t tx_buffer_offset = 0;
    for (size_t i = 0; i < self->uploads_len; i++) {
        texture_upload_t* upload = &self->uploads[i];
        texture_t const* texture = get_texture(self, upload->texture);
        if (texture == nullptr) {
            continue;
        }

        SDL_assert(texture->width == upload->width && texture->height == upload->height);

        // Copy texture data to transfer buffer
        size_t data_size = upload->width * upload->height * 4;
        SDL_assert(tx_buffer_offset + data_size <= self->tx_buffer_size);
        SDL_memcpy(tx_buffer_data + tx_buffer_offset, upload->data, data_size);

        // Buffer upload command for texture data
        SDL_GPUTextureTransferInfo copy_src = {
            .transfer_buffer = self->tx_buffer,
            .offset = tx_buffer_offset,
            .pixels_per_row = upload->width,
            .rows_per_layer = upload->height,
        };
        SDL_GPUTextureRegion copy_dst = {
            .texture = texture->texture,
            .w = upload->width,
            .h = upload->height,
            .d = 1,
        };
        SDL_UploadToGPUTexture(copy_pass, &copy_src, &copy_dst, false);

        upload->texture = TEXTURE_ID_INVALID;
        SDL_free(upload->data);
        upload->data = nullptr;

        tx_buffer_offset += data_size;
    }

    // Unmap transfer buffer
    SDL_UnmapGPUTransferBuffer(self->device, self->tx_buffer);

end:
    if (copy_pass != opt_copy_pass) {
        SDL_EndGPUCopyPass(copy_pass);
    }
}

bool texture_manager__is_valid(texture_manager_t const* self, texture_id_t id) {
    texture_t const* texture = get_texture(self, id);
    if (texture == nullptr || texture->texture == nullptr) {
        return false;
    }

    return true;
}

bool texture_manager__get_texture_info(
    texture_manager_t const* self,
    texture_id_t id,
    uint32_t* out_width,
    uint32_t* out_height
) {
    texture_t const* texture = get_texture(self, id);

    *out_width = texture->width;
    *out_height = texture->height;

    return true;
}

texture_id_t texture_manager__create_texture(texture_manager_t* self) {
    texture_t* texture = nullptr;

    // Check if existing texture slot can be reused
    uint32_t texture_idx = 0;
    for (uint32_t i = 0; i < self->textures_capacity; i++) {
        if (!self->textures[i].assigned) {
            texture = &self->textures[i];
            texture_idx = i;
            break;
        }
    }

    // If no texture, resize texture list and initialize
    if (texture == nullptr) {
        size_t new_textures_capacity = self->textures_capacity + TEXTURE_ALLOC_SIZE;
        texture_t* new_textures = SDL_realloc(self->textures, sizeof(texture_t) * new_textures_capacity);
        if (new_textures == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu textures", new_textures_capacity);
            return TEXTURE_ID_INVALID;
        }

        for (size_t i = self->textures_capacity; i < new_textures_capacity; i++) {
            new_textures[i].texture = nullptr;
        }

        texture = &new_textures[self->textures_capacity];
        texture_idx = self->textures_capacity;

        self->textures_capacity = new_textures_capacity;
        self->textures = new_textures;
    }

    // Initialize texture
    texture->assigned = true;
    texture->texture = nullptr;
    texture->width = 0;
    texture->height = 0;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Allocated texture %u", texture_idx + 1);

    return (texture_id_t) (texture_idx + 1);
}

void texture_manager__delete_texture(texture_manager_t* self, texture_id_t id) {
    texture_t* texture = get_texture_mut(self, id);
    if (texture == nullptr) {
        return;
    }

    SDL_ReleaseGPUTexture(self->device, texture->texture);
    texture->texture = nullptr;
    texture->assigned = false;
    texture->gen++;
}

bool texture_manager__upload_texture_data(
    texture_manager_t* self,
    texture_id_t id,
    void const* data,
    size_t width, size_t height
) {
    texture_t const* texture = get_texture(self, id);
    if (texture == nullptr) {
        return false;
    }
    
    // Ensure upload buffer is large enough
    size_t upload_index = self->uploads_len;
    if (self->uploads_len + 1 > self->uploads_capacity) {
        size_t new_uploads_capacity = self->uploads_capacity + UPLOAD_ALLOC_SIZE;
        texture_upload_t* new_uploads = SDL_realloc(self->uploads, sizeof(texture_upload_t) * new_uploads_capacity);
        if (new_uploads == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu texture uploads", new_uploads_capacity);
            return false;
        }

        self->uploads_capacity = new_uploads_capacity;
        self->uploads = new_uploads;
    }

    texture_upload_t* upload = &self->uploads[upload_index];

    // Init upload object
    upload->texture = id;
    upload->width = width;
    upload->height = height;

    // Copy data buffer
    upload->data = SDL_malloc(width * height * 4);
    if (upload->data == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu-byte buffer for texture data", width * height * 4);
        return false;
    }
    SDL_memcpy(upload->data, data, width * height * 4);

    self->uploads_len++;

    return true;
}

bool texture_manager__bind_texture(
    texture_manager_t* self,
    uint32_t slot,
    texture_id_t id,
    sampler_config_t sampler_config,
    SDL_GPURenderPass* render_pass
) {
    texture_t const* texture = get_texture(self, id);
    if (texture == nullptr) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Tried to bind invalid texture");
        return false;
    }

    SDL_GPUSampler* sampler = sampler_manager__get_sampler(&self->samplers, sampler_config);
    if (sampler == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to acquire sampler for texture");
        return false;
    }

    SDL_BindGPUFragmentSamplers(
        render_pass,
        slot,
        &(SDL_GPUTextureSamplerBinding) {
            .texture = texture->texture,
            .sampler = sampler,
        },
        1
    );

    return true;
}

static texture_t const* get_texture(texture_manager_t const* self, texture_id_t id) {
    if (id == TEXTURE_ID_INVALID) {
        return nullptr;
    }

    size_t idx = GET_IDX(id);
    size_t gen = GET_GEN(id);

    if (idx >= self->textures_capacity) {
        return nullptr;
    }

    texture_t* texture = &self->textures[idx];
    if (gen != texture->gen || !texture->assigned) {
        return nullptr;
    }

    return texture;
}

static texture_t* get_texture_mut(texture_manager_t* self, texture_id_t id) {
    if (id == TEXTURE_ID_INVALID) {
        return nullptr;
    }

    size_t idx = GET_IDX(id);
    size_t gen = GET_GEN(id);

    if (idx >= self->textures_capacity) {
        return nullptr;
    }

    texture_t* texture = &self->textures[idx];
    if (gen != texture->gen || !texture->assigned) {
        return nullptr;
    }

    return texture;
}