#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

/*
    destral_ecs.c --


*/




/*  de_cp_id:
    
    type identifier for a component.

    component storages are preallocated in a array size of DE_COMPONENT_MAX_ID.
    this allows fast lookup for storage but will occupy more memory.

    you will normally not need more than UINT16_MAX components..
    If you need more, beware that changing to uint32_t the preallocation 
    cost will be huge...
*/

/* typedef for the components id */
typedef uint16_t de_cp_id;

/* max number of components id allowed */
#define DE_COMPONENT_MAX_ID (UINT16_MAX)


/*  de_entity:
    
    opaque 32 bits entity identifier.

    A 32 bits entity identifier guarantees:

    20 bits for the entity number(suitable for almost all the games).
    12 bit for the version(resets in[0 - 4095]).

    use the functions de_entity_version and de_entity_identifier to retrieve
    each part of a de_entity.
*/

/* typedef for the entity type */
typedef uint32_t de_entity;
typedef struct de_entity_ver { uint32_t ver; } de_entity_ver;
typedef struct de_entity_id { uint32_t id; } de_entity_id;

/* masks for retrieving the id and the version of an entity */
#define DE_ENTITY_ID_MASK       ((uint32_t)0xFFFFF) /* Mask to use to get the entity number out of an identifier.*/  
#define DE_ENTITY_VERSION_MASK  ((uint32_t)0xFFF)   /* Mask to use to get the version out of an identifier. */
#define DE_ENTITY_SHIFT         ((size_t)20)        /* Extent of the entity number within an identifier. */   

/* Returns the version part of the entity */
de_entity_ver de_entity_version(de_entity e) { return (de_entity_ver) { .ver = e >> DE_ENTITY_SHIFT }; }

/* Returns the id part of the entity */
de_entity_id de_entity_identifier(de_entity e) { return (de_entity_id) { .id = e & DE_ENTITY_ID_MASK }; }

/* Makes a de_entity from entity_id and entity_version */
de_entity de_make_entity(de_entity_id id, de_entity_ver version) { return id.id | (version.ver << DE_ENTITY_SHIFT); }


/*
    the de_null is a de_entity that represents a null entity.
*/
// FIX (dani) Only This line will be in the header.
extern const de_entity de_null;
const de_entity de_null = (de_entity)DE_ENTITY_ID_MASK;




/*
    de_sparse:

    How the components sparse set works?
    The main idea comes from ENTT C++ library.
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

    this allows fast itreration on each entity using the dense array or
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
*/
typedef struct de_sparse {
    /*  sparse entity identifiers indices array.
        - index is the de_entity_id. (without version)
        - value is the index of the dense array
    */
    de_entity* sparse;
    size_t sparse_size;

    /*  Dense entities array.
        - index is linked with the sparse value.
        - value is the full de_entity
    */
    de_entity* dense;
    size_t dense_size;
} de_sparse;

/* de_sparse constructor */
de_sparse* de_sparse_init(de_sparse* s) {
    if (s) {
        *s = (de_sparse) {0};
        s->sparse = 0;
        s->dense = 0;
    }
    return s;
}

de_sparse* de_sparse_new() {
    return de_sparse_init(malloc(sizeof(de_sparse)));
}

/* de_sparse destructor */
void de_sparse_destroy(de_sparse* s) {
    if (s) {
        free(s->sparse);
        free(s->dense);
    }
}

void de_sparse_delete(de_sparse* s) {
    de_sparse_destroy(s);
    free(s);
}

/*
    returns true if the sparse_set contains the entity e 
    
    warning: passing entity that doesn't belong to the sparse set
    or the null entity results in undefined behavior.
*/
bool de_sparse_contains(de_sparse* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    const de_entity_id eid = de_entity_identifier(e);
    return (eid.id < s->sparse_size) && (s->sparse[eid.id] != de_null);
}

/*
    returns the index position of an entity in the sparse array to be used
    as the index in the dense array.
    
    warning: attempting to get the index for an entity that doesn't belong
    to the sparse setresults in undefined behavior.
*/
size_t de_sparse_index(de_sparse* s, de_entity e) {
    assert(s);
    assert(de_sparse_contains(s, e));
    return s->sparse[e];
}

/*
    assigns an entity to the sparse set.

    warning: attempting to assign an entity that already belongs to the sparse set
    or the de_null entity results in undefined behavior.
*/
void de_sparse_emplace(de_sparse* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    const de_entity_id eid = de_entity_identifier(e);
    if (eid.id >= s->sparse_size) { // check if we need to realloc
        const size_t new_sparse_size = eid.id + 1;
        s->sparse = realloc(s->sparse, new_sparse_size * sizeof *s->sparse);
        memset(s->sparse + s->sparse_size, de_null, (new_sparse_size - s->sparse_size) * sizeof * s->sparse);
        s->sparse_size = new_sparse_size;
    }
    s->sparse[eid.id] =(de_entity)s->dense_size; // set this eid index to the last dense index (dense_size)
    s->dense = realloc(s->dense, (s->dense_size + 1) * sizeof *s->dense);
    s->dense[s->dense_size] = e;
    s->dense_size++;
}

/*
    Removes an entity from a sparse set.
    
    returns the dense index where the entity was occupying before removing.
    warning: Attempting to remove an entity that doesn't belong to the sparse set
    results in undefined behavior.<br/>

*/
size_t de_sparse_remove(de_sparse* s, de_entity e) {
    assert(s);
    assert(de_sparse_contains(s, e));

    const size_t pos = s->sparse[de_entity_identifier(e).id];
    const de_entity other = s->dense[s->dense_size - 1];

    s->sparse[de_entity_identifier(other).id] = (de_entity)pos;
    s->dense[pos] = other;
    s->sparse[pos] = de_null;

    s->dense = realloc(s->dense, (s->dense_size - 1) * sizeof * s->dense);
    s->dense_size--;

    return pos;
}


/*
    de_storage

    handles the raw component data aligned with a de_sparse.
    stores packed component data elements for each entity in the sparse set.

    the packed component elements data is aligned always with the dense array from the sparse set.

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
    void* cp_data; /*  packed component elements array. aligned with sparse->dense*/
    size_t cp_data_size; /* number of elements in the cp_data array */
    size_t cp_sizeof; /* sizeof for each cp_data element */
    de_sparse sparse;
} de_storage;

/* initializes a de_storage, cp_size is the component data storage sizeof */
de_storage* de_storage_init(de_storage* s, size_t cp_size) {
    if (s) {
        *s = (de_storage){ 0 };
        de_sparse_init(&s->sparse);
        s->cp_sizeof = cp_size;
    }
    return s;
}

/* allocate and initialize a de_storage with component data size cp_size*/
de_storage* de_storage_new(size_t cp_size) {
    return de_storage_init(malloc(sizeof(de_storage)), cp_size);
}

/* deinitializes a de_storage  */
void de_storage_destroy(de_storage* s) {
    if (s) {
        de_sparse_destroy(&s->sparse);
        free(s->cp_data);
    }
}

/* deinitializes and destroy a de_storage */
void de_storage_delete(de_storage* s) {
    de_storage_destroy(s);
    free(s);
}

/*
    assigns an entity to a storage and allocates the memory for its component 
    struct data and returns the pointer to the allocated pointer. the memory
    returned is ONLY allocated NOT initialized.

    warning: attempting to use a de_entity that already belongs to the storage
    or the de_null entity results in undefined behavior.
*/
void* de_storage_emplace(de_storage* s, de_entity e) {
    assert(s);
    // now allocate the data for the new component at the end of the array
    s->cp_data = realloc(s->cp_data, (s->cp_data_size + 1) * sizeof(char) * s->cp_sizeof);
    s->cp_data_size++;

    // return the component data pointer (last position)
    void* cp_data_ptr = &((char*)s->cp_data)[(s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof];
    
    // then add the entity to the sparse set
    de_sparse_emplace(&s->sparse, e);

    return cp_data_ptr;
}

/*
    removes an entity from a storage and deallocates the memory. The memory is ONLY
    deallocated NOT deinitialized.

    warning: attempting to use a de_entity that doesn't belongs to the storage
    or the de_null entity results in undefined behavior.
*/
void de_storage_remove(de_storage* s, de_entity e) {
    assert(s);
    size_t pos_to_remove = de_sparse_remove(&s->sparse, e);

    // swap (memmove because if cp_data_size 1 it will overlap dst and source.
    memmove(
        &((char*)s->cp_data)[pos_to_remove * sizeof(char) * s->cp_sizeof],
        &((char*)s->cp_data)[(s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof],
        s->cp_sizeof);

    // and pop
    s->cp_data = realloc(s->cp_data, (s->cp_data_size - 1) * sizeof(char) * s->cp_sizeof);
    s->cp_data_size--;
}

/*
    returns the data pointer associated with an entity

    warning: attempting to use an entity that doesn't belongs to the storage
    or the de_null entity results in undefined behavior.
*/
void* de_storage_get(de_storage* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    return &((char*)s->cp_data)[de_sparse_index(&s->sparse, e) * sizeof(char) * s->cp_sizeof ];
}

/*
    returns the data pointer associated with an entity, if any.
    returns 0 if entity doesn't belongs to the storage.
*/
void* de_storage_try_get(de_storage* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    return de_sparse_contains(&s->sparse, e) ? de_storage_get(s, e) : 0;
}


/*
    returns true if the storage contains the entity e

    warning: passing entity that doesn't belong to the storage
    or the null entity results in undefined behavior.
*/
bool de_storage_contains(de_storage* s, de_entity e) {
    assert(s);
    assert(e != de_null);
    return de_sparse_contains(&s->sparse, e);
}

/*  de_registry
    
    is the global context that holds each components and entities.

    FIX (dani) todo list:
    - entity creation/destruction reciclying ids (DONE)
    - hability to create/destroy sparse sets of each component type. (DONE)
    - add/remove components to entities (DONE)
    - iteration of components/entites in a view

*/
typedef struct de_registry {
    de_storage** storages; /* DE_COMPONENT_MAX_ID array preallocated in the begining */
    size_t entities_size; 
    de_entity* entities; /* contains all the created entities */
    de_entity_id available_id; /* first index in the list to recycle */
} de_registry;

/* initializes a de_registry */
de_registry* de_init(de_registry* r) {
    if (r) {
        r->available_id.id = de_null;
        r->entities_size = 0;
        r->entities = NULL;

        // initialize and null the components storage array
        r->storages = calloc(DE_COMPONENT_MAX_ID, sizeof * r->storages);
    }
    return r;
}

/* allocates and initializes a de_registry */
de_registry* de_new() {
    return de_init(malloc(sizeof(de_registry)));
}

/* deinitializes a de_registry */
void de_deinit(de_registry* r) {
    if (r) {
        if (r->storages) {
            for (size_t i = 0; i < DE_COMPONENT_MAX_ID; i++) {
                de_storage_delete(r->storages[i]);
            }
        }
        free(r->entities);
    }
}

/* deinitializes and deletes a registry*/
void de_delete(de_registry* r) {
    de_deinit(r);
    free(r);
}

/* 
    returns true if the component is registered correctly.
    returns false if the component can't be registered (another one exists)

*/
bool de_register_component(de_registry* r, uint16_t cp_id, size_t cp_size) {
    assert(r);
    assert(cp_size); // FIX (dani) add support for 0 sized components
    if (r->storages[cp_id] == 0) {
        r->storages[cp_id] = de_storage_new(cp_size);
        return true;
    }
    return false;
}

/*
    checks if an entity identifier is a valid one.
    the entity e can be a valid or an invalid one.
    returns true if the identifier is valid, false otherwise
*/
bool de_valid(de_registry* r, de_entity e) {
    assert(r);
    const de_entity_id id = de_entity_identifier(e);
    return id.id < r->entities_size&& r->entities[id.id] == e;
}


/* internal function to generate a new de_entity */
de_entity _de_generate_entity(de_registry* r) {
    assert(r->entities_size < DE_ENTITY_ID_MASK);

    // alloc one more element to the entities array
    r->entities = realloc(r->entities, (r->entities_size + 1) * sizeof(de_entity));

    // create new entity and add it to the array
    const de_entity e = de_make_entity((de_entity_id) {(uint32_t)r->entities_size}, (de_entity_ver) { 0 });
    r->entities[r->entities_size] = e;
    r->entities_size++;
    
    return e;
}

/* internal function to recycle a non used entity from the linked list */
de_entity _de_recycle_entity(de_registry* r) {
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

/* internal function add the entity to the recycle linked list */
void _de_release_entity(de_registry* r, de_entity e, de_entity_ver desired_version) {
    const de_entity_id e_id = de_entity_identifier(e);
    r->entities[e_id.id] = de_make_entity(r->available_id, desired_version);
    r->available_id = e_id;
}

/* 
    creates a new entity and returns it 
    the identifier can be:
     - new identifier in case no entities have been previously destroyed
     - recycled identifier with an update version.
*/
de_entity de_create(de_registry* r) {
    assert(r);
    if (r->available_id.id == de_null) {
        return _de_generate_entity(r);
    } else {
        return _de_recycle_entity(r);
    }
}





/*
    returns the de_storage pointer for a given component id.
    if the component id is not registered it will return a null pointer.

*/
de_storage* de_assure(de_registry* r, de_cp_id cp_id) {
    assert(r);
    return r->storages[cp_id] ? r->storages[cp_id] : 0;
}

/*
    removes all the components from an entity and makes it orphaned (no components in it)
    the entity remains alive and valid without components.

    warning: attempting to use invalid entity results in undefined behavior

*/
void de_remove_all(de_registry* r, de_entity e) {
    assert(r);
    assert(de_valid(r, e));

    // FIX (dani) super bad.. we have to traverse the entire pools array to delete the entity
    // from all of them if is contained.. very bad performance..
    for (size_t i = DE_COMPONENT_MAX_ID; i; --i) {
        if (r->storages[i - 1] && de_sparse_contains(&r->storages[i - 1]->sparse, e)) {
            de_storage_remove(r->storages[i - 1], e);
        }
    }
}

/*
    removes the given component from the entity.
    
    warning: attempting to use invalid entity results in undefined behavior
*/
void de_remove(de_registry* r, de_entity e, de_cp_id cp_id) {
    assert(false);
    assert(de_valid(r, e));
    de_storage_remove(de_assure(r, cp_id), e);
}


/*
    destroys an entity.

    when an entity is destroyed, its version is updated and the identifier
    can be recycled when needed.

    warning:
    Undefined behavior if the entity is not valid.
*/
void de_destroy(de_registry* r, de_entity e) {
    assert(r);
    assert(e != de_null);

    // 1) remove all the components of the entity
    de_remove_all(r, e);

    // 2) release_entity with a desired new version
    de_entity_ver new_version = de_entity_version(e);
    new_version.ver++;
    _de_release_entity(r, e, new_version);
}

/*
    checks if the entity has the given component

    warning: using an invalid entity results in undefined behavior.
*/
bool de_has(de_registry* r, de_entity e, de_cp_id cp_id) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_id));
    return de_storage_contains(de_assure(r, cp_id), e);
}


/*
    assigns a given component id to an entity and returns it.

    a new memory instance of component id cp_id is allocated for the
    entity e and returned. 

    note: the memory returned is only allocated not initialized.

    warning: use an invalid entity or assigning a component to a valid
    entity that currently has this component instance id results in 
    undefined behavior.
    if cp_id is not registered the result is undefined behavior.
*/
void* de_emplace(de_registry* r, de_entity e, de_cp_id cp_id) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_id));
    return de_storage_emplace(de_assure(r, cp_id), e);
}

/*
    returns the pointer to the given component id data for the entity

    warning:
    use an invalid entity to get a component from an entity
    that doesn't own it results in undefined behavior.

    if cp_id is not registered the result is undefined behavior.
*/
void* de_get(de_registry* r, de_entity e, de_cp_id cp_id) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_id));
    return de_storage_get(de_assure(r, cp_id), e);
}

/*
    returns the pointer to the given component id data for the entity

    warning:
    use an invalid entity to get a component from an entity
    that doesn't own it results in undefined behavior.

    if cp_id is not registered the result is undefined behavior.
*/
void* de_try_get(de_registry* r, de_entity e, de_cp_id cp_id) {
    assert(r);
    assert(de_valid(r, e));
    assert(de_assure(r, cp_id));
    return de_storage_try_get(de_assure(r, cp_id), e);
}

/*
    iterates all the entities that are still in use.

    the function pointer is invoked for each entity that is still in use.
    
    this is a fairly slow operation and should not be used frequently.
    however it's useful for iterating all the entities still in use,
    regarding their components.

    warning: the function is not optional, undefined behavior if fun is null.
*/
void de_each(de_registry* r, void (*fun)(de_registry*, de_entity, void*), void* udata) {
    assert(r);
    assert(fun);

    if (r->available_id.id == de_null) {
        for (size_t i = r->entities_size; i; --i) {
            fun(r, r->entities[i - 1], udata);
        }
    } else {
        for (size_t i = r->entities_size; i; --i) {
            const de_entity e = r->entities[i - 1];
            if (de_entity_identifier(e).id == (i - 1)) {
                fun(r, e, udata);
            }
        }
    }
}

///////// VIEWS
/* 
 we can acces each component of an entity using de_get or try_get but this is not cache friendly.
 so we need a way to iterate (view) all the components of the same type in a cache friendly way.

 FIX (dani) think of an structure to iterate the storages ENTT uses the view class
 investigate a little bit the main idea should be this:

    for the component id = 0

    const size_t entities_size = r->storages[0].sparse.dense_size;
    
    de_entity*  entities_array = r->storages[0].sparse.dense;
    void*       components_array = r->storages[0]->cp_data;


    for (size_t i = 0; i < entities_size; i++) {
        de_entity e = entities_array[i];
        void*     c = components_array[i]; // this will need some pointer gymnastics ofc..
        
        .... your code...
    }
*/



///// sample program
typedef struct {
    int x;
    int y;
    int z;
} transform;

typedef struct {
    int w;
} test_cp1;


void each_e(de_registry* r, de_entity e, void* udata) {
    printf("entity %u\n", de_entity_identifier(e).id);
}





int main() {

    de_registry *r = de_new();
    de_register_component(r, 0, sizeof(transform));
    de_register_component(r, 1, sizeof(test_cp1));

    de_entity e1 = de_create(r);
    de_destroy(r, e1);
    e1 = de_create(r);
    de_entity e2 = de_create(r);
    de_entity e3 = de_create(r);

    transform* c1 = de_emplace(r, e1, 0);
    c1->x = 1; c1->y = 2; c1->z = 3;
    transform* c3 = de_emplace(r, e3, 0);
    c3->x = 7; c3->y = 8; c3->z = 9;
    transform* c2 = de_emplace(r, e2, 0);
    c2->x = 4; c2->y = 5; c2->z = 6;

    test_cp1* c4 = de_emplace(r, e1, 1);
    c4->w = 69;

    //de_remove_all(r, e1);
    // fast iteration using packed arrays, for the component id 0.
    // FIX (dani) make a good interface to this
    for (size_t i = 0; i < r->storages[0]->cp_data_size; i++) {
        de_entity e = r->storages[0]->sparse.dense[i];
        transform* c = (transform*) &((char*)r->storages[0]->cp_data)[r->storages[0]->cp_sizeof * i];
        printf("test_cp  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, c->x, c->y, c->z);
    }

    // fast iteration using packed arrays, for the component id 1.
    for (size_t i = 0; i < r->storages[1]->cp_data_size; i++) {
        de_entity e = r->storages[1]->sparse.dense[i];
        test_cp1* c = (test_cp1*)&((char*)r->storages[1]->cp_data)[r->storages[1]->cp_sizeof * i];
        printf("test_cp1  entity: %d => w=%d\n", de_entity_identifier(e).id, c->w);
    }

    // slow traversal for each entity in the registry
    de_each(r, each_e, 0);


    de_delete(r);
	return EXIT_SUCCESS;
}

//// encapsulate type information needed for the registry
//typedef struct de_component_type_info {
//    size_t cp_id; // component unique id
//    size_t cp_sizeof; // component sizeof
//    // maybe here we can add function callbacks for initialization and deinitialization of the component
//}de_cp_type;
//
//typedef struct position { int x; int y; } position;
//typedef struct rotation { float rotation; } rotation;
//
//// here you define each component you will use
//de_cp_type cp_position = { .cp_id = 0, .cp_sizeof = sizeof(position) };
//de_cp_type cp_rotation = { .cp_id = 1, .cp_sizeof = sizeof(rotation) };
//
//
//// and use it like this (no registration needed)
//de_registry* r = de_new();
//de_entity e = de_create(r);
//position* tr = de_emplace(r, e, cp_position);
//rotation* rot = de_emplace(r, e, cp_rotation);