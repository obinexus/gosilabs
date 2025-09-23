// phenomenology_platform.h
#ifndef PHENO_PLATFORM_H
#define PHENO_PLATFORM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Bitfield structure for phenomenological relationships
typedef struct {
    // Subject-to-Subject mapping (8 bits each)
    unsigned int subject_id     : 8;
    unsigned int subject_type   : 8;
    unsigned int subject_state  : 8;
    unsigned int subject_class  : 8;
    
    // Class-to-Class mapping (8 bits each)
    unsigned int class_id       : 8;
    unsigned int class_category : 8;
    unsigned int class_taxonomy : 8;
    unsigned int class_level    : 8;
    
    // Instance-to-Instance mapping (8 bits each)
    unsigned int instance_id    : 8;
    unsigned int instance_type  : 8;
    unsigned int instance_state : 8;
    unsigned int instance_flags : 8;
    
    // Person-to-Person model (8 bits each)
    unsigned int person_id      : 8;
    unsigned int person_role    : 8;
    unsigned int person_auth    : 8;
    unsigned int person_state   : 8;
} PhenoRelation;

// Bit manipulation macros
#define BIT_SET(value, bit)     ((value) |= (1U << (bit)))
#define BIT_CLEAR(value, bit)   ((value) &= ~(1U << (bit)))
#define BIT_TOGGLE(value, bit)  ((value) ^= (1U << (bit)))
#define BIT_CHECK(value, bit)   (((value) >> (bit)) & 1U)

// Rotate operations for phenomenological cycling
#define ROTATE_LEFT(n, b)  (((n) << (b)) | ((n) >> (8 - (b))))
#define ROTATE_RIGHT(n, b) (((n) >> (b)) | ((n) << (8 - (b))))

// Size-aware shift (>>>0 equivalent)
#define UNSIGNED_SHIFT(val) ((size_t)(val) >> 0)
