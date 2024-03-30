/*
bazelisk run --config=default_cpp20 //src/tests:test_helpers
*/

#include <gtest/gtest.h>

#include "src/helpers/typelist.h"
#include <cstdint>
#include <type_traits>

namespace compound_unit::typelist_helper
{
namespace test
{
struct Type0
{};

struct Type1
{};

} // namespace test

TEST(has_type_in_typelist, case_non_empty_typelist)
{
    using TestTypeListA =
        TypeList<test::Type0, std::int32_t, test::Type1&&, const std::int64_t, double&>;

    EXPECT_TRUE((TestTypeListA::has_type<int>));
    EXPECT_TRUE((TestTypeListA::has_type<int>));
    EXPECT_TRUE((TestTypeListA::has_type<test::Type0>));

    // The type qualifiers do matter.
    EXPECT_TRUE((TestTypeListA::has_type<test::Type1&&>));

    EXPECT_FALSE((TestTypeListA::has_type<std::int64_t>));
    EXPECT_TRUE((TestTypeListA::has_type<const std::int64_t>));

    EXPECT_TRUE((TestTypeListA::has_type<double&>));
    EXPECT_FALSE((TestTypeListA::has_type<double>));

    EXPECT_FALSE((TestTypeListA::has_type<char>));
}

TEST(has_type_in_typelist, case_empty_typelist)
{
    using TestTypeList = TypeList<>;

    EXPECT_FALSE((TestTypeList::has_type<int>));
    EXPECT_FALSE((TestTypeList::has_type<bool&>));
}

TEST(union_types_of_two_typelist, case_two_non_empty_typelists)
{
    using TypeList0 = TypeList<double, std::int32_t, const double&>;
    using TypeList1 = TypeList<double, std::int64_t, double&&>;

    using JointType = typelist_union_t<TypeList0, TypeList1>;
    using ExpectedType = TypeList<double, std::int32_t, const double&, std::int64_t, double&&>;

    EXPECT_TRUE((std::same_as<JointType, ExpectedType>));
}

TEST(typelist_typec_cat, _)
{
    { // cat non-empty typelist
        using TupleA = TypeList<int, float>;
        using TupleB = TypeList<float, double>;

        using TypeExpected = TypeList<int, float, float, double>;
        EXPECT_TRUE((std::same_as<TypeExpected, typelist_cat_t<TupleA, TupleB>>));
    }

    { // cat empty typelist
        using TupleA = TypeList<>;
        using TupleB = TypeList<float, double>;

        using TypeExpected = TypeList<float, double>;
        EXPECT_TRUE((std::same_as<TypeExpected, typelist_cat_t<TupleA, TupleB>>));
    }
}

TEST(union_types_of_two_typelist, _)
{
    { // case_typelist_0_empty_typelist_1_non_empty
        using TypeList0 = TypeList<>;
        using TypeList1 = TypeList<std::int32_t>;

        using JointType = typelist_union_t<TypeList0, TypeList1>;
        using ExpectedType = TypeList1;

        EXPECT_TRUE((std::is_same_v<JointType, ExpectedType>));
    }

    { // case_typelist_0_nonempty_typelist_1_empty
        using TypeList0 = TypeList<std::int32_t>;
        using TypeList1 = TypeList<>;

        using JointType = typelist_union_t<TypeList0, TypeList1>;
        using ExpectedType = TypeList0;

        EXPECT_TRUE((std::is_same_v<JointType, ExpectedType>));
    }

    { // case_both_typelists_empty
        using TypeList0 = TypeList<>;
        using TypeList1 = TypeList<>;

        using JointType = typelist_union_t<TypeList0, TypeList1>;
        using ExpectedType = TypeList<>;

        EXPECT_TRUE((std::is_same_v<JointType, ExpectedType>));
    }

    { // case Two typelist exactly same (order same, element unique)
        using TypeList0 = TypeList<std::int8_t, std::int16_t, std::int32_t, std::int64_t>;
        using JointType = typelist_union_t<TypeList0, TypeList0>;

        using ExpectedType = TypeList0;
        EXPECT_TRUE((std::is_same_v<JointType, ExpectedType>));
    }

    { // case Two typelist element type same but order different, element unique
        using TypeList0 = TypeList<std::int8_t, std::int16_t, std::int32_t, std::int64_t>;
        using TypeList1 = TypeList<std::int8_t, std::int64_t, std::int32_t, std::int16_t>;

        {
            using JointType = typelist_union_t<TypeList0, TypeList1>;
            using ExpectedType = TypeList0; // The joint typelist type will follow the
                                            // order the first template arg.
            EXPECT_TRUE((std::is_same_v<JointType, ExpectedType>));
        }
        {
            using JointType = typelist_union_t<TypeList1, TypeList0>;
            using ExpectedType = TypeList1; // The joint typelist type will follow the
                                            // order the first template arg.
            EXPECT_TRUE((std::is_same_v<JointType, ExpectedType>));
        }
    }

    { // case Two typelist exactly same, order same, element not unique.
        using TypeListA = TypeList<std::int8_t, std::int16_t, float, std::int32_t, float,
                                   std::int8_t, std::int64_t>;
        using JointType = typelist_union_t<TypeListA, TypeListA>;
        using ExpectedType = TypeList<std::int8_t, std::int16_t, float, std::int32_t, std::int64_t>;
        EXPECT_TRUE((std::is_same_v<JointType, ExpectedType>));
    }
}

TEST(typelist_element_type_unique, _)
{
    {
        using TypeListA = TypeList<std::int32_t, char, bool>;
        EXPECT_TRUE((is_each_type_unique<TypeListA>));
    }

    {
        using TypeListA = TypeList<std::int32_t, char, bool, char>;
        EXPECT_FALSE((is_each_type_unique<TypeListA>));
    }

    {
        using TypeListA = TypeList<>;
        EXPECT_TRUE((is_each_type_unique<TypeListA>));
    }
}

TEST(remove_duplicated_types, _)
{
    { // case_no_duplicated_elements
        using TestTypeList = TypeList<int, float, double, std::int64_t&, int*>;
        using OutputType = remove_duplicated_type_t<TestTypeList>;
        EXPECT_TRUE((std::is_same_v<OutputType, TestTypeList>));
    }

    { // case_duplicated_elements
        using TestTypeList = TypeList<int, float, bool, double, std::int64_t&, int*, double, double,
                                      char, const bool>;
        using OutputType = remove_duplicated_type_t<TestTypeList>;
        using ExpectedType =
            TypeList<int, float, bool, double, std::int64_t&, int*, char, const bool>;
        EXPECT_TRUE((std::is_same_v<OutputType, ExpectedType>));
    }

    { // case_empty_typelist
        using TestTypeList = TypeList<>;
        using OutputType = remove_duplicated_type_t<TestTypeList>;
        EXPECT_TRUE((std::is_same_v<OutputType, TestTypeList>));
    }
}

TEST(typelists_interchangeable, _)
{
    {
        using TypeList0 = TypeList<std::int8_t, std::int16_t, std::int32_t, std::int64_t>;
        using TypeList1 = TypeList<std::int8_t, std::int64_t, std::int32_t, std::int16_t>;
        EXPECT_TRUE((are_typelists_interchangeable_v<TypeList0, TypeList1>));
    }

    {
        using TypeList0 = TypeList<std::int8_t>;
        using TypeList1 = TypeList<std::int8_t>;
        EXPECT_TRUE((are_typelists_interchangeable_v<TypeList0, TypeList1>));
    }
    {
        using TypeList = TypeList<>;
        EXPECT_TRUE((are_typelists_interchangeable_v<TypeList, TypeList>));
    }
}

} // namespace compound_unit::typelist_helper
