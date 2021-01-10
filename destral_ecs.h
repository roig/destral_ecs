#ifndef DESTRAL_ECS_H
#define DESTRAL_ECS_H
/*
    destral_ecs.h -- simple ECS system

    Do this:
        #define DESTRAL_ECS_IMPL
    before you include this file in *one* C file to create the implementation.

    FIX (dani) TODO proper instructions

*/

#include <stdint.h>
#include <stdbool.h>

/*  de_cp_type:
    Component Type identifier information.
*/
typedef struct de_cp_type {
    size_t cp_id; // component unique id
    size_t cp_sizeof; // component sizeof
    const char* name; // component name
} de_cp_type;

#define DE_MAKE_CP_TYPE(TypeId, TypeName) { .cp_id = TypeId, .cp_sizeof = sizeof(TypeName) , .name = #TypeName }

/*  ecs_entity:

    Opaque 32 bits entity identifier.

    A 32 bits entity identifier guarantees:

    20 bits for the entity number(suitable for almost all the games).
    12 bit for the version(resets in[0 - 4095]).

    Use the functions ecs_entity_version and ecs_entity_id to retrieve
    each part of a ecs_entity.
*/

/* Typedef for the entity type */
typedef uint32_t ecs_entity;
typedef struct uint32_t { uint32_t ver; } uint32_t;
typedef struct uint32_t { uint32_t id; } uint32_t;

/* Masks for retrieving the id and the version of an entity */
#define ECS_ENTITY_ID_MASK       ((uint32_t)0xFFFFF) /* Mask to use to get the entity number out of an identifier.*/  
#define ECS_ENTITY_VERSION_MASK  ((uint32_t)0xFFF)   /* Mask to use to get the version out of an identifier. */
#define ECS_ENTITY_SHIFT         ((size_t)20)        /* Extent of the entity number within an identifier. */   

/* The de_null is a ecs_entity that represents a null entity. */
extern const ecs_entity de_null;

/* ecs_entity utilities */

/* Returns the version part of the entity */
uint32_t ecs_entity_version(ecs_entity e);

/* Returns the id part of the entity */
uint32_t ecs_entity_id(ecs_entity e);

/* Makes a ecs_entity from entity_id and entity_version */
ecs_entity ecs_entity_assemble(uint32_t id, uint32_t version);


/*  de_registry

    Is the global context that holds each storage for each component types
    and the entities.
*/
typedef struct de_registry de_registry;

/*  Allocates and initializes a de_registry context */
de_registry* de_registry_create();

/*  Deinitializes and frees a de_registry context */
void de_registry_destroy(de_registry* r);

/*
    Creates a new entity and returns it
    The identifier can be:
     - New identifier in case no entities have been previously destroyed
     - Recycled identifier with an update version.
*/
ecs_entity s_ecs_create_enitty(de_registry* r);

/*
    Destroys an entity.

    When an entity is destroyed, its version is updated and the identifier
    can be recycled when needed.

    Warning:
    Undefined behavior if the entity is not valid.
*/
void de_destroy(de_registry* r, ecs_entity e);

/*
    Checks if an entity identifier is a valid one.
    The entity e can be a valid or an invalid one.
    Teturns true if the identifier is valid, false otherwise
*/
bool de_valid(de_registry* r, ecs_entity e);

/*
    Removes all the components from an entity and makes it orphaned (no components in it)
    the entity remains alive and valid without components.

    Warning: attempting to use invalid entity results in undefined behavior
*/
void de_remove_all(de_registry* r, ecs_entity e);

/*
    Assigns a given component type to an entity and returns it.

    A new memory instance of component type cp_type is allocated for the
    entity e and returned.

    Note: the memory returned is only allocated not initialized.

    Warning: use an invalid entity or assigning a component to a valid
    entity that currently has this component instance id results in
    undefined behavior.
*/
void* de_emplace(de_registry* r, ecs_entity e, de_cp_type cp_type);

/*
    Removes the given component from the entity.

    Warning: attempting to use invalid entity results in undefined behavior
*/
void de_remove(de_registry* r, ecs_entity e, de_cp_type cp_type);

/*
    Checks if the entity has the given component

    Warning: using an invalid entity results in undefined behavior.
*/
bool de_has(de_registry* r, ecs_entity e, de_cp_type cp_type);

/*
    Returns the pointer to the given component type data for the entity

    Warning: Using an invalid entity or get a component from an entity
    that doesn't own it results in undefined behavior.

    Note: This is the fastest way of retrieveing component data but
    has no checks. This means that you are 100% sure that the entity e
    has the component emplaced. Use de_try_get to check if you want checks
*/
void* de_get(de_registry* r, ecs_entity e, de_cp_type cp_type);

/*
    Returns the pointer to the given component type data for the entity
    or nullptr if the entity doesn't have this component.

    Warning: Using an invalid entity results in undefined behavior.
    Note: This is safer but slower than de_get.
*/
void* de_try_get(de_registry* r, ecs_entity e, de_cp_type cp_type);

/*
    Iterates all the entities that are still in use and calls
    the function pointer for each one.

    This is a fairly slow operation and should not be used frequently.
    However it's useful for iterating all the entities still in use,
    regarding their components.
*/
void de_each(de_registry* r, void (*fun)(de_registry*, ecs_entity, void*), void* udata);

/*
    Returns true if an entity has no components assigned, false otherwise.

    Warning: Using an invalid entity results in undefined behavior.
*/
bool de_orphan(de_registry* r, ecs_entity e);

/*
    Iterates all the entities that are orphans (no components in it) and calls
    the function pointer for each one.

    This is a fairly slow operation and should not be used frequently.
    However it's useful for iterating all the entities still in use,
    regarding their components.
*/
void de_orphans_each(de_registry* r, void (*fun)(de_registry*, ecs_entity, void*), void* udata);


void de_iterate(de_registry* r, de_cp_type cp_id, void (*fn)(de_registry, size_t, ecs_entity, void*));

/*
    de_view_single

    Use this view to iterate entities that have the component type specified.
*/
typedef struct de_view_single {
    void* pool; // de_storage opaque pointer
    size_t entity_index;
    ecs_entity entity;
} de_view_single;

de_view_single de_create_view_single(de_registry* r, de_cp_type cp_type);
bool de_view_single_valid(de_view_single* v);
ecs_entity de_view_single_entity(de_view_single* v);
void* de_view_single_get(de_view_single* v);
void de_view_single_next(de_view_single* v);



/*
    de_view

    Use this view to iterate entities that have multiple component types specified.

    Note: You don't need to destroy the view because it doesn't allocate and it
    is not recommended that you save a view. Just use de_create_view each time.
    It's a "cheap" operation.

    Example usage with two components:

    for (de_view v = de_create_view(r, 2, (de_cp_type[2]) {transform_type, velocity_type }); de_view_valid(&v); de_view_next(&v)) {
        ecs_entity e = de_view_entity(&v);
        transform* tr = de_view_get(&v, transform_type);
        velocity* tc = de_view_get(&v, velocity_type);
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", ecs_entity_id(e).id, tr->x, tr->y, tr->z);
        printf("velocity  entity: %d => w=%f\n", ecs_entity_id(e).id, tc->v);
    }

*/
typedef struct de_view {
    #define DE_MAX_VIEW_COMPONENTS (16)
    /* value is the component id, index is where is located in the all_pools array */
    size_t to_pool_index[DE_MAX_VIEW_COMPONENTS];
    void* all_pools[DE_MAX_VIEW_COMPONENTS]; // de_storage opaque pointers
    size_t pool_count;
    void* pool; // de_storage opaque pointer
    size_t entity_index;
    ecs_entity entity;
} de_view;


de_view de_create_view(de_registry* r, size_t cp_count, de_cp_type* cp_types);
bool de_view_valid(de_view* v);
ecs_entity de_view_entity(de_view* v);
void* de_view_get(de_view* v, de_cp_type cp_type);
void* de_view_get_by_index(de_view* v, size_t pool_index);
void de_view_next(de_view* v);

/**************** Implementation ****************/
#define DESTRAL_ECS_IMPL
#ifdef DESTRAL_ECS_IMPL

/*
 TODO LIST:
    - Context variables (those are like global variables) but are inside the registry, malloc/freed inside the registry.
    - Try to make the API simpler  for single/multi views. 
    (de_it_start, de_it_next, de_it_valid
    (de_multi_start, de_multi_next, de_multi_valid,)
    - Callbacks on component insertions/deletions/updates
*/

#include <assert.h>
#include <string.h>
#include <stdlib.h>

const ecs_entity de_null = (ecs_entity)ECS_ENTITY_ID_MASK;

/* Returns the version part of the entity */
uint32_t ecs_entity_version(ecs_entity e) { return (uint32_t) { .ver = e >> ECS_ENTITY_SHIFT }; }
/* Returns the id part of the entity */
uint32_t ecs_entity_id(ecs_entity e) { return (uint32_t) { .id = e & ECS_ENTITY_ID_MASK }; }
/* Makes a ecs_entity from entity_id and entity_version */
ecs_entity ecs_entity_assemble(uint32_t id, uint32_t version) { return id.id | (version.ver << ECS_ENTITY_SHIFT); }

/*
    de_storage
    Here is the storage for each component type.

    ENTITIES:

    The main idea comes from ENTT C++ library:
    https://github.com/skypjack/entt
    https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#views
    (Credits to skypjack) for the awesome library.

    We have an sparse array that maps entity identifiers to the dense array indices that contains the full entity.

    sparse array:
    sparse => contains the index in the dense array of an entity identifier (without version)
    that means that the index of this array is the entity identifier (without version) and
    the content is the index of the dense array.

    dense array:
    dense => contains all the entities (ecs_entity).
    the index is just that, has no meaning here, it's referenced in the sparse.
    the content is the ecs_entity.

    this allows fast iteration on each entity using the dense array or
    lookup for an entity position in the dense using the sparse array.

    ---------- Example:
    Adding:
     ecs_entity = 3 => (e3)
     ecs_entity = 1 => (e1)

    In order to check the entities first in the sparse, we have to retrieve the uint32_t part of the ecs_entity.
    The uint32_t part will be used to index the sparse array.
    The full ecs_entity will be the value in the dense array.

                           0    1     2    3
    sparse idx:         eid0 eid1  eid2  eid3    this is the array index based on uint32_t (NO VERSION)
    sparse content:   [ null,   1, null,   0 ]   this is the array content. (index in the dense array)

    dense         idx:    0    1
    dense     content: [ e3,  e2]

    COMPONENT DATA:
    How the component data is stored?
    This is the easy part, the component data array is aligned with the dense array.
    This means that the entity from the index 0 of the dense array has the component data
    of the index 0 of the cp_data array.

    The packed component elements data is aligned always with the dense array from the sparse set.

    adding/removing an entity to the storage will:
        - add/remove from the sparse
        - use the sparse_set dense array position to move the components data aligned.

    Example:

                  idx:    0    1    2
    dense     content: [ e3,  e2,  e1]
    cp_data   content: [e3c, e2c, e1c] contains component data for the entity in the corresponding index

    If now we remove from the storage the entity e2:

                  idx:    0    1    2
    dense     content: [ e3,  e1]
    cp_data   content: [e3c, e1c] contains component data for the entity in the corresponding index

    note that the alignment to the index in the dense and in the cp_data is always preserved.

    This allows fast iteration for each component and having the entities accessible aswell.
    for (i = 0; i < dense_size; i++) {  // mental example, wrong syntax
        ecs_entity e = dense[i];
        void*   ecp = cp_data[i];
    }


*/
typedef struct de_storage {
    size_t cp_id; /* component id for this storage */
    size_t cp_sizeof; /* sizeof for each cp_data element */

    /*  packed component elements array. aligned with sparse->dense*/
    void* cp_data; 
    size_t cp_data_size; /* number of elements in the cp_data array */
    
    /*  sparse entity identifiers indices array.
    - index is the uint32_t. (without version)
    - value is the index of the dense array
    */
    ecs_entity* sparse;
    size_t sparse_size; /* number of elements in the sparse array */

    /*  Dense entities array.
        - index is linked with the sparse value.
        - value is the full ecs_entity
    */
    ecs_entity* dense;
    size_t dense_size; /* number of elements in the dense array */

} de_storage;




static de_storage* de_storage_new(size_t cp_size, size_t cp_id) {
    de_storage* s = malloc(sizeof(de_storage));
    if (s) {
        *s = (de_storage){ 0 };
        s->cp_sizeof = cp_size;
        s->cp_id = cp_id;
    }
    return s;
}

static void de_storage_delete(de_storage* s) {
    if (s) {
        free(s->sparse);
        free(s->dense);
        free(s->cp_data);
    }
    free(s);
}

static bool de_storage_contains(de_storage* s, ecs_entity e) {
    assert(s);
    assert(e != de_null);
    const uint32_t eid = ecs_entity_id(e);
    return (eid.id < s->sparse_size) && (s->sparse[eid.id] != de_null);
}


static void* de_storage_emplace(de_storage* s, ecs_entity e) {
    assert(s);
    assert(e != de_null);
    // now allocate the data for the new component at the end of the array
    s->cp_data = realloc(s->cp_data, (s->cp_data_size + 1) * sizeof(char) * s->cp_sizeof);
    s->cp_data_size++;

    // return the component data pointer (last position)
    void* cp_data_ptr = &((char*)s->cp_data)[(s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof];
    
    // Then add the entity to the sparse/dense arrays
    const uint32_t eid = ecs_entity_id(e);
    if (eid.id >= s->sparse_size) { // check if we need to realloc
        const size_t new_sparse_size = eid.id + 1;
        s->sparse = realloc(s->sparse, new_sparse_size * sizeof * s->sparse);
        memset(s->sparse + s->sparse_size, de_null, (new_sparse_size - s->sparse_size) * sizeof * s->sparse);
        s->sparse_size = new_sparse_size;
    }
    s->sparse[eid.id] = (ecs_entity)s->dense_size; // set this eid index to the last dense index (dense_size)
    s->dense = realloc(s->dense, (s->dense_size + 1) * sizeof * s->dense);
    s->dense[s->dense_size] = e;
    s->dense_size++;
    return cp_data_ptr;
}

static void de_storage_remove(de_storage* s, ecs_entity e) {
    assert(s);
    assert(de_storage_contains(s, e));
    
    // Remove from sparse/dense arrays
    const size_t pos_to_remove = s->sparse[ecs_entity_id(e).id];
    const ecs_entity other = s->dense[s->dense_size - 1];

    s->sparse[ecs_entity_id(other).id] = (ecs_entity)pos_to_remove;
    s->dense[pos_to_remove] = other;
    s->sparse[pos_to_remove] = de_null;

    s->dense = realloc(s->dense, (s->dense_size - 1) * sizeof * s->dense);
    s->dense_size--;


    // swap (memmove because if cp_data_size 1 it will overlap dst and source.
    memmove(
        &((char*)s->cp_data)[pos_to_remove * sizeof(char) * s->cp_sizeof],
        &((char*)s->cp_data)[(s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof],
        s->cp_sizeof);

    // and pop
    s->cp_data = realloc(s->cp_data, (s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof);
    s->cp_data_size--;
}

static void* de_storage_get_by_index(de_storage* s, size_t index) {
    assert(s);
    assert(index < s->cp_data_size);
    return &((char*)s->cp_data)[index * sizeof(char) * s->cp_sizeof];
}

static void* de_storage_get(de_storage* s, ecs_entity e) {
    assert(s);
    assert(e != de_null);
    assert(de_storage_contains(s, e));
    return de_storage_get_by_index(s, s->sparse[ecs_entity_id(e).id]);
}

static void* de_storage_try_get(de_storage* s, ecs_entity e) {
    assert(s);
    assert(e != de_null);
    return de_storage_contains(s, e) ? de_storage_get(s, e) : 0;
}



/*  de_registry

    Is the global context that holds each storage for each component types
    and the entities.
*/
typedef struct de_registry {
    de_storage** storages; /* array to pointers to storage */
    size_t storages_size; /* size of the storages array */
    size_t entities_size;
    ecs_entity* entities; /* contains all the created entities */
    uint32_t available_id; /* first index in the list to recycle */
} de_registry;

de_registry* de_registry_create() {
    de_registry* r = malloc(sizeof(de_registry));
    if (r) {
        r->storages = 0;
        r->storages_size = 0;
        r->available_id.id = de_null;
        r->entities_size = 0;
        r->entities = 0;
    }
    return r;
} 

void de_registry_destroy(de_registry* r) {
    if (r) {
        if (r->storages) {
            for (size_t i = 0; i < r->storages_size; i++) {
                de_storage_delete(r->storages[i]);
            }
        }
        free(r->entities);
    }
    free(r);
}

bool de_valid(de_registry* r, ecs_entity e) {
    assert(r);
    const uint32_t id = ecs_entity_id(e);
    return id.id < r->entities_size && r->entities[id.id] == e;
}

static ecs_entity _de_generate_entity(de_registry* r) {
    // can't create more identifiers entities
    assert(r->entities_size < ECS_ENTITY_ID_MASK);

    // alloc one more element to the entities array
    r->entities = realloc(r->entities, (r->entities_size + 1) * sizeof(ecs_entity));

    // create new entity and add it to the array
    const ecs_entity e = ecs_entity_assemble((uint32_t) {(uint32_t)r->entities_size}, (uint32_t) { 0 });
    r->entities[r->entities_size] = e;
    r->entities_size++;
    return e;
}

/* internal function to recycle a non used entity from the linked list */
static ecs_entity _de_recycle_entity(de_registry* r) {
    assert(r->available_id.id != de_null);
    // get the first available entity id
    const uint32_t curr_id = r->available_id;
    // retrieve the version
    const uint32_t curr_ver = ecs_entity_version(r->entities[curr_id.id]);
    // point the available_id to the "next" id
    r->available_id = ecs_entity_id(r->entities[curr_id.id]);
    // now join the id and version to create the new entity
    const ecs_entity recycled_e = ecs_entity_assemble(curr_id, curr_ver);
    // assign it to the entities array
    r->entities[curr_id.id] = recycled_e;
    return recycled_e;
}

static void _de_release_entity(de_registry* r, ecs_entity e, uint32_t desired_version) {
    const uint32_t e_id = ecs_entity_id(e);
    r->entities[e_id.id] = ecs_entity_assemble(r->available_id, desired_version);
    r->available_id = e_id;
}

ecs_entity s_ecs_create_enitty(de_registry* r) {
    assert(r);
    if (r->available_id.id == de_null) {
        return _de_generate_entity(r);
    } else {
        return _de_recycle_entity(r);
    }
}

de_storage* de_assure(de_registry* r, de_cp_type cp_type) {
    assert(r);
    de_storage* storage_found = 0;

    for (size_t i = 0; i < r->storages_size; i++) {
        if (r->storages[i]->cp_id == cp_type.cp_id) {
            storage_found = r->storages[i];
        }
    }

    if (storage_found) {
        return storage_found;
    } else {
        de_storage* storage_new = de_storage_new(cp_type.cp_sizeof, cp_type.cp_id);
        r->storages = realloc(r->storages, (r->storages_size + 1) * sizeof * r->storages);
        r->storages[r->storages_size] = storage_new;
        r->storages_size++;
        return storage_new;
    }
}

void de_remove_all(de_registry* r, ecs_entity e) {
    assert(r);
    assert(de_valid(r, e));
    
    for (size_t i = r->storages_size; i; --i) {
        if (r->storages[i - 1] && de_storage_contains(r->storages[i - 1], e)) {
            de_storage_remove(r->storages[i - 1], e);
        }
    }
}

void de_remove(de_registry* r, ecs_entity e, de_cp_type cp_type) {
    assert(false);
    assert(de_valid(r, e));
    de_storage_remove(de_assure(r, cp_type), e);
}

void de_destroy(de_registry* r, ecs_entity e) {
    assert(r);
    assert(e != de_null);

    // 1) remove all the components of the entity
    de_remove_all(r, e);

    // 2) release_entity with a desired new version
    uint32_t new_version = ecs_entity_version(e);
    new_version.ver++;
    _de_release_entity(r, e, new_version);
}

bool de_has(de_registry* r, ecs_entity e, de_cp_type cp_type) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_type));
    return de_storage_contains(de_assure(r, cp_type), e);
}

void* de_emplace(de_registry* r, ecs_entity e, de_cp_type cp_type) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_type));
    return de_storage_emplace(de_assure(r, cp_type), e);
}

void* de_get(de_registry* r, ecs_entity e, de_cp_type cp_type) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_type));
    return de_storage_get(de_assure(r, cp_type), e);
}

void* de_try_get(de_registry* r, ecs_entity e, de_cp_type cp_type) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_type));
    return de_storage_try_get(de_assure(r, cp_type), e);
}


void de_each(de_registry* r, void (*fun)(de_registry*, ecs_entity, void*), void* udata) {
    assert(r);
    if (!fun) {
        return;
    }

    if (r->available_id.id == de_null) {
        for (size_t i = r->entities_size; i; --i) {
            fun(r, r->entities[i - 1], udata);
        }
    } else {
        for (size_t i = r->entities_size; i; --i) {
            const ecs_entity e = r->entities[i - 1];
            if (ecs_entity_id(e).id == (i - 1)) {
                fun(r, e, udata);
            }
        }
    }
}

bool de_orphan(de_registry* r, ecs_entity e) {
    assert(r);
    assert(de_valid(r, e));
    for (size_t pool_i = 0; pool_i < r->storages_size; pool_i++) {
        if (r->storages[pool_i]) {
            if (de_storage_contains(r->storages[pool_i], e)) {
                return false;
            }
        }
    }
    return true;
}

/* Internal function to iterate orphans*/
typedef struct de_orphans_fun_data {
    void* orphans_udata;
    void (*orphans_fun)(de_registry*, ecs_entity, void*);
} de_orphans_fun_data;

static void _de_orphans_each_executor(de_registry* r, ecs_entity e, void* udata) {
    de_orphans_fun_data* orphans_data = udata;
    if (de_orphan(r, e)) {
        orphans_data->orphans_fun(r, e, orphans_data->orphans_udata);
    }
}

void de_orphans_each(de_registry* r, void (*fun)(de_registry*, ecs_entity, void*), void* udata) {
    de_each(r, _de_orphans_each_executor, &(de_orphans_fun_data) { .orphans_udata = udata, .orphans_fun = fun });
}

// VIEW SINGLE COMPONENT

de_view_single de_create_view_single(de_registry* r, de_cp_type cp_type) {
    assert(r);
    de_view_single v = { 0 };
    v.pool = de_assure(r, cp_type);
    assert(v.pool);

    de_storage* pool = (de_storage *)v.pool;
    if (pool->cp_data_size != 0) {
        // get the last entity of the pool
        v.entity_index = pool->cp_data_size - 1; 
        v.entity = pool->dense[v.entity_index];
    } else {
        v.entity_index = 0;
        v.entity = de_null;
    }
    return v;
}

bool de_view_single_valid(de_view_single* v) {
    assert(v);
    return (v->entity != de_null);
}

ecs_entity de_view_single_entity(de_view_single* v) {
    assert(v);
    return v->entity;
}

void* de_view_single_get(de_view_single* v) {
    assert(v);
    return de_storage_get_by_index(v->pool, v->entity_index);
}

void de_view_single_next(de_view_single* v) {
    assert(v);
    if (v->entity_index) {
        v->entity_index--;
        v->entity = ((de_storage*)v->pool)->dense[v->entity_index];
    } else {
        v->entity = de_null;
    }
}


/// VIEW MULTI COMPONENTS

bool de_view_entity_contained(de_view* v, ecs_entity e) {
    assert(v);
    assert(de_view_valid(v));

    for (size_t pool_id = 0; pool_id < v->pool_count; pool_id++) {
        if (!de_storage_contains(v->all_pools[pool_id], e)) { 
            return false; 
        }
    }
    return true;
}

size_t de_view_get_index(de_view* v, de_cp_type cp_type) {
    assert(v);
    for (size_t i = 0; i < v->pool_count; i++) {
        if (v->to_pool_index[i] == cp_type.cp_id) {
            return i;
        }
    }
    assert(0); // FIX (dani) cp not found in the view pools
    return 0;
}

void* de_view_get(de_view* v, de_cp_type cp_type) {
    return de_view_get_by_index(v, de_view_get_index(v, cp_type));
}

void* de_view_get_by_index(de_view* v, size_t pool_index) {
    assert(v);
    assert(pool_index >= 0 && pool_index < DE_MAX_VIEW_COMPONENTS);
    assert(de_view_valid(v));
    return de_storage_get(v->all_pools[pool_index], v->entity);
}

void de_view_next(de_view* v) {
    assert(v);
    assert(de_view_valid(v));
    // find the next contained entity that is inside all pools
    do {
        if (v->entity_index) {
            v->entity_index--;
            v->entity = ((de_storage*)v->pool)->dense[v->entity_index];
        }
        else {
            v->entity = de_null;
        }
    } while ((v->entity != de_null) && !de_view_entity_contained(v, v->entity));
}


de_view de_create_view(de_registry* r, size_t cp_count, de_cp_type *cp_types) {
    assert(r);
    assert(cp_count < DE_MAX_VIEW_COMPONENTS);
    

    de_view v = { 0 };
    v.pool_count = cp_count;
    // setup pools pointer and find the smallest pool that we 
    // use for iterations
    for (size_t i = 0; i < cp_count; i++) {
        v.all_pools[i] = de_assure(r, cp_types[i]);
        assert(v.all_pools[i]);
        if (!v.pool) {
            v.pool = v.all_pools[i];
        } else {
            if (((de_storage*)v.all_pools[i])->cp_data_size < ((de_storage*)v.pool)->cp_data_size) {
                v.pool = v.all_pools[i];
            }
        }
        v.to_pool_index[i] = cp_types[i].cp_id;
    }

    if (v.pool && ((de_storage*)v.pool)->cp_data_size != 0) {
        v.entity_index = ((de_storage*)v.pool)->cp_data_size - 1;
        v.entity = ((de_storage*)v.pool)->dense[v.entity_index];
        // now check if this entity is contained in all the pools
        if (!de_view_entity_contained(&v, v.entity)) {
            // if not, search the next entity contained
            de_view_next(&v);
        }
    } else {
        v.entity_index = 0;
        v.entity = de_null;
    }
    return v;
}

bool de_view_valid(de_view* v) {
    assert(v);
    return v->entity != de_null;
}

ecs_entity de_view_entity(de_view* v) {
    assert(v);
    assert(de_view_valid(v));
    return ((de_storage*)v->pool)->dense[v->entity_index];
}


void de_iterate(de_registry* r, de_cp_type cp_id, void (*fn)(de_registry*, size_t, ecs_entity*, void*)) {
    assert(r);
    de_storage* s = de_assure(r, cp_id);
    assert(s);

    if (fn) {
        fn(r, s->cp_data_size, s->dense, s->cp_data);
    }
}

typedef struct de_view2 {
    ecs_entity* entities;
    size_t size;
    void* cp_data;
    size_t cp_sizeof;
} de_view2;

de_view2 de_view_make(de_registry* r, de_cp_type cp_id) {
    assert(r);
    de_storage* s = de_assure(r, cp_id);
    assert(s);
    de_view2 ret = { 0 };
    ret.size = s->cp_data_size;
    ret.cp_data = s->cp_data;
    ret.entities = s->dense;
    ret.cp_sizeof = s->cp_sizeof;
    return ret;
}

void* de_view2_data(de_view2* v, size_t index) {
    return &((char*)v->cp_data)[index * v->cp_sizeof];
}

ecs_entity de_view2_entity(de_view2 v, size_t index) {
    v.entities[index];
}

#endif // DESTRAL_ECS_IMPL
#endif // DESTRAL_ECS_H
