#include "destral_ecs.h"
#include <stdio.h>

typedef struct transform {
    int x;
    int y;
    int z;
} transform;

typedef struct test_cp1 {
    int w;
} test_cp1;

void each_e(de_registry* r, de_entity e, void* udata) {
    printf("entity %u\n", de_entity_identifier(e).id);
}

int main() {
    de_registry* r = de_new();
    // FIX (dani) I don't like the registering of components.. think another way...
    de_register_component(r, 0, sizeof(transform));
    de_register_component(r, 1, sizeof(test_cp1));

    de_entity e1 = de_create(r);
    de_destroy(r, e1);
    e1 = de_create(r);
    de_entity e2 = de_create(r);
    de_entity e3 = de_create(r);

    transform* c1 = de_emplace(r, e1, 0);  c1->x = 1; c1->y = 2; c1->z = 3;
    transform* c3 = de_emplace(r, e3, 0);  c3->x = 7; c3->y = 8; c3->z = 9;
    transform* c2 = de_emplace(r, e2, 0);  c2->x = 4; c2->y = 5; c2->z = 6;
    
    test_cp1* c4 = de_emplace(r, e1, 1);  c4->w = 69;

    puts("single view transform component");
    for (de_view_single v = de_create_view_single(r, 0); de_view_single_valid(&v); de_view_single_next(&v)) {
        de_entity e = de_view_single_entity(&v);
        transform* c = de_view_single_get(&v);
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, c->x, c->y, c->z);
    }

    puts("\nsingle view test_cp1 component");
    for (de_view_single v = de_create_view_single(r, 1); de_view_single_valid(&v); de_view_single_next(&v)) {
        de_entity e = de_view_single_entity(&v);
        test_cp1* c = de_view_single_get(&v);
        printf("test_cp1  entity: %d => w=%d\n", de_entity_identifier(e).id, c->w);
    }


    puts("\nmulti view entities with (transform AND test_cp1) components");
    for (de_view v = de_create_view(r, 2, (de_cp_id[2]){ 0, 1 }); de_view_valid(&v); de_view_next(&v)) {
        // de_entity e = de_view_single_entity(&v);
        de_entity e = de_view_entity(&v);
        transform* tr = de_view_get(&v, 0);
        test_cp1* tc = de_view_get(&v, 1);
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, tr->x, tr->y, tr->z);
        printf("test_cp1  entity: %d => w=%d\n", de_entity_identifier(e).id, tc->w);
    }

    puts("\nslow each entity traversal");
    de_each(r, each_e, 0);

    de_delete(r);
}




//// de_remove_all(r, e1);
//// fast iteration using packed arrays, for the component id 0.
//// FIX (dani) make a good interface to this
//for (size_t i = 0; i < r->storages[0]->cp_data_size; i++) {
//    //de_entity e = r->storages[0]->sparse.dense[i];
//    transform* c = (void*)&((char*)r->storages[0]->cp_data)[r->storages[0]->cp_sizeof * i];
//   // printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, c->x, c->y, c->z);
//    printf("transform  entity:  => x=%d, y=%d, z=%d\n",  c->x, c->y, c->z);
//}

//// fast iteration using packed arrays, for the component id 1.
//for (size_t i = 0; i < r->storages[1]->cp_data_size; i++) {
//    de_entity e = r->storages[1]->sparse.dense[i];
//    struct test_cp1* c = (void*)&((char*)r->storages[1]->cp_data)[r->storages[1]->cp_sizeof * i];
//    printf("test_cp1  entity: %d => w=%d\n", de_entity_identifier(e).id, c->w);
//}
