#ifndef DESTRAL_ECS_H
#define DESTRAL_ECS_H
/*
    destral_ecs.hpp -- simple ECS system

    Do this:
        #define DESTRAL_ECS_IMPL
    before you include this file in *one* C++ file to create the implementation.

    FIX (dani) TODO proper instructions

*/

#include <cstdint>
#include <vector>
//#include <functional>

namespace ds {

/* Typedef for the entity type */
typedef uint32_t de_entity;

/* The de_null is a de_entity that represents a null entity. */
extern const de_entity de_null;

/*  ecs_registry

    Is the global context that holds each storage for each component types
    and the entities.
*/
typedef struct ecs_registry ecs_registry;
ecs_registry* ecs_registry_create(); /*  Allocates and initializes a ecs_registry context */
void ecs_registry_destroy(ecs_registry* r); /*  Deinitializes and frees a ecs_registry context */


//--------------------------------------------------------------------------------------------------
// Component
typedef void (ecs_cp_serialize_fn)(ecs_registry* r, de_entity e, void* cp, void* udata);
typedef void (ecs_cp_cleanup_fn)(ecs_registry* r, de_entity e, void* cp, void* udata);
void ecs_cp_register(ecs_registry* r, const char* name, size_t cp_sizeof, ecs_cp_serialize_fn* serialize_fn = nullptr, ecs_cp_cleanup_fn* cleanup_fn = nullptr);
#define ECS_CP_REGISTER(r,name,fun1,fun2) ecs_cp_register(r, #name, sizeof(name), fun1, fun2);


//--------------------------------------------------------------------------------------------------
// View
typedef struct ecs_cp_view ecs_cp_view;
de_entity ecs_view_entity(ecs_cp_view* v);
bool ecs_view_valid(ecs_cp_view* v);
size_t ecs_view_cp_index(ecs_cp_view* v, const char* cp_id);
void* ecs_view_cp(ecs_cp_view* v, size_t cp_idx);

//--------------------------------------------------------------------------------------------------
// System
typedef void (ecs_sys_update_fn) (ecs_registry* r, ecs_cp_view* view);
void ecs_sys_add(ecs_registry* r, const char* name, std::vector<const char*> cps, ecs_sys_update_fn* update_fn);
void ecs_run_systems(ecs_registry* r);

//--------------------------------------------------------------------------------------------------
// Entity
void ecs_entity_register(ecs_registry* r, const char* name, std::vector<const char*> cps);
de_entity ecs_entity_make(ecs_registry* r, const char* name);


struct ecs_cp_view {
    std::vector<void*> data;
    de_entity current_entity = de_null;


    struct ecs_view_impl {
        size_t current_entity_index = 0;
        de_entity* dense_array;
        size_t dense_array_size;
    } _impl;

    std::vector<ecs_cp_storage*> cp_storages;
    ecs_cp_storage* iterating_storage = nullptr;
};

/*  de_entity:

    Opaque 32 bits entity identifier.

    A 32 bits entity identifier guarantees:

    20 bits for the entity number(suitable for almost all the games).
    12 bit for the version(resets in[0 - 4095]).

    Use the functions de_entity_version and de_entity_identifier to retrieve
    each part of a de_entity.
*/











//
///*
//    Creates a new entity and returns it
//    The identifier can be:
//     - New identifier in case no entities have been previously destroyed
//     - Recycled identifier with an update version.
//*/
//de_entity de_create(ecs_registry* r);
//
///*
//    Destroys an entity.
//
//    When an entity is destroyed, its version is updated and the identifier
//    can be recycled when needed.
//
//    Warning:
//    Undefined behavior if the entity is not valid.
//*/
//void de_destroy(ecs_registry* r, de_entity e);
//
///*
//    Checks if an entity identifier is a valid one.
//    The entity e can be a valid or an invalid one.
//    Teturns true if the identifier is valid, false otherwise
//*/
//bool de_valid(ecs_registry* r, de_entity e);
//
///*
//    Removes all the components from an entity and makes it orphaned (no components in it)
//    the entity remains alive and valid without components.
//
//    Warning: attempting to use invalid entity results in undefined behavior
//*/
//void de_remove_all(ecs_registry* r, de_entity e);
//
///*
//    Assigns a given component type to an entity and returns it.
//
//    A new memory instance of component type cp_type is allocated for the
//    entity e and returned.
//
//    Note: the memory returned is only allocated not initialized.
//
//    Warning: use an invalid entity or assigning a component to a valid
//    entity that currently has this component instance id results in
//    undefined behavior.
//*/
//void* de_emplace(ecs_registry* r, de_entity e, de_cp_type cp_type);
//
///*
//    Removes the given component from the entity.
//
//    Warning: attempting to use invalid entity results in undefined behavior
//*/
//void de_remove(ecs_registry* r, de_entity e, de_cp_type cp_type);
//
///*
//    Checks if the entity has the given component
//
//    Warning: using an invalid entity results in undefined behavior.
//*/
//bool de_has(ecs_registry* r, de_entity e, de_cp_type cp_type);
//
///*
//    Returns the pointer to the given component type data for the entity
//
//    Warning: Using an invalid entity or get a component from an entity
//    that doesn't own it results in undefined behavior.
//
//    Note: This is the fastest way of retrieveing component data but
//    has no checks. This means that you are 100% sure that the entity e
//    has the component emplaced. Use de_try_get to check if you want checks
//*/
//void* de_get(ecs_registry* r, de_entity e, de_cp_type cp_type);
//
///*
//    Returns the pointer to the given component type data for the entity
//    or nullptr if the entity doesn't have this component.
//
//    Warning: Using an invalid entity results in undefined behavior.
//    Note: This is safer but slower than de_get.
//*/
//void* de_try_get(ecs_registry* r, de_entity e, de_cp_type cp_type);
//
///*
//    Iterates all the entities that are still in use and calls
//    the function pointer for each one.
//
//    This is a fairly slow operation and should not be used frequently.
//    However it's useful for iterating all the entities still in use,
//    regarding their components.
//*/
//void de_each(ecs_registry* r, void (*fun)(ecs_registry*, de_entity, void*), void* udata);
//
////
////void de_iterate(ecs_registry* r, de_cp_type cp_id, void (*fn)(ecs_registry, size_t, de_entity *, void*));
//
///*
//    de_view_single
//
//    Use this view to iterate entities that have the component type specified.
//*/
//typedef struct de_view_single {
//    void* pool; // de_storage opaque pointer
//    size_t current_entity_index;
//    de_entity entity;
//} de_view_single;
//
//de_view_single de_create_view_single(ecs_registry* r, de_cp_type cp_type);
//bool de_view_single_valid(de_view_single* v);
//de_entity de_view_single_entity(de_view_single* v);
//void* de_view_single_get(de_view_single* v);
//void de_view_single_next(de_view_single* v);
//
//
//
///*
//    de_view
//
//    Use this view to iterate entities that have multiple component types specified.
//
//    Note: You don't need to destroy the view because it doesn't allocate and it
//    is not recommended that you save a view. Just use de_create_view each time.
//    It's a "cheap" operation.
//
//    Example usage with two components:
//
//    for (de_view v = de_create_view(r, 2, (de_cp_type[2]) {transform_type, velocity_type }); de_view_valid(&v); de_view_next(&v)) {
//        de_entity e = de_view_entity(&v);
//        transform* tr = de_view_get(&v, transform_type);
//        velocity* tc = de_view_get(&v, velocity_type);
//        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, tr->x, tr->y, tr->z);
//        printf("velocity  entity: %d => w=%f\n", de_entity_identifier(e).id, tc->v);
//    }
//
//*/
//typedef struct de_view {
//    #define DE_MAX_VIEW_COMPONENTS (16)
//    /* value is the component id, index is where is located in the all_pools array */
//    size_t to_pool_index[DE_MAX_VIEW_COMPONENTS];
//    void* all_pools[DE_MAX_VIEW_COMPONENTS]; // de_storage opaque pointers
//    size_t pool_count;
//    void* pool; // de_storage opaque pointer
//    size_t current_entity_index;
//    de_entity current_entity;
//} de_view;
//
//
//de_view de_create_view(ecs_registry* r, size_t cp_count, de_cp_type* cp_types);
//bool de_view_valid(de_view* v);
//de_entity de_view_entity(de_view* v);
//void* de_view_get(de_view* v, de_cp_type cp_type);
//void* de_view_get_by_index(de_view* v, size_t pool_index);
//void de_view_next(de_view* v);


}
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

//#include <assert.h>
//#include <string.h>
//#include <stdlib.h>

#include <cassert>
#include <vector>
#include <string>
#include <unordered_map>


namespace ds {

typedef struct de_entity_ver { uint32_t ver; } de_entity_ver;
typedef struct de_entity_id { uint32_t id; } de_entity_id;

/* Masks for retrieving the id and the version of an entity */
#define DE_ENTITY_ID_MASK       ((uint32_t)0xFFFFF) /* Mask to use to get the entity number out of an identifier.*/  
#define DE_ENTITY_VERSION_MASK  ((uint32_t)0xFFF)   /* Mask to use to get the version out of an identifier. */
#define DE_ENTITY_SHIFT         ((size_t)20)        /* Extent of the entity number within an identifier. */   



const de_entity de_null = (de_entity)DE_ENTITY_ID_MASK;
/* de_entity utilities */

/* Returns the version part of the entity */
de_entity_ver de_entity_version(de_entity e);

/* Returns the id part of the entity */
de_entity_id de_entity_identifier(de_entity e);

/* Makes a de_entity from entity_id and entity_version */
de_entity de_make_entity(de_entity_id id, de_entity_ver version);

/* Returns the version part of the entity */
de_entity_ver de_entity_version(de_entity e) { return { .ver = e >> DE_ENTITY_SHIFT }; }
/* Returns the id part of the entity */
de_entity_id de_entity_identifier(de_entity e) { return { .id = e & DE_ENTITY_ID_MASK }; }
/* Makes a de_entity from entity_id and entity_version */
de_entity de_make_entity(de_entity_id id, de_entity_ver version) { return id.id | (version.ver << DE_ENTITY_SHIFT); }

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
    dense => contains all the entities (de_entity).
    the index is just that, has no meaning here, it's referenced in the sparse.
    the content is the de_entity.

    this allows fast iteration on each entity using the dense array or
    lookup for an entity position in the dense using the sparse array.

    ---------- Example:
    Adding:
     de_entity = 3 => (e3)
     de_entity = 1 => (e1)

    In order to check the entities first in the sparse, we have to retrieve the de_entity_id part of the de_entity.
    The de_entity_id part will be used to index the sparse array.
    The full de_entity will be the value in the dense array.

                           0    1     2    3
    sparse idx:         eid0 eid1  eid2  eid3    this is the array index based on de_entity_id (NO VERSION)
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
        de_entity e = dense[i];
        void*   ecp = cp_data[i];
    }


*/
typedef struct de_storage {
    std::string name; /* component name */
    size_t cp_id = 0; /* component id for this storage */
    size_t cp_sizeof = 0; /* sizeof for each cp_data element */

    /*  packed component elements array. aligned with sparse->dense*/
    std::vector<char> cp_data;

    /*  sparse entity identifiers indices array.
    - index is the de_entity_id. (without version)
    - value is the index of the dense array
    */
    std::vector<de_entity> sparse;

    /*  Dense entities array.
        - index is linked with the sparse value.
        - value is the full de_entity
    */
    std::vector<de_entity> dense;
} de_storage;

struct ecs_cp_storage : public de_storage {
    // Fix delete de_storage and use ecs_cp_storage
    ecs_cp_serialize_fn* serialize_fn = nullptr;
    ecs_cp_cleanup_fn* cleanup_fn = nullptr;
};



static de_storage* de_storage_new(size_t cp_size, size_t cp_id) {
    de_storage* s = new de_storage();
    if (s) {
        s->cp_sizeof = cp_size;
        s->cp_id = cp_id;
    }
    return s;
}

static void de_storage_delete(de_storage* s) {
    delete s;
}

static bool de_storage_contains(de_storage* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    const de_entity_id eid = de_entity_identifier(e);
    return (eid.id < s->sparse.size()) && (s->sparse[eid.id] != de_null);
}


static void* de_storage_emplace(de_storage* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    // now allocate the data for the new component at the end of the array
    // Fix (dani), we can't resize std::vector without initialization.. 
    // maybe we will need a custom vector that only mallocs without initialization
    s->cp_data.resize(s->cp_data.size() + (s->cp_sizeof));
    //s->cp_data = realloc(s->cp_data, (s->cp_data_size + 1) * sizeof(char) * s->cp_sizeof);
    //s->cp_data_size++;

    // return the component data pointer (last position of the component sizes)
    void* cp_data_ptr = &s->cp_data[s->cp_data.size() - s->cp_sizeof];
    //void* cp_data_ptr = &((char*)s->cp_data)[(s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof];

    // Then add the entity to the sparse/dense arrays
    const de_entity_id eid = de_entity_identifier(e);
    if (eid.id >= s->sparse.size()) { // check if we need to realloc
        s->sparse.resize(eid.id + 1, de_null);
        //const size_t new_sparse_size = eid.id + 1;
        //s->sparse = realloc(s->sparse, new_sparse_size * sizeof * s->sparse);
        //memset(s->sparse + s->sparse_size, de_null, (new_sparse_size - s->sparse_size) * sizeof * s->sparse);
        //s->sparse_size = new_sparse_size;
    }
    s->sparse.at(eid.id) =(de_entity) s->dense.size();
    //s->sparse[eid.id] = (de_entity)s->dense_size; // set this eid index to the last dense index (dense_size)

    s->dense.push_back(e);

    /*s->dense = realloc(s->dense, (s->dense_size + 1) * sizeof * s->dense);
    s->dense[s->dense_size] = e;
    s->dense_size++;*/
    return cp_data_ptr;
}

static void de_storage_remove(de_storage* s, de_entity e) {
    assert(s);
    assert(de_storage_contains(s, e));
    
    // Remove from sparse/dense arrays
    const de_entity pos_to_remove = s->sparse[de_entity_identifier(e).id];

    const de_entity other = s->dense.back();
    //const de_entity other = s->dense[s->dense_size - 1];


    //
    s->sparse[de_entity_identifier(other).id] = pos_to_remove;
    //s->sparse[de_entity_identifier(other).id] = (de_entity)pos_to_remove;
    s->dense[pos_to_remove] = other;
    s->sparse[pos_to_remove] = de_null;
    

    s->dense.pop_back();
    //s->dense = realloc(s->dense, (s->dense_size - 1) * sizeof * s->dense);
    //s->dense_size--;


    // swap (memmove because if cp_data_size 1 it will overlap dst and source.
    memmove(
        &(s->cp_data)[pos_to_remove * s->cp_sizeof],
        &(s->cp_data)[(s->cp_data.size() - s->cp_sizeof)],
        s->cp_sizeof);



    //memmove(
    //    &((char*)s->cp_data)[pos_to_remove * sizeof(char) * s->cp_sizeof],
    //    &((char*)s->cp_data)[(s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof],
    //    s->cp_sizeof);

    // and pop
    s->cp_data.pop_back();
    /*s->cp_data = realloc(s->cp_data, (s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof);
    s->cp_data_size--;*/
}

static void* de_storage_get_by_index(de_storage* s, size_t index) {
    assert(s);
    assert((index * s->cp_sizeof) < s->cp_data.size());
    return &(s->cp_data)[index * s->cp_sizeof];
}

static void* de_storage_get(de_storage* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    assert(de_storage_contains(s, e));
    return de_storage_get_by_index(s, s->sparse[de_entity_identifier(e).id]);
}

static void* de_storage_try_get(de_storage* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    return de_storage_contains(s, e) ? de_storage_get(s, e) : 0;
}


struct de_cp_type {
    size_t cp_id; // component unique id
    size_t cp_sizeof; // component sizeof
};


struct ecs_entity_type {
    std::string name;
    /* Holds the component ids hashed using std::hash<std::string> */
    std::vector<size_t> cp_ids;
};

struct ecs_system {
    std::string name;
    /* Holds the component ids hashed using std::hash<std::string> */
    std::vector<size_t> cp_ids;

    ecs_sys_update_fn* update_fn = nullptr;
};

/*  ecs_registry

    Is the global context that holds each storage for each component types
    and the entities.
*/
typedef struct ecs_registry {
    
    std::vector<de_storage*> storages;
    //de_storage** storages; /* array to pointers to storage */
    //size_t storages_size; /* size of the storages array */

    /* contains all the created entities */
    std::vector<de_entity> entities;

    //size_t entities_size;
    //de_entity* entities; 

    /* first index in the list to recycle */
    de_entity_id available_id = { de_null };

    /* Hold the component storages systems (in order) */
    std::unordered_map<size_t, ecs_cp_storage> cp_storages;

    /* Hold the registered systems (in order) */
    std::vector<ecs_system> systems;

    /* Hold the registered entity types */
    std::unordered_map<size_t, ecs_entity_type> types;
} ecs_registry;


ecs_registry* ecs_registry_create() {
    ecs_registry* r = new ecs_registry();
    return r;
} 

void ecs_registry_destroy(ecs_registry* r) {
    if (r) {
        for (size_t i = 0; i < r->storages.size(); i++) {
            de_storage_delete(r->storages[i]);
        }
    }
    delete r;
}

bool de_valid(ecs_registry* r, de_entity e) {
    assert(r);
    const de_entity_id id = de_entity_identifier(e);
    return (id.id < r->entities.size()) && (r->entities[id.id] == e);
}

static de_entity _de_generate_entity(ecs_registry* r) {
    // can't create more entity identifiers
    assert(r->entities.size() < DE_ENTITY_ID_MASK);

    // alloc one more element to the entities array
    //r->entities = realloc(r->entities, (r->entities_size + 1) * sizeof(de_entity));

    // create new entity and add it to the array
    //const de_entity e = de_make_entity((de_entity_id) {(uint32_t)r->entities_size}, (de_entity_ver) { 0 });
    const de_entity e = de_make_entity({ (uint32_t) r->entities.size() }, { 0 });
    r->entities.push_back(e);

    /*r->entities[r->entities_size] = e;
    r->entities_size++;*/

    return e;
}

/* internal function to recycle a non used entity from the linked list */
static de_entity _de_recycle_entity(ecs_registry* r) {
    assert(r->available_id.id != de_null);
    // get the first available entity id
    const de_entity_id curr_id = r->available_id;
    // retrieve the version
    const de_entity_ver curr_ver = de_entity_version(r->entities[curr_id.id]);
    // point the available_id to the "next" id
    r->available_id = de_entity_identifier(r->entities[curr_id.id]);
    // now join the id and version to create the new entity
    const de_entity recycled_e = de_make_entity(curr_id, curr_ver);
    // assign it to the entities array
    r->entities[curr_id.id] = recycled_e;
    return recycled_e;
}

static void _de_release_entity(ecs_registry* r, de_entity e, de_entity_ver desired_version) {
    const de_entity_id e_id = de_entity_identifier(e);
    r->entities[e_id.id] = de_make_entity(r->available_id, desired_version);
    r->available_id = e_id;
}

de_entity de_create(ecs_registry* r) {
    assert(r);
    if (r->available_id.id == de_null) {
        return _de_generate_entity(r);
    } else {
        return _de_recycle_entity(r);
    }
}

/*  de_cp_type:
    Component Type identifier information.
*/

/* Returns the storage pointer for the given cp id or nullptr if not exists */
static ecs_cp_storage* s_ecs_get_storage(ecs_registry* r, size_t cp_id) {
    assert(r);
    if (!r->cp_storages.contains(cp_id)) {
        return nullptr;
    }
    return &r->cp_storages[cp_id];
}

static void* de_emplace(ecs_registry* r, de_entity e, size_t cp_id) {
    assert(r);
    assert(de_valid(r, e));
    auto st = s_ecs_get_storage(r, cp_id);
    assert(st);
    return de_storage_emplace(st, e);
}


de_storage* de_assure(ecs_registry* r, de_cp_type cp_type) {
    assert(r);
    de_storage* storage_found = 0;

    for (size_t i = 0; i < r->storages.size(); i++) {
        if (r->storages[i]->cp_id == cp_type.cp_id) {
            storage_found = r->storages[i];
        }
    }

    if (storage_found) {
        return storage_found;
    } else {
        de_storage* storage_new = de_storage_new(cp_type.cp_sizeof, cp_type.cp_id);
        r->storages.push_back(storage_new);
        /*r->storages = realloc(r->storages, (r->storages_size + 1) * sizeof * r->storages);
        r->storages[r->storages_size] = storage_new;
        r->storages_size++;*/
        return storage_new;
    }
}

void de_remove_all(ecs_registry* r, de_entity e) {
    assert(r);
    assert(de_valid(r, e));
    
    for (size_t i = r->storages.size(); i; --i) {
        if (r->storages[i - 1] && de_storage_contains(r->storages[i - 1], e)) {
            de_storage_remove(r->storages[i - 1], e);
        }
    }
}

void de_remove(ecs_registry* r, de_entity e, de_cp_type cp_type) {
    assert(false);
    assert(de_valid(r, e));
    de_storage_remove(de_assure(r, cp_type), e);
}

void de_destroy(ecs_registry* r, de_entity e) {
    assert(r);
    assert(e != de_null);

    // 1) remove all the components of the entity
    de_remove_all(r, e);

    // 2) release_entity with a desired new version
    de_entity_ver new_version = de_entity_version(e);
    new_version.ver++;
    _de_release_entity(r, e, new_version);
}

bool de_has(ecs_registry* r, de_entity e, de_cp_type cp_type) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_type));
    return de_storage_contains(de_assure(r, cp_type), e);
}



void* de_get(ecs_registry* r, de_entity e, de_cp_type cp_type) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_type));
    return de_storage_get(de_assure(r, cp_type), e);
}

void* de_try_get(ecs_registry* r, de_entity e, de_cp_type cp_type) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_type));
    return de_storage_try_get(de_assure(r, cp_type), e);
}


void de_each(ecs_registry* r, void (*fun)(ecs_registry*, de_entity, void*), void* udata) {
    assert(r);
    if (!fun) {
        return;
    }

    if (r->available_id.id == de_null) {
        for (size_t i = r->entities.size(); i; --i) {
            fun(r, r->entities[i - 1], udata);
        }
    } else {
        for (size_t i = r->entities.size(); i; --i) {
            const de_entity e = r->entities[i - 1];
            if (de_entity_identifier(e).id == (i - 1)) {
                fun(r, e, udata);
            }
        }
    }
}





/* hash a const char * using std::hash<std::string> */
static size_t s_ecs_hash_cstring(const char* str) {
    std::hash<std::string> str_hash;
    return str_hash(str);
}

void ecs_entity_register(ecs_registry* r, const char* name, std::vector<const char*> cps) {
    assert(r);
    assert(name);
    const size_t type_id = s_ecs_hash_cstring(name);
    assert(!r->types.contains(type_id)); // you are trying to register an existing entity type
    ecs_entity_type et;
    et.name = name;
    for (size_t i = 0; i < cps.size(); ++i) {
        et.cp_ids.push_back(s_ecs_hash_cstring(cps[i]));
    }
    r->types.insert({ type_id, et});
}

de_entity ecs_entity_make(ecs_registry* r, const char* name) {
    assert(r);
    const size_t type_id = s_ecs_hash_cstring(name);
    assert(r->types.contains(type_id));

    // create the entity and emplace all the components
    ecs_entity_type* type = &r->types[type_id];
    de_entity e = de_create(r);
    for (size_t i = 0; i < type->cp_ids.size(); ++i) {
        const size_t cp_id = type->cp_ids[i];
        auto st = s_ecs_get_storage(r, cp_id);
        assert(st);
        void* cp_data = de_storage_emplace(st, e);
        // TODO call serialize function for component
    }
    return e;
}

void ecs_cp_register(ecs_registry* r, const char* name, size_t cp_sizeof,
    ecs_cp_serialize_fn* serialize_fn, ecs_cp_cleanup_fn* cleanup_fn) {
    assert(r);
    assert(name);
    const size_t cp_id = s_ecs_hash_cstring(name);
    assert(!r->cp_storages.contains(cp_id)); // you are trying to register a component with the same name/id
    ecs_cp_storage cp_storage;
    cp_storage.name = name;
    cp_storage.cp_id = cp_id;
    cp_storage.cp_sizeof = cp_sizeof;
    cp_storage.serialize_fn = serialize_fn;
    cp_storage.cleanup_fn = cleanup_fn;
    r->cp_storages.insert({ cp_id, cp_storage });
}

void ecs_sys_add(ecs_registry* r, const char* name, std::vector<const char*> cps, 
    ecs_sys_update_fn* update_fn = nullptr) {
    assert(name);

    // hash all the cps using hash<std::string>
    ecs_system s;
    s.name = name;
    s.update_fn = update_fn;
    for (size_t i = 0; i < cps.size(); ++i) {
        s.cp_ids.push_back(s_ecs_hash_cstring(cps[i]));
    }
    r->systems.push_back(s);
}

//struct ecs_cp_view {
//    std::vector<ecs_cp_storage*> cp_storages;
//    ecs_cp_storage* iterating_storage = nullptr;
//    size_t current_entity_index = 0;
//    de_entity current_entity = de_null;
//};

bool ecs_view_valid(ecs_cp_view* v) {
    assert(v);
    return v->current_entity != de_null;
}

de_entity ecs_view_entity(ecs_cp_view* v) {
    assert(v);
    return v->current_entity;
}

size_t ecs_view_cp_index(ecs_cp_view* v, const char* cp_id) {
    assert(v);
    assert(cp_id);
    const size_t cp_id_hashed = s_ecs_hash_cstring(cp_id);
    for (size_t i = 0; i < v->cp_storages.size(); i++) {
        if (v->cp_storages[i]->cp_id == cp_id_hashed) {
            return i;
        }
    }
    // error path no component with that cp_id in this view
    assert(0);
    return 0;
}


void* ecs_view_cp(ecs_cp_view* v, size_t cp_idx) {
    assert(v);
    assert(cp_idx < v->cp_storages.size());
    return de_storage_get(v->cp_storages[cp_idx], v->current_entity);
}


void ecs_view_next(ecs_cp_view* v) {
    assert(v);
    assert(ecs_view_valid(v));
    // find the next contained entity that is inside all pools
    bool entity_contained = false;
    do {
        if (v->current_entity_index < v->iterating_storage->dense.size() - 1) {
            // select next entity from the iterating storage (smaller one..)
            ++v->current_entity_index;
            v->current_entity = v->iterating_storage->dense[v->current_entity_index];

            // now check if the entity is contained in ALL other storages:
            entity_contained = true;
            for (size_t st_id = 0; st_id < v->cp_storages.size(); st_id++) {
                if (!de_storage_contains(v->cp_storages[st_id], v->current_entity)) {
                    entity_contained = false;
                    break;
                }
            }
        } else {
            v->current_entity = de_null;
        }
    } while ((v->current_entity != de_null) && !entity_contained);
}


void ecs_run_systems(ecs_registry* r) {
    assert(r);
    for (size_t i = 0; i < r->systems.size(); ++i) {
        ecs_system* sys = &r->systems[i];

        // if no component ids for this system or no update function set skip it
        if ((sys->cp_ids.size() == 0) || (sys->update_fn == nullptr)) {
            continue;
        }


        // SETUP VIEW: create the view to iterate components (setup storages
        ecs_cp_view view;
        // TODO BREAKING ERROR. Esta malament la view perke al crear-la no considero que la primera entity estigui en tots els components
        // FIX create an internal function to create a view
        // Retrieve all the system storages for component ids for this system
        // and find the shorter storage (the one with less entities to iterate)
        view.cp_storages.reserve(sys->cp_ids.size());
        for (size_t cp_id_idx = 0; cp_id_idx < sys->cp_ids.size(); ++cp_id_idx) {
            const size_t cp_id = sys->cp_ids[cp_id_idx];
            ecs_cp_storage* cp_storage = s_ecs_get_storage(r, cp_id);
            assert(cp_storage); // no component storage for this cp_id!!!

            // find the shorter storage to iterate it
            if (view.iterating_storage == nullptr) {
                view.iterating_storage = cp_storage;
            } else if (cp_storage->dense.size() < view.iterating_storage->dense.size()) {
                view.iterating_storage = cp_storage;
            }
            view.cp_storages.push_back(cp_storage);
        }

        assert(view.iterating_storage);
        view.current_entity_index = 0;
        if (view.iterating_storage->dense.size() == 0) {
            // mark the view as non valid
            view.current_entity = de_null;
        } else {
            view.current_entity = view.iterating_storage->dense[view.current_entity_index];
        }

        // execute function
        sys->update_fn(r, &view);
    }
}




}
#endif // DESTRAL_ECS_IMPL
#endif // DESTRAL_ECS_H
