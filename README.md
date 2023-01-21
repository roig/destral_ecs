# BEFORE PROCEEDING: 
# PROJECT NOT MAINTAINED, IT HAS MEMORY BUGS THAT ARE NOT FIXED
# destral_ecs

Is a single-file (destral_ecs.h) C header that implements a very fast ECS registry.

The inner implementation uses an sparse set based on the outstanding ENTT (C++) ECS library.

# Project goals:

* Fast iteration of components (SOA).
* Fast compilation.
* Super easy API to use.

# Basic usage:
Download only the destral_ecs.h header.
Then in ONLY one of your source files, do this:
```cpp
#define DESTRAL_ECS_IMPL
#include "destral_ecs.h"
```
This will "convert" the header file into a header+source file in one Translation Unit, so it will compile all the functions one time.
In other files just use the regular include without the #define. 

You have more libraries like this here:
- https://github.com/RandyGaul/cute_headers
- https://github.com/nothings/stb

# Basic Example:

```c
#define DESTRAL_ECS_IMPL
#include "destral_ecs.h"
#include <stdio.h>

typedef struct transform {
    int x;
    int y;
    int z;
} transform;

typedef struct velocity {
    float v;
} velocity;

// Define your component types directly:
static const de_cp_type transform_type = { .cp_id = 0, .cp_sizeof = sizeof(transform) , .name = "transform"};
// Or with the macro:
static const de_cp_type velocity_type = DE_MAKE_CP_TYPE(1, velocity);

int main() {
    de_ecs* r = de_ecs_make();

    
    de_entity e1 = de_create(r);
    de_entity e2 = de_create(r);
    de_entity e3 = de_create(r);
    de_entity e4 = de_create(r);

    transform* c1 = de_emplace(r, e1, transform_type);  c1->x = 40; c1->y = 50; c1->z = 60;
    velocity* v1 = de_emplace(r, e1, velocity_type);  v1->v = 69;
    transform* c2 = de_emplace(r, e2, transform_type);  c2->x = 4; c2->y = 5; c2->z = 6;
    transform* c3 = de_emplace(r, e3, transform_type);  c3->x = 41; c3->y = 150; c3->z = 160;
    velocity* v4 = de_emplace(r, e4, velocity_type);  v4->v = 12;



    puts("single view transform component");
    for (de_view_single v = de_create_view_single(r, transform_type); de_view_single_valid(&v); de_view_single_next(&v)) {
        de_entity e = de_view_single_entity(&v);
        transform* c = de_view_single_get(&v);
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, c->x, c->y, c->z);
    }

    puts("\nsingle view velocity component");
    for (de_view_single v = de_create_view_single(r, velocity_type); de_view_single_valid(&v); de_view_single_next(&v)) {
        de_entity e = de_view_single_entity(&v);
        velocity* c = de_view_single_get(&v);
        printf("velocity  entity: %d => w=%f\n", de_entity_identifier(e).id, c->v);
    }

    puts("\nmulti view entities with (transform AND velocity) components");
    for (de_view v = de_create_view(r, 2, (de_cp_type[2]) {transform_type, velocity_type }); de_view_valid(&v); de_view_next(&v)) {
        de_entity e = de_view_entity(&v);
        transform* tr = de_view_get(&v, transform_type);
        velocity* tc = de_view_get(&v, velocity_type);
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", de_entity_identifier(e).id, tr->x, tr->y, tr->z);
        printf("velocity  entity: %d => w=%f\n", de_entity_identifier(e).id, tc->v);
    }

    de_ecs_destroy(r);
}
```
