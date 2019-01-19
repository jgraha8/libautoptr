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
#include <libautoptr/autoptr.h>
#include <stdlib.h>
#include <string.h>

#define AUTOPTR(a) ((struct autoptr *)(a))
#define AUTOPTR_LOCK(a) (pthread_mutex_lock(&AUTOPTR(a)->mutex))
#define AUTOPTR_UNLOCK(a) (pthread_mutex_unlock(&AUTOPTR(a)->mutex))

#define AUTOPTR_M(a) (AUTOPTR(a)->manager)
#define AUTOPTR_M_LOCK(a) (pthread_mutex_lock(&AUTOPTR_M(a)->mutex))
#define AUTOPTR_M_UNLOCK(a) (pthread_mutex_unlock(&AUTOPTR_M(a)->mutex))

void autoptr_ctor(void *ptr, size_t obj_len, void (*obj_dtor)(void *))
{
        memset(ptr, 0, sizeof(struct autoptr));

        AUTOPTR(ptr)->__magic = AUTOPTR_MAGIC;
        assert(pthread_mutex_init(&AUTOPTR(ptr)->mutex, NULL) == 0);
        AUTOPTR(ptr)->obj_len     = obj_len;
        AUTOPTR(ptr)->obj_dtor    = obj_dtor;
        AUTOPTR(ptr)->manager     = AUTOPTR(ptr); // defaults to self
        AUTOPTR(ptr)->num_managed = 1;
}

void autoptr_dtor(void *ptr)
{
        autoptr_assert(ptr);
        pthread_mutex_destroy(&AUTOPTR(ptr)->mutex);
}

void autoptr_set_obj(void *ptr, size_t obj_len, void (*obj_dtor)(void *))
{
        AUTOPTR_M(ptr)->obj_len  = obj_len;
        AUTOPTR_M(ptr)->obj_dtor = obj_dtor;
}

void autoptr_unbind(void **ptr)
{
        if (*ptr == NULL)
                return;

        if (AUTOPTR_M(*ptr) == NULL)
                return;

        if (!autoptr_destroy(*ptr)) {
                autoptr_release(*ptr);
                return;
        }

        // The lock shouldn't be needed since all object references have been cleared
        // AUTOPTR_M_LOCK(*ptr);

        struct autoptr *manager = AUTOPTR_M(*ptr);
        size_t num_managed      = manager->num_managed;
        bool allocd             = manager->allocd;

        // Call the destructor for all objects (going in reverse)
        for (ssize_t i = num_managed - 1; i >= 0; --i) {
                void *obj = (void *)((char *)manager + manager->obj_len * i);

                if (i > 0) {
                        assert(AUTOPTR(obj)->manager == manager);
                        assert(AUTOPTR(obj)->num_managed == 0);
                }

                AUTOPTR(obj)->obj_dtor(obj);
        }

        if (allocd) {
                // We free the manager object
                free(manager);
        }
        *ptr = NULL;
}

void autoptr_vbindl(void *ptr, size_t size, void *ptr_list[])
{
        size_t n;
        for (n              = 0; n < size; ++n)
                ptr_list[n] = autoptr_bind(ptr + AUTOPTR(ptr)->obj_len * n);
}

void autoptr_lbindl(void *ptr[], size_t size, void *ptr_list[])
{
        size_t n;
        for (n              = 0; n < size; ++n)
                ptr_list[n] = autoptr_bind(ptr[n]);
}

void autoptr_lunbind(void *ptr_list[], size_t size)
{
        size_t n;
        for (n = 0; n < size; ++n)
                autoptr_unbind(ptr_list + n);
}

void autoptr_free_obj(void **ptr)
{
        assert(*ptr != NULL);
        autoptr_unbind(ptr);
        *ptr = NULL;
}

void autoptr_vfree_obj(void **ptr, size_t size)
{
        assert(*ptr != NULL);

        // Based on the assumption of this functions usage, we take that the passed
        // object is the manager and that size is that of the number of managed objects
        AUTOPTR_M_LOCK(*ptr);
        assert(AUTOPTR_M(*ptr) == AUTOPTR(*ptr));
        assert(AUTOPTR_M(*ptr)->num_managed == size);
        AUTOPTR_M_UNLOCK(*ptr);

        autoptr_unbind(ptr);
        *ptr = NULL;
}
