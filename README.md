# Destral_ecs
ENTT like implementation of ECS in C


# Basic usage:
In ONE of your source files, do this:
```cpp
#define DESTRAL_ECS_IMPL
#include "destral_ecs.h"
```
and that's it! use the header in other files as a normal header


# Example:

```cpp
typedef struct transform {
    int x;
    int y;
    int z;
} transform;

typedef struct velocity {
    float v;
} velocity;


int main() {
    de_registry* r = de_new();

    de_register_component(r, 0, sizeof(transform));
    de_register_component(r, 1, sizeof(velocity));

    de_entity e1 = de_create(r);
    de_entity e2 = de_create(r);
    de_entity e3 = de_create(r);

    transform* c1 = de_emplace(r, e1, 0);  c1->x = 40; c1->y = 50; c1->z = 60;
    velocity* c4 = de_emplace(r, e1, 1);  c4->v = 69;
    transform* c2 = de_emplace(r, e2, 0);  c2->x = 4; c2->y = 5; c2->z = 6;
    transform* c3 = de_emplace(r, e3, 0);  c3->x = 41; c3->y = 150; c3->z = 160;

    puts("single view transform component");
    for (de_view_single v = de_create_view_single(r, 0); de_view_single_valid(&v); de_view_single_next(&v)) {
        de_entity e = de_view_single_entity(&v);
        transform* c = de_view_single_get(&v);
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, c->x, c->y, c->z);
    }

    puts("\nsingle view velocity component");
    for (de_view_single v = de_create_view_single(r, 1); de_view_single_valid(&v); de_view_single_next(&v)) {
        de_entity e = de_view_single_entity(&v);
        velocity* c = de_view_single_get(&v);
        printf("velocity  entity: %d => w=%f\n", de_entity_identifier(e).id, c->v);
    }


    puts("\nmulti view entities with (transform AND velocity) components");
    for (de_view v = de_create_view(r, 2, (de_cp_id[2]) { 0, 1 }); de_view_valid(&v); de_view_next(&v)) {
        de_entity e = de_view_entity(&v);
        transform* tr = de_view_get(&v, 0);
        velocity* tc = de_view_get(&v, 1);
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, tr->x, tr->y, tr->z);
        printf("velocity  entity: %d => w=%f\n", de_entity_identifier(e).id, tc->v);
    }
 
    de_delete(r);
}

```
