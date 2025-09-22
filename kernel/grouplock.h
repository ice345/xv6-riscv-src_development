#ifndef GROUPLOCK_H
#define GROUPLOCK_H

#include "types.h"
#include "param.h"
#include "spinlock.h"

// Maximum number of group locks
#define MAX_GROUPLOCKS 64

// Z/2Z group element type
typedef enum {
    GROUP_ELEM_0 = 0,    // Unlocked state, identity element
    GROUP_ELEM_1 = 1     // Locked state
} group_element_t;

// Group lock structure
struct grouplock {
    volatile group_element_t state;  // Current group element state
    int group_id;                    // Group lock ID
    int holder_pid;                  // Process ID holding the lock
    char name[16];                   // Lock name
    int ref_count;                   // Reference count
    uint64 acquire_time;             // Lock acquisition timestamp
    struct spinlock debug_lock;      // Lock protecting debug information
};

// Group operation functions
group_element_t group_add(group_element_t a, group_element_t b);
group_element_t group_inverse(group_element_t a);
int group_is_identity(group_element_t a);

// Group lock operation functions
void grouplock_init(void);
int grouplock_create(int group_id, char *name);
int grouplock_acquire(int group_id);
int grouplock_release(int group_id);
int grouplock_destroy(int group_id);
void grouplock_debug_info(int group_id);

// Mathematical verification functions
int verify_group_properties(void);
int verify_deadlock_freedom(void);
int verify_atomic_group_operations(void);

#endif
