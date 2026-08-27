#include "./object.h"
#include <stdio.h>

typedef void (*DrawFn)(DObject_t* object);

static void draw_normal(DObject_t* object);
static void draw_model(DObject_t* object);
static void draw_worldmodel(DObject_t* object);
static void draw_sprite(DObject_t* object);
static void draw_light(DObject_t* object);
static void draw_camera(DObject_t* object);
static void draw_particlesystem(DObject_t* object);
static void draw_polygrid(DObject_t* object);
static void draw_linesystem(DObject_t* object);
static void draw_container(DObject_t* object);

static DrawFn const DRAW_FUNCTIONS[NumObjectTypes] = {
    [ObjectType_Normal] = draw_normal,
    [ObjectType_Model] = draw_model,
    [ObjectType_WorldModel] = draw_worldmodel,
    [ObjectType_Sprite] = draw_sprite,
    [ObjectType_Light] = draw_light,
    [ObjectType_Camera] = draw_camera,
    [ObjectType_ParticleSystem] = draw_particlesystem,
    [ObjectType_PolyGrid] = draw_polygrid,
    [ObjectType_LineSystem] = draw_linesystem,
    [ObjectType_Container] = draw_container,
};

void object__draw(DObject_t* object) {
    if (object->m_ObjectType >= NumObjectTypes) {
        printf("Invalid object draw type %hhu\n", object->m_ObjectType);
        return;
    }

    DRAW_FUNCTIONS[object->m_ObjectType](object);
}

static void draw_normal(DObject_t* object) {
    printf("Drawing normal object %hu\n", object->m_ObjectID);
}

static void draw_model(DObject_t* object) {
    printf("Drawing model object %hu\n", object->m_ObjectID);

}

static void draw_worldmodel(DObject_t* object) {
    printf("Drawing worldmodel object %hu\n", object->m_ObjectID);

}

static void draw_sprite(DObject_t* object) {
    printf("Drawing sprite object %hu\n", object->m_ObjectID);

}

static void draw_light(DObject_t* object) {
    printf("Drawing light object %hu\n", object->m_ObjectID);

}

static void draw_camera(DObject_t* object) {
    printf("Drawing camera object %hu\n", object->m_ObjectID);

}

static void draw_particlesystem(DObject_t* object) {
    printf("Drawing particlesystem object %hu\n", object->m_ObjectID);

}

static void draw_polygrid(DObject_t* object) {
    printf("Drawing polygrid object %hu\n", object->m_ObjectID);

    PolyGrid_t* polygrid = (PolyGrid_t*) object;
    printf("Polygrid is %ux%u\n", polygrid->m_Width, polygrid->m_Height);
}

static void draw_linesystem(DObject_t* object) {
    printf("Drawing linesystem object %hu\n", object->m_ObjectID);

}

static void draw_container(DObject_t* object) {
    printf("Drawing container object %hu\n", object->m_ObjectID);

}

