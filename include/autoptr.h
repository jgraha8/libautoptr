/*
 * Copyright (c) 2017-2019 Jason Graham <jgraham@compukix.net>
 *
 * This file is part of libautoptr.
 *
 * libautoptr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libautoptr is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libautoptr.  If not, see
 * <https://www.gnu.org/licenses/>.
 */
/**
 * @file
 * @brief Autoptr Module Definitions
 *
 * Provides memory management support and lifetime management of shared objects.
 *
 * @author Jason Graham <jgraham@compukix.net>
 */

#ifndef __AUTOPTR_H__
#define __AUTOPTR_H__

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUTOPTR_MAGIC 0x643B2B47 ///< cksum (32-bit) CRC of "AUTOPTR_MAGIC"
#define AUTOPTR(a)                                                                                                \
        ((struct autoptr *)(a)) ///< Macro used for casting the object pointer to @c autoptr (internal usage)
#define AUTOPTR_LOCK(a) (pthread_mutex_lock(&AUTOPTR(a)->mutex))
#define AUTOPTR_UNLOCK(a) (pthread_mutex_unlock(&AUTOPTR(a)->mutex))

#define AUTOPTR_M(a) (AUTOPTR(a)->manager)
#define AUTOPTR_M_LOCK(a) (pthread_mutex_lock(&AUTOPTR_M(a)->mutex))
#define AUTOPTR_M_UNLOCK(a) (pthread_mutex_unlock(&AUTOPTR_M(a)->mutex))

#ifdef AUTOPTR_ASSERT
#define autoptr_assert(ptr)                                                                                       \
        {                                                                                                         \
                AUTOPTR_LOCK(ptr);                                                                                \
                assert(ptr);                                                                                      \
                const unsigned int __magic = ((struct autoptr *)(ptr))->__magic;                                  \
                assert(__magic == AUTOPTR_MAGIC);                                                                 \
                if (__magic != AUTOPTR_MAGIC) {                                                                   \
                        fprintf(stderr, "autoptr_assert_magic: magic = 0x%8x != 0x%8x\n", __magic,                \
                                AUTOPTR_MAGIC);                                                                   \
                        exit(EXIT_FAILURE);                                                                       \
                }                                                                                                 \
                AUTOPTR_UNLOCK(ptr);                                                                              \
        }

#else
#define autoptr_assert(ptr)
#endif

/**
 * @brief Autoptr memory management data structure.
 *
 * Tracks bound references and stores the memory state for the memory management
 * support and lifetime management of shared objects. It uses the usual
 * reference counting and includes a heap-allocation flag and the object
 * destruction/free function pointers. Provides automatic object cleanup of
 * shared object when the final bound reference is unbound.
 *
 * For more information see the documentation in @c autoptr.h.
 *
 */
struct autoptr {
        unsigned int __magic; ///< Magic number
        pthread_mutex_t mutex;
        bool allocd;              ///< Allocation flag; indicates if an object is heap-allocated
        int r_count;              ///< Reference count
        size_t obj_len;           ///< Length in bytes of managed object
        void (*obj_dtor)(void *); ///< Destructor for the managed object
        struct autoptr *manager;  ///< Manager object of a managed contiguous set (e.g. 1st in vector of objects)
        size_t num_managed;       ///< Number of objects of a managed contiguous set
};

/**
 * @brief Constructor for the memory management data structure
 *
 * @param ptr Address of memory managed object
 * @param obj_len Length in bytes of the managed object
 * @param obj_dtor Destructor for the managed object
 * @param obj_free Free procedure for the managed object
 * @param obj_vfree Vector free procedure for the managed object
 */
void autoptr_ctor(void *ptr, size_t obj_len, void (*obj_dtor)(void *));

/**
 * @brief Destructor for the memory management data structure
 *
 * @param ptr Address of memory managed object
 */
void autoptr_dtor(void *ptr);

/**
 * @brief Sets the object length and destructor function.
 *
 * Typically used to reasign the object length and destructor for a
 * derived data-structure.
 *
 * @param ptr Address of memory managed object
 * @param obj_dtor Destructor for the managed object
 * @param obj_free Free procedure for the managed object
 */
void autoptr_set_obj(void *ptr, size_t obj_len, void (*obj_dtor)(void *));

/**
 * @brief Sets the manager for a contigous allocation of managed objects
 *
 * @param ptr Address of memory managed object
 * @param num_managed Number of managed objects (i.e., number of objects in the contiguous allocation)
 *
 * @note This procedures is not thread safe.
 */
static inline void autoptr_set_managed(void *ptr, size_t num_managed)
{
        autoptr_assert(ptr);
        // Assert first object is self-managed
        assert(AUTOPTR(ptr)->manager == AUTOPTR(ptr));

        // Set the number of managed objects for the manager
        AUTOPTR(ptr)->num_managed = num_managed; // Includes self

        for (size_t i = 1; i < num_managed; ++i) {
                void *obj = (void *)((char *)ptr + AUTOPTR(ptr)->obj_len * i);
                autoptr_assert(obj);

                AUTOPTR(obj)->manager     = AUTOPTR(ptr);
                AUTOPTR(obj)->num_managed = 0;
        }
}

static inline size_t autoptr_num_managed(void *ptr)
{
        autoptr_assert(ptr);

        AUTOPTR_M_LOCK(ptr);
        size_t num_managed = AUTOPTR_M(ptr)->num_managed;
        AUTOPTR_M_UNLOCK(ptr);

        return num_managed;
}

/**
 * @brief Tests if the object may be destroyed (i.e. the reference count is zero)
 *
 * @param ptr Address of memory managed object
 */
static inline bool autoptr_destroy(void *ptr)
{
        autoptr_assert(ptr);

        AUTOPTR_M_LOCK(ptr);
        assert(AUTOPTR_M(ptr)->r_count >= 0);
        bool destroy = (AUTOPTR_M(ptr)->r_count == 0);
        AUTOPTR_M_UNLOCK(ptr);
        return destroy;
}

/**
 * @brief Gets the heap allocation flag
 *
 * @param ptr Address of memory managed object
 */
static inline bool autoptr_get_allocd(void *ptr)
{
        autoptr_assert(ptr);

        AUTOPTR_M_LOCK(ptr);
        bool allocd = AUTOPTR_M(ptr)->allocd;
        AUTOPTR_M_UNLOCK(ptr);
        return allocd;
}

/**
 * @brief Sets the heap allocation flag
 *
 * @param ptr Address of memory managed object
 */
static inline void autoptr_set_allocd(void *ptr, bool allocd)
{
        autoptr_assert(ptr);
        AUTOPTR_M_LOCK(ptr);
        AUTOPTR_M(ptr)->allocd = allocd;
        AUTOPTR_M_UNLOCK(ptr);
}

/**
 * @brief Retains ownership of the object
 *
 * @param ptr Address of memory managed object
 */
static inline void autoptr_retain(void *ptr)
{
        autoptr_assert(ptr);

        AUTOPTR_M_LOCK(ptr);
        AUTOPTR_M(ptr)->r_count++;
        AUTOPTR_M_UNLOCK(ptr);
}

/**
 * @brief Releases ownership of the object
 *
 * @param ptr Address of the memory management object
 */
static inline void autoptr_release(void *ptr)
{
        autoptr_assert(ptr);

        AUTOPTR_M_LOCK(ptr);
        assert(--AUTOPTR_M(ptr)->r_count >= 0);
        AUTOPTR_M_UNLOCK(ptr);
}

/**
 * @brief Binds a reference to an object
 *
 * @param ptr Address of memory managed object
 * @return Address of memory managed object
 * @note This procedure is similar to @c autoptr_retain, but here returns the address of the managed object
 */
static inline void *autoptr_bind(void *ptr)
{
        autoptr_retain(ptr);
        return ptr;
}

/**
 * @brief Unbinds an object reference
 *
 * @param ptr Pointer to address of memory managed object
 */
void autoptr_unbind(void **ptr);

/**
 * @brief Binds a list of references to a vector of objects
 *
 * @param ptr Address of memory managed object vector
 * @param size Size of the vector (i.e. number of objects in the vector)
 * @retval ptr_list List of object references (vector of pointers; of size @c size)
 *
 */
void autoptr_vbindl(void *ptr, size_t size, void *ptr_list[]);

/**
 * @brief Binds a list of references to a list of objects
 *
 * @param ptr List of managed objects (vector of pointers)
 * @param size Size of the list (i.e. number of objects in the list)
 * @retval ptr_list List of object references (vector of pointers)
 */
void autoptr_lbindl(void *ptr[], size_t size, void *ptr_list[]);

/**
 * @brief Unbinds a list of references
 *
 * @param ptr_list List of managed objects (vector of pointers)
 * @param size Size of the list (i.e. number of objects in the list)
 */
void autoptr_lunbind(void *ptr_list[], size_t size);

/**
 * @brief Generic procedure for freeing memory managed objects
 *
 * Provides a standard procedure for destroying a memory managed object and
 * freeing its heap allocation.
 *
 * @param ptr Pointer to address of memory managed object; the object address is set to @c NULL upon returning.
 *
 */
void autoptr_free_obj(void **ptr);

/**
 * @brief Generic procedure for freeing a vector of memory managed objects
 *
 * Provides a standard procedure for destroying a vector of memory managed
 * objects and freeing its heap allocation.
 *
 * @param ptr Pointer to address of memory managed object vector; the vector address is set to @c NULL upon
 * returning.
 * @param size Size of the vector (i.e. number of objects in the vector)
 *
 */
void autoptr_vfree_obj(void **ptr, size_t size);

// Undefine local definitions
#undef AUTOPTR
#undef AUTOPTR_LOCK
#undef AUTOPTR_UNLOCK

#undef AUTOPTR_M
#undef AUTOPTR_M_LOCK
#undef AUTOPTR_M_UNLOCK

#ifdef __cplusplus
}
#endif

#endif // __AUTOPTR_H__
