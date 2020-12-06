#include "destral_ecs.h"
#include <stdio.h>

///// sample program
typedef struct {
    int x;
    int y;
    int z;
} transform;

struct test_cp1 {
    int w;
};

void each_e(de_registry* r, de_entity e, void* udata) {
    printf("entity %u\n", de_entity_identifier(e).id);
}

int main() {


    de_registry* r = de_new();
    de_register_component(r, 0, sizeof(transform));
    de_register_component(r, 1, sizeof(struct test_cp1));

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

    struct test_cp1* c4 = de_emplace(r, e1, 1);
    c4->w = 69;

    // de_remove_all(r, e1);
    // fast iteration using packed arrays, for the component id 0.
    // FIX (dani) make a good interface to this
    for (size_t i = 0; i < r->storages[0]->cp_data_size; i++) {
        de_entity e = r->storages[0]->sparse.dense[i];
        transform* c = (void*)&((char*)r->storages[0]->cp_data)[r->storages[0]->cp_sizeof * i];
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, c->x, c->y, c->z);
    }

    // fast iteration using packed arrays, for the component id 1.
    for (size_t i = 0; i < r->storages[1]->cp_data_size; i++) {
        de_entity e = r->storages[1]->sparse.dense[i];
        struct test_cp1* c = (void*)&((char*)r->storages[1]->cp_data)[r->storages[1]->cp_sizeof * i];
        printf("test_cp1  entity: %d => w=%d\n", de_entity_identifier(e).id, c->w);
    }

    // slow traversal for each entity in the registry
    de_each(r, each_e, 0);

    de_delete(r);

}