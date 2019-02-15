#include <assert.h>
#include <stdlib.h>

#include <libautoptr/autoptr.h>
#include "test_common.h"

int main(int argc, char **argv)
{
	struct test t;
	struct test *p = NULL;
	
	test_ctor(&t);
	p = autoptr_bind(&t);

	assert( ! autoptr_destroy_ok(&t) );

	// Release ownership of primary test object
	autoptr_release(&t);

	// The test object should be destroyable (i.e., single owner)
	assert( autoptr_destroy_ok(&t) );

	autoptr_unbind((void **)&p);
	assert( p == NULL );

	// Ensure that destructor callback was called
	assert(!test_initd);

	return 0;
}
