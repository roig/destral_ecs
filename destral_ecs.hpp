#ifndef DESTRAL_ECS_H
#define DESTRAL_ECS_H
/*
    destral_ecs.hpp -- simple ECS system

    Do this:
        #define DESTRAL_ECS_IMPL
    before you include this file in *one* C++ file to create the implementation.

    FIX (dani) TODO proper instructions

    TODO: Construction and initialization of components.
    TODO: Construction and initialization of entities.
    TODO: Delayed construction of entities.
    TODO: Delayed destruction of entities.
    TODO: Something like entt::basic_handle: https://github.com/skypjack/entt/blob/master/src/entt/entity/handle.hpp


    Construction of an entity:
    1 -> create the entity in the registry
    2 -> construct (placement new) and serialize (default initialize its components)
    --- now all the components are constructed and default initialized for that entity ---
    3 -> call user initialize function (this should be a callback provided in ecs_entity_make)
        The purpose of this function is to allow to initialize some variables on a per instance or to be used in the step 4. 
        It´s like a custom constructor before the real entity constructor in step 4.
    4 -> call initialize entity function. (this should be a callback set when registering a new entity type)
        this is like a constructor by parameter for that entity, because it can read the config variables set by step 3
        and configure all the components needed based on those configuration variables. It can be null of course.

    
    Example:

    struct pos2d {
        int x = 1;
        int y = 2;
        static void placement_new() {placement_new()...}
        static void serialize() { read from key value pairs..  }
        // no initialize needed here
    };

    struct player {
        std::string name = "Unknown";
        int startingx = 0;
        static void placement_new() {placement_new()...}
        static void serialize() { read from key value pairs..  }

        // this function is not from the component... its for the entity..
        static void initialize() {
            get_cp(pos2d)->x = startingx;
        }

        
    };

    register_cp(pos2d, pos2d::serialize);
    register_cp(player, player::serialize);


    register_entity_type("Text2D", {{"pos2d", "text_rd"}}, player::initialize );



*/

#include <cstdint>
#include <vector>
#include <string>
#include <cassert>

namespace ds {
//--------------------------------------------------------------------------------------------------
// Configuration

/* Typedef for the opaque entity type, contains the version and id*/
/* Masks for retrieving the id and the version of an entity */
typedef uint32_t ecs_entity;
#define ECS_ENTITY_ID_MASK       ((ecs_entity)0xFFFFF) /* Mask to use to get the entity number out of an identifier.*/  
#define ECS_ENTITY_VERSION_MASK  ((ecs_entity)0xFFF)   /* Mask to use to get the version out of an identifier. */
#define ECS_ENTITY_SHIFT         ((size_t)20)        /* Extent of the entity number within an identifier. */   

/* The entity_null is a ecs_entity that represents a null entity. */
static constexpr ecs_entity entity_null = ECS_ENTITY_ID_MASK;

//--------------------------------------------------------------------------------------------------
// Registry: global context that holds each storage for each component types and the entities.

typedef struct ecs_registry ecs_registry;
ecs_registry* ecs_registry_create(); /*  Allocates and initializes a ecs_registry context */
void ecs_registry_destroy(ecs_registry* r); /*  Deinitializes and frees a ecs_registry context */


//--------------------------------------------------------------------------------------------------
// Component
#define ECS_CP_REGISTER(r,name,fun1,fun2) ecs_cp_register(r, #name, sizeof(name), fun1, fun2);
typedef void (ecs_cp_serialize_fn)(ecs_registry* r, ecs_entity e, void* cp, void* udata);
typedef void (ecs_cp_cleanup_fn)(ecs_registry* r, ecs_entity e, void* cp, void* udata);
void ecs_cp_register(ecs_registry* r, const char* name, size_t cp_sizeof, ecs_cp_serialize_fn* serialize_fn = nullptr, ecs_cp_cleanup_fn* cleanup_fn = nullptr);


//--------------------------------------------------------------------------------------------------
// Components View
typedef struct ecs_view ecs_view;
ecs_view ecs_view_create(ecs_registry* r, std::vector<const char*> cps);

//--------------------------------------------------------------------------------------------------
// System
typedef void (ecs_sys_update_fn) (ecs_registry* r, ecs_view* view);
void ecs_sys_add(ecs_registry* r, const char* name, std::vector<const char*> cps, ecs_sys_update_fn* update_fn);
void ecs_run_systems(ecs_registry* r);

//--------------------------------------------------------------------------------------------------
// Entity

// Registers an entity type with it's components
void ecs_entity_register(ecs_registry* r, const char* name, std::vector<const char*> cps);
// Instantiates an entity
ecs_entity ecs_entity_make(ecs_registry* r, const char* name);
// Returns the component cp for the entity e. If entity has not the cp, undefined behaviour. Use ecs_entity_try_get instead
void* ecs_entity_get(ecs_registry* r, ecs_entity e, const char* cp);
// Returns the component cp for the entity e if it exists or nullptr.
void* ecs_entity_try_get(ecs_registry* r, ecs_entity e, const char* cp);
// Returns true only if the entity is valid. Valid means that registry has created it. 
bool ecs_entity_valid(ecs_registry* r, ecs_entity e);

//bool ecs_entity_mark_delete(ecs_registry* r, ecs_entity e);
//bool ecs_entity_has_delete(ecs_registry* r, ecs_entity e);
//void ecs_delete_entities(ecs_registry* r);

/* Returns the version part of the entity */
constexpr uint32_t ecs_entity_version(ecs_entity e) { return e >> ECS_ENTITY_SHIFT; }
/* Returns the id part of the entity */
constexpr uint32_t ecs_entity_id(ecs_entity e) { return { e & ECS_ENTITY_ID_MASK }; }
/* Makes a ecs_entity from entity_id and entity_version */
constexpr ecs_entity ecs_entity_assemble(uint32_t id, uint32_t version) { return id | (version << ECS_ENTITY_SHIFT); }

// Template Helpers:
template <typename T> T* ecs_entity_get(ecs_registry* r, ecs_entity e, const char* cp) {
    return (T*)ecs_entity_get(r, e, cp);
}
template <typename T> T* ecs_entity_try_get(ecs_registry* r, ecs_entity e, const char* cp) {
    return (T*)ecs_entity_try_get(r, e, cp);
}


// Implementation details here...

namespace impl {

/* hash a const char * using std::hash<std::string_view> */
size_t hash_cstring(const char* str);

struct ecs_storage {
    std::string name; /* component name */
    size_t cp_id = 0; /* component id for this storage */
    size_t cp_sizeof = 0; /* sizeof for each cp_data element */

    /*  packed component elements array. aligned with sparse->dense*/
    std::vector<char> cp_data;

    /*  sparse entity identifiers indices array.
    - index is the uint32_t. (without version)
    - value is the index of the dense array
    */
    std::vector<ecs_entity> sparse;

    /*  Dense entities array.
        - index is linked with the sparse value.
        - value is the full ecs_entity
    */
    std::vector<ecs_entity> dense;

    // TODO Fix delete ecs_storage and use ecs_cp_storage
    ecs_cp_serialize_fn* serialize_fn = nullptr;
    ecs_cp_cleanup_fn* cleanup_fn = nullptr;

    inline bool contains(ecs_entity e) {
        assert(e != entity_null);
        const uint32_t eid = ecs_entity_id(e);
        return (eid < sparse.size()) && (sparse[eid] != entity_null);
    }


    inline void* emplace(ecs_entity e) {
        assert(e != entity_null);
        // now allocate the data for the new component at the end of the array
        cp_data.resize(cp_data.size() + (cp_sizeof));

        // return the component data pointer (last position of the component sizes)
        void* cp_data_ptr = &cp_data[cp_data.size() - cp_sizeof];

        // Then add the entity to the sparse/dense arrays
        const uint32_t eid = ecs_entity_id(e);
        if (eid >= sparse.size()) { // check if we need to realloc
            sparse.resize(eid + 1, entity_null);
        }
        sparse.at(eid) = (ecs_entity)dense.size();
        dense.push_back(e);
        return cp_data_ptr;
    }

    inline void remove(ecs_entity e) {
        assert(contains(e));
        // Remove from sparse/dense arrays
        const ecs_entity pos_to_remove = sparse[ecs_entity_id(e)];
        const ecs_entity other = dense.back();
        sparse[ecs_entity_id(other)] = pos_to_remove;
        dense[pos_to_remove] = other;
        sparse[pos_to_remove] = entity_null;
        dense.pop_back();
        // swap (memmove because if cp_data_size 1 it will overlap dst and source.
        memmove(
            &(cp_data)[pos_to_remove * cp_sizeof],
            &(cp_data)[(cp_data.size() - cp_sizeof)],
            cp_sizeof);
        // and pop
        cp_data.pop_back();
    }

    inline void* get_by_index(size_t index) {
        assert((index * cp_sizeof) < cp_data.size());
        return &(cp_data)[index * cp_sizeof];
    }

    inline void* get(ecs_entity e) {
        assert(e != entity_null);
        assert(contains(e));
        return get_by_index(sparse[ecs_entity_id(e)]);
    }

    inline void* try_get(ecs_entity e) {
        assert(e != entity_null);
        return contains(e) ? get(e) : 0;
    }


};

}


struct ecs_view {
    inline bool valid() {
        return _impl.entity != entity_null;
    }

    inline ecs_entity entity() {
        return _impl.entity;
    }

    inline size_t index(const char* cp_id) {
        assert(cp_id);
        const size_t cp_id_hashed = impl::hash_cstring(cp_id);
        for (size_t i = 0; i < _impl.cp_storages.size(); i++) {
            if (_impl.cp_storages[i]->cp_id == cp_id_hashed) {
                return i;
            }
        }
        // error, no component with that cp_id in this view!
        assert(0);
        return 0;
    }

    template <typename T>
    inline T* data(size_t cp_idx) {
        assert(valid());
        assert(cp_idx < _impl.cp_storages.size());
        return (T*)_impl.cp_storages[cp_idx]->get(_impl.entity);
    }

    inline void next() {
        assert(valid());
        // find the next contained entity that is inside all pools
        bool entity_contained = false;
        do {
            if (_impl.entity_index < _impl.iterating_storage->dense.size() - 1) {
                // select next entity from the iterating storage (smaller one..)
                ++_impl.entity_index;
                _impl.entity = _impl.iterating_storage->dense[_impl.entity_index];

                // now check if the entity is contained in ALL other storages:
                entity_contained = true;
                for (size_t st_id = 0; st_id < _impl.cp_storages.size(); st_id++) {
                    if (!_impl.cp_storages[st_id]->contains(_impl.entity)) {
                        entity_contained = false;
                        break;
                    }
                }
            }
            else {
                _impl.entity = entity_null;
            }
        } while ((_impl.entity != entity_null) && !entity_contained);
    }

    struct view_impl {
        std::vector<impl::ecs_storage*> cp_storages;
        impl::ecs_storage* iterating_storage = nullptr;
        size_t entity_index = 0;
        ecs_entity entity = entity_null;
    };
    view_impl _impl;
};
}

/**************** Implementation ****************/
//#define DESTRAL_ECS_IMPL
#ifdef DESTRAL_ECS_IMPL


/*
 TODO LIST:
    - Context variables (those are like global variables) but are inside the registry, malloc/freed inside the registry.
    - Try to make the API simpler  for single/multi views. 
    (de_it_start, de_it_next, de_it_valid
    (de_multi_start, de_multi_next, de_multi_valid,)
    - Callbacks on component insertions/deletions/updates
*/
#include <cassert>
#include <vector>
#include <string>
#include <unordered_map>


namespace ds {

using namespace impl;



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

static ecs_storage* s_ecs_storage_new(size_t cp_size, size_t cp_id) {
    ecs_storage* s = new ecs_storage();
    if (s) {
        s->cp_sizeof = cp_size;
        s->cp_id = cp_id;
    }
    return s;
}

struct de_cp_type {
    size_t cp_id; // component unique id
    size_t cp_sizeof; // component sizeof
};


struct ecs_entity_type {
    std::string name;
    // Holds the component ids hashed using std::hash<std::string>
    std::vector<size_t> cp_ids;
};

struct ecs_system {
    std::string name;
    // Holds the component ids hashed using std::hash<std::string>
    std::vector<size_t> cp_ids;
    // Function that will be executed for that system when running systems.
    ecs_sys_update_fn* update_fn = nullptr;
};

struct ecs_registry {
    /* contains all the created entities */
    std::vector<ecs_entity> entities;

    /* first index in the list to recycle */
    uint32_t available_id = { entity_null };

    /* Hold the component storages */
    std::unordered_map<size_t, ecs_storage> cp_storages;

    /* Hold the registered systems (in order) */
    std::vector<ecs_system> systems;

    /* Hold the registered entity types */
    std::unordered_map<size_t, ecs_entity_type> types;
};


ecs_registry* ecs_registry_create() {
    ecs_registry* r = new ecs_registry();
    return r;
} 

void ecs_registry_destroy(ecs_registry* r) {
    delete r;
}

static inline ecs_entity s_ecs_generate_entity(ecs_registry* r) {
    // can't create more entity identifiers
    assert(r->entities.size() < ECS_ENTITY_ID_MASK);

    // alloc one more element to the entities array
    // create new entity and add it to the array
    const ecs_entity e = ecs_entity_assemble({ (uint32_t) r->entities.size() }, { 0 });
    r->entities.push_back(e);
    return e;
}

/* internal function to recycle a non used entity from the linked list */
static inline ecs_entity s_ecs_recycle_entity(ecs_registry* r) {
    assert(r->available_id != entity_null);
    // get the first available entity id
    const uint32_t curr_id = r->available_id;
    // retrieve the version
    const uint32_t curr_ver = ecs_entity_version(r->entities[curr_id]);
    // point the available_id to the "next" id
    r->available_id = ecs_entity_id(r->entities[curr_id]);
    // now join the id and version to create the new entity
    const ecs_entity recycled_e = ecs_entity_assemble(curr_id, curr_ver);
    // assign it to the entities array
    r->entities[curr_id] = recycled_e;
    return recycled_e;
}

static inline void _de_release_entity(ecs_registry* r, ecs_entity e, uint32_t desired_version) {
    const uint32_t e_id = ecs_entity_id(e);
    r->entities[e_id] = ecs_entity_assemble(r->available_id, desired_version);
    r->available_id = e_id;
}

static inline ecs_entity s_ecs_create_enitty(ecs_registry* r) {
    assert(r);
    if (r->available_id == entity_null) {
        return s_ecs_generate_entity(r);
    } else {
        return s_ecs_recycle_entity(r);
    }
}


/* Returns the storage pointer for the given cp id or nullptr if not exists */
static ecs_storage* s_ecs_get_storage(ecs_registry* r, size_t cp_id) {
    assert(r);
    if (!r->cp_storages.contains(cp_id)) {
        return nullptr;
    }
    return &r->cp_storages[cp_id];
}

static void* de_emplace(ecs_registry* r, ecs_entity e, size_t cp_id) {
    assert(r);
    assert(ecs_entity_valid(r, e));
    auto st = s_ecs_get_storage(r, cp_id);
    assert(st);
    return st->emplace(e);
}
//
//void de_remove_all(ecs_registry* r, ecs_entity e) {
//    assert(r);
//    assert(ecs_entity_valid(r, e));
//    
//    for (size_t i = r->storages.size(); i; --i) {
//        if (r->storages[i - 1] && r->storages[i - 1]->contains(e)) {
//            r->storages[i - 1]->remove(e);
//        }
//    }
//}
//
//void de_remove(ecs_registry* r, ecs_entity e, de_cp_type cp_type) {
//    assert(false);
//    assert(ecs_entity_valid(r, e));
//    de_assure(r, cp_type)->remove(e);
//}
//
//void de_destroy(ecs_registry* r, ecs_entity e) {
//    assert(r);
//    assert(e != entity_null);
//
//    // 1) remove all the components of the entity
//    de_remove_all(r, e);
//
//    // 2) release_entity with a desired new version
//    uint32_t new_version = ecs_entity_version(e);
//    new_version++;
//    _de_release_entity(r, e, new_version);
//}


/* hash a const char * using std::hash<std::string_view> */
size_t impl::hash_cstring(const char* str) {
    return std::hash<std::string_view>()(std::string_view(str));
}

void ecs_entity_register(ecs_registry* r, const char* name, std::vector<const char*> cps) {
    assert(r);
    assert(name);
    const size_t type_id = hash_cstring(name);
    assert(!r->types.contains(type_id)); // you are trying to register an existing entity type
    ecs_entity_type et;
    et.name = name;
    for (size_t i = 0; i < cps.size(); ++i) {
        et.cp_ids.push_back(hash_cstring(cps[i]));
    }
    r->types.insert({ type_id, et});
}

ecs_entity ecs_entity_make(ecs_registry* r, const char* name) {
    assert(r);
    const size_t type_id = hash_cstring(name);
    assert(r->types.contains(type_id));

    // create the entity and emplace all the components
    ecs_entity_type* type = &r->types[type_id];
    ecs_entity e = s_ecs_create_enitty(r);
    for (size_t i = 0; i < type->cp_ids.size(); ++i) {
        const size_t cp_id = type->cp_ids[i];
        auto st = s_ecs_get_storage(r, cp_id);
        assert(st);
        void* cp_data = st->emplace(e);
        // TODO call serialize function for component
    }
    return e;
}

void* ecs_entity_get(ecs_registry* r, ecs_entity e, const char* cp) {
    assert(r);
    assert(ecs_entity_valid(r, e));
    assert(s_ecs_get_storage(r, hash_cstring(cp) ));
    return s_ecs_get_storage(r, hash_cstring(cp))->get(e);
}

void* ecs_entity_try_get(ecs_registry* r, ecs_entity e, const char* cp) {
    assert(r);
    assert(ecs_entity_valid(r, e));
    assert(s_ecs_get_storage(r, hash_cstring(cp)));
    return s_ecs_get_storage(r, hash_cstring(cp))->try_get(e);
}

bool ecs_entity_valid(ecs_registry* r, ecs_entity e) {
    assert(r);
    const uint32_t id = ecs_entity_id(e);
    return (id < r->entities.size()) && (r->entities[id] == e);
}


void ecs_cp_register(ecs_registry* r, const char* name, size_t cp_sizeof,
    ecs_cp_serialize_fn* serialize_fn, ecs_cp_cleanup_fn* cleanup_fn) {
    assert(r);
    assert(name);
    const size_t cp_id = hash_cstring(name);
    assert(!r->cp_storages.contains(cp_id)); // you are trying to register a component with the same name/id
    ecs_storage cp_storage;
    //cp_storage.name = name;
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
        s.cp_ids.push_back(hash_cstring(cps[i]));
    }
    r->systems.push_back(s);
}

ecs_view ecs_view_create_hashed_cps(ecs_registry* r, std::vector<size_t> cp_ids) {
    ecs_view view;
    // TODO BREAKING ERROR. Esta malament la view perke al crear-la no considero que la primera entity estigui en tots els components
    // FIX create an internal function to create a view
    // Retrieve all the system storages for component ids for this system
    // and find the shorter storage (the one with less entities to iterate)
    view._impl.cp_storages.reserve(cp_ids.size());
    for (size_t cp_id_idx = 0; cp_id_idx < cp_ids.size(); ++cp_id_idx) {
        const size_t cp_id = cp_ids[cp_id_idx];
        ecs_storage* cp_storage = s_ecs_get_storage(r, cp_id);
        assert(cp_storage); // no component storage for this cp_id!!!

        // find the shorter storage to iterate it
        if (view._impl.iterating_storage == nullptr) {
            view._impl.iterating_storage = cp_storage;
        }
        else if (cp_storage->dense.size() < view._impl.iterating_storage->dense.size()) {
            view._impl.iterating_storage = cp_storage;
        }
        view._impl.cp_storages.push_back(cp_storage);
    }

    assert(view._impl.iterating_storage);
    view._impl.entity_index = 0;
    if (view._impl.iterating_storage->dense.size() == 0) {
        // mark the view as non valid
        view._impl.entity = entity_null;
    }
    else {
        view._impl.entity = view._impl.iterating_storage->dense[view._impl.entity_index];
    }
    return view;
}

ecs_view ecs_view_create(ecs_registry* r, std::vector<const char*> cp_ids) {
    std::vector<size_t> cps;
    for (auto c : cp_ids) {
        cps.push_back(hash_cstring(c));
    }
    return ecs_view_create_hashed_cps(r, cps);
}

void ecs_run_systems(ecs_registry* r) {
    assert(r);
    for (size_t i = 0; i < r->systems.size(); ++i) {
        ecs_system* sys = &r->systems[i];

        // if no component ids for this system or no update function set skip it
        if ((sys->cp_ids.size() == 0) || (sys->update_fn == nullptr)) {
            continue;
        }

        auto view = ecs_view_create_hashed_cps(r, sys->cp_ids);
        sys->update_fn(r, &view);
    }
}

}
#endif // DESTRAL_ECS_IMPL
#endif // DESTRAL_ECS_H
