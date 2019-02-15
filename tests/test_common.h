#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include <string.h>

#include <libautoptr/autoptr.h>

static int test_initd = 0;

struct test {
        struct autoptr __autoptr;
        int data;
};

static void test_dtor(struct test *t);
static void test_ctor(struct test *t)
{
        autoptr_ctor(t, sizeof(*t), (void (*)(void *))test_dtor);
        ++test_initd;

        t->data = 42;
}

static void test_dtor(struct test *t)
{
        if (!autoptr_destroy_ok(t)) {
                autoptr_release(t);
                return;
        }

        assert(--test_initd >= 0);
        autoptr_zero_obj(t);
}

__attribute__((unused)) static struct test *test_alloc()
{
        struct test *t = calloc(1, sizeof(*t));
        test_ctor(t);

        autoptr_set_allocd(t, true);

        return t;
}

__attribute__((unused)) static struct test *test_valloc(size_t n)
{
        struct test *t = calloc(n, sizeof(*t));

        for (size_t i = 0; i < n; ++i) {
                test_ctor(t + i);
        }
        autoptr_set_allocd(t, true);
        autoptr_set_managed(t, n);

        return t;
}

#endif // __TEST_COMMON_H__
