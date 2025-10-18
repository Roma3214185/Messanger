#ifndef META_H
#define META_H
#include <functional>
#include <any>
#include "../../DebugProfiling/Debug_profiling.h"
#include <QDateTime>

struct Field{
    const char* name;
    const std::type_info& type;
    std::function<std::any(const void*)> get;
    std::function<void(void*, const std::any&)> set;
};

struct Meta {
    const char* name;
    const char* tableName;
    std::vector<Field> fields;

    const Field* find(const std::string& n) const {
        for (const auto& f : fields)
            if (n == f.name) return &f;
        return nullptr;
    }
};

template <class T, class M>
Field make_field(const char* name, M T::* member) {
    return Field{
        .name = name,
        .type = typeid(M),
        .get = [member](const void* obj) -> std::any {
            const T* element = static_cast<const T*>(obj);
            return element->*member;
        },
        .set = [member](void* obj, const std::any& val) {
            T* element = static_cast<T*>(obj);

            if constexpr (std::is_same_v<M, QDateTime>) {
                if (!val.has_value()) {
                    LOG_ERROR("!has value");
                    return;
                }

                if (val.type() == typeid(QDateTime)) {
                    element->*member = std::any_cast<QDateTime>(val);
                } else if (val.type() == typeid(long long)) {
                    element->*member = QDateTime::fromSecsSinceEpoch(std::any_cast<long long>(val));
                } else if (val.type() == typeid(int)) {
                    element->*member = QDateTime::fromSecsSinceEpoch(std::any_cast<int>(val));
                } else {
                    LOG_ERROR("Invalid type for QDateTime: '{}'", val.type().name());
                }
            } else {
                if (!val.has_value()) return;
                if constexpr (std::is_same_v<M, std::string>) {
                    if (val.type() == typeid(const char*))
                        (element->*member) = std::string(std::any_cast<const char*>(val));
                    else
                        (element->*member) = std::any_cast<M>(val);
                } else {
                    (element->*member) = std::any_cast<M>(val);
                }
            }

        }
    };
}


template<typename T>
struct Reflection {
    static Meta meta();
};


#endif // META_H
