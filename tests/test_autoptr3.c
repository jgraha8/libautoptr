#include <assert.h>
#include <stdlib.h>

#include "test_common.h"
#include <libautoptr/autoptr.h>

int main(int argc, char **argv)
{
        struct test *t  = test_valloc(3);
        struct test *p0 = autoptr_bind(&t[0]);
        struct test *p1 = autoptr_bind(&t[1]);
        struct test *p2 = autoptr_bind(&t[2]);

        assert(!autoptr_destroy_ok(t));

        autoptr_unbind((void **)&p0);
        assert(p0 == NULL);
        assert(test_initd);

        autoptr_unbind((void **)&p1);
        assert(p1 == NULL);
        assert(test_initd);

        autoptr_unbind((void **)&p2);
        assert(p2 == NULL);
        assert(test_initd);

        autoptr_vfree_obj((void **)&t, 3);
        assert(!test_initd);

        return 0;
}
