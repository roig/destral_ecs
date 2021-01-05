#include "destral_ecs.hpp"
#include <cstdio>
using namespace ds;
struct transform {
    int x;
    int y;
    int z;
};

struct hero_data {
    int life_points = 10;
};

void hero_system_fn(ecs_registry* r, ecs_cp_view* v) {
    // retrieve component view index (here will assert if cp is not in the view)
    const size_t tr_idx = ecs_view_cp_index(v, "transform");
    const size_t hd_idx = ecs_view_cp_index(v, "hero_data");
    while (ecs_view_valid(v)) {
        de_entity e = ecs_view_entity(v);
        transform* c = (transform*)ecs_view_cp(v, tr_idx);
        hero_data* chd = (hero_data*)ecs_view_cp(v, hd_idx);
        // do whatever you want.. here..
        c->x = 40; c->y = 50; c->z = 60;
        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", e, c->x, c->y, c->z); fflush(stdout);
        ecs_view_next(v);
    }
}

int main() {
    ecs_registry* r = ecs_registry_create();
    // register the components
    ecs_cp_register(r, "transform", sizeof(transform));
    ecs_cp_register(r, "hero_data", sizeof(hero_data));

    // register the entities 
    ecs_entity_register(r, "hero", {"transform", "hero_data"});

    // register the systems
    ecs_sys_add(r, "transform_system", {"transform", "hero_data" }, &hero_system_fn);

    // create three hero entities
    de_entity hero1 = ecs_entity_make(r, "hero");
    de_entity hero2 = ecs_entity_make(r, "hero");
    de_entity hero3 = ecs_entity_make(r, "hero");

    // run the application (systems)
    while (true) {
        ecs_run_systems(r);
    }
    ecs_registry_destroy(r);
}
















//void hero_system_fn(ecs_registry* r, ecs_cp_view* v) {
//    transform** tr_ptr = (transform**)ecs_view_cp(v, "transform");
//    de_entity e = ecs_view_entity(v);
//    while (ecs_view_valid(v)) {
//        transform* c = *tr_ptr;
//        c->x = 40; c->y = 50; c->z = 60;
//        printf("transform  entity: %d => x=%d, y=%d, z=%d\n", *e, c->x, c->y, c->z); fflush(stdout);
//        ecs_view_next(v);
//    }
//}
