#pragma once
#include <memory>

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
