#pragma once

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/variant.hpp>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <variant>

#include "spdlog/spdlog.h"

/*
 This needs to be reworked, the current approach with std::variant was done
 largely to save time in prototpying, but requires deep injection of
 dependencies to the base layer. Would like if possible for the component types
 to not be compile time dependent. I am not sure the best way to do this.

Pros of current method: Fast to implement, easy to define relations between
components, easy to serialize Cons: Probably bad design and requires lots of
compile time modiciation and weird stuff. I am sure there is a better way

*/

struct AssemblySlot {
    virtual std::string toString() const = 0;
    virtual bool isDistinct() const = 0;
    virtual bool isSameType(const AssemblySlot& s) const = 0;
    virtual bool isSamePos(const AssemblySlot& s) const = 0;
    virtual int getType(const AssemblySlot& s) const = 0;
};

template <typename C>
struct has_to_string_func {
   private:
    template <typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().toString()),
                              std::string>::type;

    template <typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

   public:
    static constexpr bool value = type::value;
};

template <typename T>
struct fmt::formatter<T, std::enable_if_t<has_to_string_func<T>::value, char>> {
    formatter<std::string> stringfmt;

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return stringfmt.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const T& s, FormatContext& ctx) {
        return stringfmt.format(s.toString(), ctx);
    }
};

#define CreateFunctionChecker(name, func, ret, ...)                     \
    template <typename, typename T>                                     \
    struct name##_func {                                                \
        static_assert(                                                  \
            std::integral_constant<T, false>::value,                    \
            "Second template parameter needs to be of function type."); \
    };                                                                  \
    template <typename C, typename Ret, typename... Args>               \
    struct name##_func<C, Ret(Args...)> {                               \
       private:                                                         \
        template <typename T>                                           \
        static constexpr auto check(T*) -> typename std::is_same<       \
            decltype(std::declval<T>().func(std::declval<Args>()...)),  \
            Ret>::type;                                                 \
                                                                        \
        template <typename>                                             \
        static constexpr std::false_type check(...);                    \
                                                                        \
        typedef decltype(check<C>(0)) type;                             \
                                                                        \
       public:                                                          \
        static constexpr bool value = type::value;                      \
    };                                                                  \
                                                                        \
    template <typename T>                                               \
    constexpr bool name = name##_func<T, ret(__VA_ARGS__)>::value

CreateFunctionChecker(has_get_hash, getHash, std::size_t, );
CreateFunctionChecker(has_same_pos, isSamePos, bool, const T&);
CreateFunctionChecker(has_same_type, isSameType, bool, const T&);

inline void hash_combine([[maybe_unused]] std::size_t& seed) {}

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}

enum CompType { Engine = 1, Module, Wagon };

template <typename T, typename = int>
struct HasIsDistinct : std::false_type {};

template <typename T>
struct HasIsDistinct<T, decltype((void)T::is_distinct, 0)> : std::true_type {};

#define HASHABLE(...)                    \
    inline std::size_t getHash() const { \
        std::size_t h = 0;               \
        hash_combine(h, __VA_ARGS__);    \
        return h;                        \
    }

struct ExternalComponent {
    std::string identifier;
    ExternalComponent() = default;
    ExternalComponent(std::string i) : identifier{std::move(i)} {}
    std::string toString() const {
        return fmt::format("ExtComp({})", identifier);
    }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(
            // cereal::make_nvp("ComponentType", comp_type),
            cereal::make_nvp("ID", identifier));
    }
};

struct SlotCassette {
    struct PlaneID {
        int plane, icas0, icas1;
        bool operator==(const PlaneID& other) const {
            return std::tie(plane, icas0, icas1) ==
                   std::tie(other.plane, other.icas0, other.icas1);
        }
        std::string toString() const {
            return fmt::format("{}, {}, {}", plane, icas0, icas1);
        }
        template <class Archive>
        void serialize(Archive& archive) {
            archive(cereal::make_nvp("Plane", plane),
                    cereal::make_nvp("ICas1", icas0),
                    cereal::make_nvp("ICas2", icas1));
        }
    } loc;
    //  using PlaneID = std::tuple<std::size_t, std::size_t, std::size_t>;
    //   PlaneID loc;
    std::string toString() const { return fmt::format("Cassette {}", loc); }
    auto getCassette() const { return loc; }

    bool isSameType(const SlotCassette& c) const { return c.loc == loc; }

    SlotCassette() = default;
    //    SlotCassette(std::size_t pl, std::string s)
    //        : plane{pl}, type{std::move(s)} {}
    SlotCassette(PlaneID pl) : loc{std::move(pl)} {}

    bool operator==(const SlotCassette& w1) const { return loc == w1.loc; }
    HASHABLE(loc.plane, loc.icas0, loc.icas1)
    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("PlaneID", loc));
    }
};

struct SlotModule {
    std::string toString() const {
        return fmt::format("Module type {} at ({},{}) for Cassette {},", type,
                           u, v, cas.toString());
    }
    int u = 0, v = 0;
    SlotCassette cas;
    //    std::size_t plane = 0;
    std::string type = "";
    bool isSameType(const SlotModule& s) const { return s.type == type; }
    bool isSamePos(const SlotModule& s) const {
        return std::tie(u, v, cas) == std::tie(s.u, s.v, s.cas);
    }
    auto getCassette() const { return cas.getCassette(); }
    SlotModule() = default;
    SlotModule(int x, int y, SlotCassette z, std::string t = "")
        : u{x}, v{y}, cas{z}, type{t} {}
    bool operator==(const SlotModule& other) const {
        return std::tie(cas, u, v, type) ==
               std::tie(other.cas, other.u, other.v, other.type);
    }

    //    bool operator<(const SlotModule& other) const {
    //        return std::tie(cas, u, v, type) <
    //               std::tie(cas.plane, other.u, other.v, other.type);
    //    }
    HASHABLE(u, v, cas, type)
    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("U", u), cereal::make_nvp("V", v),
                cereal::make_nvp("Cassette", cas),
                cereal::make_nvp("Type", type));
    }
    static constexpr bool is_distinct = true;
};
// This is bad
struct SlotNull {
    std::string toString() const { return fmt::format("Nullslot"); }
    bool operator==(const SlotNull& other) const {
        (void)other;
        return true;
    }
    SlotCassette::PlaneID getCassette() const { return {-1, -1, -1}; }
    template <class Archive>
    void serialize(Archive& archive) {
        (void)archive;
    }

    HASHABLE(0);
};

struct SlotEngine {
    SlotModule left;
    SlotModule right;
    std::string type = "LD";

    std::string toString() const {
        return fmt::format("Engine between modules {} {}", left.toString(),
                           right.toString());
    }
    bool isSameType(const SlotEngine& s) const { return s.type == type; }
    auto getCassette() const { return left.getCassette(); }
    SlotEngine() = default;
    SlotEngine(SlotModule r, SlotModule l, std::string t = "LD")
      : left{l}, right{r}, type{std::move(t)} {}

    bool operator==(const SlotEngine& w1) const {
        return right == w1.right && left == w1.left;
    }
    HASHABLE(left, right, type)
    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("Left", left),
                cereal::make_nvp("Right", right),
                cereal::make_nvp("Type", type));
    }
};
struct SlotDepDCDC {
    SlotModule mod;
    std::string toString() const {
        return fmt::format("Deported DCDC module on {}", mod);
    }
    auto getCassette() const { return mod.getCassette(); }
    SlotDepDCDC() = default;
    SlotDepDCDC(SlotModule r) : mod{r} {}

    bool operator==(const SlotDepDCDC& w1) const { return mod == w1.mod; }
    HASHABLE(mod)
    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("Module", mod));
    }
};

struct SlotScrews {
    std::string toString() const {
        return fmt::format("Screws for module {}", mod.toString());
    }
    SlotModule mod;
    std::string type;
    auto getCassette() const { return mod.getCassette(); }
    SlotScrews() = default;
    SlotScrews(SlotModule r, std::string s = "") : mod{r}, type{std::move(s)} {}

    bool operator==(const SlotScrews& w1) const { return mod == w1.mod; }
    HASHABLE(mod, type)
    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("Module", mod),
                cereal::make_nvp("Type", type));
    }
    static constexpr bool is_distinct = false;
};

struct SlotWagon {
    std::string toString() const {
        return fmt::format("Wagon type {} from engine {}", type,
                           eng.toString());
    }
    SlotEngine eng;
    std::string type = "";
    int leftright = 0;
    int orientation = 0;

    bool isSameType(const SlotWagon& w) const { return w.type == type; }
    auto getCassette() const { return eng.getCassette(); }
    SlotWagon() = default;
    SlotWagon(SlotEngine e, std::string t, int lr, int orient)
        : eng{e}, type{std::move(t)}, leftright{lr}, orientation{orient} {}
    bool operator==(const SlotWagon& w1) const {
        return std::tie(eng, type, leftright, orientation) ==
               std::tie(w1.eng, w1.type, w1.leftright, w1.orientation);
    }
    HASHABLE(eng, type, leftright, orientation)
    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("Engine", eng), cereal::make_nvp("Type", type),
                cereal::make_nvp("LR", leftright),
                cereal::make_nvp("Orient", orientation));
    }
};

using AnyType = std::variant<SlotWagon, SlotModule, SlotEngine, SlotScrews,
                             SlotCassette, SlotDepDCDC, SlotNull>;
class CasSlot {
    AnyType value;

   public:
    std::string toString() const {
        return std::visit([](auto&& x) { return x.toString(); }, value);
    }
    std::size_t getHeldType() const { return value.index(); }

    template <typename T>
    T getValue() const {
        return std::get<T>(value);
    }

    template <typename T>
    bool isType() const {
        return std::holds_alternative<T>(value);
    }

    std::size_t getHash() const;

    bool isSameType(const CasSlot& c) const {
        static_assert(has_same_type<std::remove_reference_t<SlotModule>>);
        bool same_slot = (c.value.index() == value.index());
        if (!same_slot) return false;
        bool res = std::visit(
            [&](const auto& x, const auto& y) {
                if constexpr (std::is_same_v<decltype(x), decltype(y)> &&
                              has_same_type<
                                  std::remove_reference_t<decltype(x)>>) {
                    return x.isSameType(y);
                } else {
                    return true;
                }
            },
            value, c.value);
        return res;
    }

    bool isSamePos(const CasSlot& c) const {
        bool same_slot = (c.value.index() == value.index());
        if (!same_slot)
            return false;
        else {
            bool res = std::visit(
                [&](const auto& x, const auto& y) {
                    if constexpr (std::is_same_v<decltype(x), decltype(y)> &&
                                  has_same_pos<
                                      std::remove_reference_t<decltype(x)>> &&
                                  has_same_pos<
                                      std::remove_reference_t<decltype(y)>>) {
                        return x.isSamePos(y);
                    } else {
                        return true;
                    }
                },
                value, c.value);
            return res;
        }
    }

    bool isDistinct() const {
        bool res = std::visit(
            [&](const auto& x) {
                if constexpr (HasIsDistinct<std::remove_reference_t<
                                  decltype(x)>>::value) {
                    return x.is_distinct;
                } else {
                    return true;
                }
            },
            value);
        return res;
    }

    auto getCassette() const {
        return std::visit([](auto&& x) { return x.getCassette(); }, value);
    }

    template <typename T>
    CasSlot(T t) : value{std::move(t)} {}
    CasSlot() = default;
    CasSlot(AnyType t) : value{std::move(t)} {};

    bool operator==(const CasSlot& c2) const { return value == c2.value; }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("CasSlot", value));
    }
};

#undef HASHABLE

#define MAKE_STD_HASH(classname)                           \
    template <>                                            \
    struct hash<classname> {                               \
        std::size_t operator()(const classname& k) const { \
            return k.getHash();                            \
        }                                                  \
    };

namespace std {
template <>
struct hash<CasSlot> {
    std::size_t operator()(const CasSlot& k) const {
        using std::hash;
        using std::size_t;
        return k.getHash();
    }
};

// template <typename T>
// struct hash<typename std::enable_if<has_get_hash<T>,T>::type> {
//    std::size_t operator()(const T& k) const { return k.getHash(); }
//};

MAKE_STD_HASH(SlotModule)
MAKE_STD_HASH(SlotEngine)
MAKE_STD_HASH(SlotWagon)
MAKE_STD_HASH(SlotScrews)
MAKE_STD_HASH(SlotCassette)
MAKE_STD_HASH(SlotDepDCDC)
MAKE_STD_HASH(SlotNull)
}  // namespace std
