# libautoptr

## Overview

The intent of this module is to provide a mechanism for maintaining ownership of
a shared object. The primary data structure autoptr contains a reference counter
that indicates how many references (i.e. shared owners, not including the
creator reference) an object currently has. The reference counter for an object
begins at zero since we are using the strategy that if a calling procedure
creates an object, it is responsible for destroying it (i.e. it is the object
creator). An object is said to be shared through the notion of binding. When a
reference is bound to an object, the object lifetime is guaranteed to be of at
least that of the reference, even if the creator of the object attempts to
destroy the object. This of course requires the object's destructor/free
procedures to be conformant to these rules and for the constructor/alloc
procedures to properly setup its memory management data structure (see below).

The rules for managing the lifetime of an object are:

- The creation of an object does not increase the reference count of an object
  (i.e. it must be zero upon creation).

   We use the idea that an object should be destroyed in a like manner in which
   it was created. That is, if an object is created by its constructor/alloc
   procedures, then it must be destroyed by calls to its destructor/free
   procedures. This provides consistency in the usage patterns for objects.

- The lifetime of an object can be guaranteed by using a bound reference
  (e.g. using autoptr_bind).

   When binding a reference to an object its reference count is
   incremented. Once the object reference is no longer needed it may be unbound
   using autoptr_unbind or similar procedures.

- An object may be destroyed only if its reference count is zero upon entry to
     an unbind procedure call or the object's destructor/free procedure call.

   This occurs in one of two ways:

   -# the object has no shared references and its creator calls its
      destructor/free procedure

   -# the creator of the object has released the object (remember it does not
      use a bound reference so an unbind is not called by the creator) or called
      its destructor/free procedures while the object is shared, and an unbind
      is called on the last shared object reference. In this case the object's
      destructor/free procedures will be called and the object will be
      destroyed.


## Conformant Usage

To use the memory management support for an object, we first need to use autoptr
within the managed data structure. The autoptr structure is required to be used
as the first member of a managed data structure. An example usage is
 
    struct my_struct {
        struct autoptr __autoptr; // Treating as a "private" member
        ...
    };
 
where the my_struct instance may be cast to autoptr. We use void pointers for
all of the autoptr procedures and perform this cast internally. When
constructing the mangaged object we also construct the autoptr member using
 
    autoptr_ctor( my_struct, sizeof(*my_struct),
 		      (void (*)(void *))my_struct_dtor,
 		      (void (*)(void **))my_struct_free,
 		      (void (*)(void **, cl_uint))my_struct_vfree );
 
where the my_struct_dtor is the object destructor and my_struct_free and
my_struct_vfree are the free and vector free procedures (for heap allocated
instances), respectively. In most cases, one can use the generic free and vfree
procedures provided by the autoptr module such that the call then becomes
 
    autoptr_ctor( my_struct, sizeof(*my_struct),
 		      (void (*)(void *))my_struct_dtor,
 		      autoptr_free_obj,
 		      autoptr_vfree_obj );
 
If the an object does not use heap or vector (heap) allocation, then the free
functions may be set to NULL (a free procedure is called only if the allocd flag
is true). The object destructor is required since it provides all of the
internal cleanup of the object and will be called by the free procedures. If an
object is heap-allocated then the allocd flag must be set using
 
    autoptr_set_allocd( my_struct, true );
 
during construction of the object. The object destructor, then needs to have as
the first statement a check for whether or not the object is to be destroyed
(this is based on the reference count). For our example my_struct, its
destructor looks like
 
    void my_struct_dtor( struct my_struct *my_struct )
    {
            if( ! autoptr_destroy( my_struct ) ) {
                    autoptr_release( my_struct );
                    return;
            }
            // Destroy the internal state otherwise
            ...
    }
 
where if the object still has bound references, then the ownership of the
calling scope is released to the other references and the object is not
destroyed, at least until an unbind is performed on the last shared reference.

## Summary

We adopt the notion that an object has a single creator (primary owner) and thus
any additional persistent references must be bound to the object in order to
ensure that the object-lifetime extends to at least the lifetime of the
reference. The creator is responsible for destroying an object, that is unless
another object holds a bound reference to the object. In such a case, then the
creator may release its ownership and responsibilty for cleaning up the object
and the object will automatically be cleaned up when the last bound reference is
unbound.

