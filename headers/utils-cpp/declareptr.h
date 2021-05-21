#pragma once

namespace std {
#ifdef __APPLE__
    inline namespace __1 {
        template<class> class __attribute__ ((__type_visibility__("default"))) shared_ptr;
        template<class> class __attribute__ ((__type_visibility__("default"))) weak_ptr;
    }

    using namespace __1;
#else
    template<class> class shared_ptr;
    template<class> class weak_ptr;
#endif
}

#define DECLARE_PTR(class_name) \
    class class_name; \
    using class_name##Ptr = std::shared_ptr<class_name>; \
    using class_name##CPtr = std::shared_ptr<const class_name>; \
    using class_name##WPtr = std::weak_ptr<class_name>;


#define DECLARE_PTR_STRUCT(class_name) \
    struct class_name; \
    using class_name##Ptr = std::shared_ptr<class_name>; \
    using class_name##CPtr = std::shared_ptr<const class_name>; \
    using class_name##WPtr = std::weak_ptr<class_name>;
