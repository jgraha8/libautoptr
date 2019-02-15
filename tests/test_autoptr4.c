#include <assert.h>
#include <stdlib.h>

#include "test_common.h"
#include <libautoptr/autoptr.h>

int main(int argc, char **argv)
{
        struct test *t = test_valloc(3);
        struct test *p[3];

        // Bind the vector to a list of pointers
        autoptr_vbindl(t, 3, (void **)p);

        // Bind each pointer in the list
        struct test *p0 = autoptr_bind(p[0]);
        struct test *p1 = autoptr_bind(p[1]);
        struct test *p2 = autoptr_bind(p[2]);

        // Transfer ownership
        autoptr_release(t);
        assert(!autoptr_destroy_ok(t));

        // Unbind the list of pointers
        autoptr_lunbind((void **)p, 3);
        assert(test_initd);

        autoptr_unbind((void **)&p0);
        assert(p0 == NULL);
        assert(test_initd);

        autoptr_unbind((void **)&p1);
        assert(p1 == NULL);
        assert(test_initd);

        autoptr_unbind((void **)&p2);
        assert(p2 == NULL);

        // Ensure that destructor callback was called
        assert(!test_initd);

        return 0;
}
