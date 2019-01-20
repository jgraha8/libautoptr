#include <assert.h>
#include <stdlib.h>

#include <libautoptr/autoptr.h>
#include "test_common.h"

int main(int argc, char **argv)
{
	struct test *t = test_valloc(3);
	struct test *p0 = autoptr_bind(&t[0]);	
	struct test *p1 = autoptr_bind(&t[1]);
	struct test *p2 = autoptr_bind(&t[2]);
	
	assert( ! autoptr_destroy_ok(t) );

	// Release ownership of primary test object
	autoptr_release(t);

	autoptr_unbind((void **)&p0);
	assert( p0 == NULL );
	assert(test_initd);

	autoptr_unbind((void **)&p1);
	assert( p1 == NULL );
	assert(test_initd);	

	autoptr_unbind((void **)&p2);
	assert( p2 == NULL );
	
	// Ensure that destructor callback was called
	assert(!test_initd);	

	return 0;
}
