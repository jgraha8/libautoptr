#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include <libautoptr/autoptr.h>

static int test_initd = 0;

struct test {
	struct autoptr __autoptr;
};

static void test_dtor(struct test *t)
{
	if( !autoptr_destroy_ok(t) ) {
		autoptr_release(t);
		return;
	}
		
	assert( --test_initd >= 0 );
}

__attribute__((unused))
static struct test *test_alloc()
{
	struct test *t = calloc(1, sizeof(*t));

	autoptr_ctor((struct autoptr *)t, sizeof(*t), (void (*)(void *))test_dtor);
	autoptr_set_allocd(t, true);

	++test_initd;

	return t;
}

__attribute__((unused))
static struct test *test_valloc(size_t n)
{
	struct test *t = calloc(n, sizeof(*t));

	autoptr_ctor((struct autoptr *)t, sizeof(*t), (void (*)(void *))test_dtor);
	autoptr_set_allocd(t, true);
	autoptr_set_managed(t, n);

	test_initd += n;

	return t;
}

#endif // __TEST_COMMON_H__
