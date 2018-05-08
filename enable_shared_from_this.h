#ifndef ENABLE_SHARED_FROM_THIS_H
#define ENABLE_SHARED_FROM_THIS_H
#include <memory>

#pragma GCC diagnostic push
/
// Effective C++ (correctly) warns when a non-virtual destructor is
// in the base class of one with a virtual destructor.  However, the
// enable_shared_from_this *explicitly* is built for exactly this
// behavior.
//
// This wrapper class is just to suppress the non-virtual destructor
// warning.  Derive from this class instead of std::enable_shared_from_this
// for gcc.
//
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
template <typename T>
class enable_shared_from_this : public std::enable_shared_from_this<T>
{
public:
    virtual ~enable_shared_from_this() = default;
};
#pragma GCC diagnostic pop

#endif // ENABLE_SHARED_FROM_THIS_H
