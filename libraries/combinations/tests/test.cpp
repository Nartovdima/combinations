#include "combinations/Combinations.hpp"
#include "combinations/Component.hpp"
#include "gtest/gtest.h"

namespace {

TEST(ComponentTest, from_string_basic) {
    EXPECT_NE(InstrumentType::Unknown, Component::from_string("F 1 2020-02-02").type);
    EXPECT_NE(InstrumentType::Unknown, Component::from_string("U -1 2020-02-02").type);
    EXPECT_NE(InstrumentType::Unknown, Component::from_string("P -1.5 2 2020-02-02").type);
    EXPECT_NE(InstrumentType::Unknown, Component::from_string("C -1 2.5 2020-02-02").type);
    EXPECT_NE(InstrumentType::Unknown, Component::from_string("O -1.5 2.5 2020-02-02").type);
    EXPECT_EQ(InstrumentType::Unknown, Component::from_string("").type);
    EXPECT_EQ(InstrumentType::Unknown, Component::from_string("X 1 2020-02-02").type);
    EXPECT_EQ(InstrumentType::Unknown, Component::from_string("blabla 1 2020-02-02").type);
    EXPECT_EQ(InstrumentType::Unknown, Component::from_string("O blabla 2 2020-02-02").type);
    EXPECT_EQ(InstrumentType::Unknown, Component::from_string("O 1 blabla 2020-02-02").type);
    EXPECT_EQ(InstrumentType::Unknown, Component::from_string("O 1 2 blabla").type);
}

TEST(CombinationsResourceTest, empty_path) {
    Combinations combinations;
    ASSERT_FALSE(combinations.load({}));
}

TEST(CombinationsResourceTest, resource_does_not_exist) {
    const std::filesystem::path path{"test/etc/unknown.xml"};
    std::error_code ec;
    ASSERT_FALSE(std::filesystem::exists(path, ec));

    Combinations combinations;
    ASSERT_FALSE(combinations.load(path));
}

TEST(CombinationsResourceTest, empty_resource) {
    const std::filesystem::path path{"test/etc/empty.xml"};
    std::error_code ec;
    ASSERT_TRUE(std::filesystem::exists(path, ec));

    Combinations combinations;
    ASSERT_FALSE(combinations.load(path));
}

class CombinationsTest: public ::testing::Test {
public:
    static const auto& combinations() { return m_combinations; }

    static bool check_order_basic(std::vector<int> order) {
        std::sort(order.begin(), order.end());
        int counter = 0;
        for (const auto i : order) {
            if (i != ++counter) {
                return false;
            }
        }
        return true;
    }

protected:
    static void SetUpTestCase() {
        const std::filesystem::path path{"test/etc/combinations.xml"};
        std::error_code ec;
        ASSERT_TRUE(std::filesystem::exists(path, ec));

        ASSERT_TRUE(m_combinations.load(path));
    }

private:
    static inline Combinations m_combinations;
};

TEST_F(CombinationsTest, empty) {
    std::vector<int> order;
    ASSERT_EQ("Unclassified", combinations().classify({}, order));
    ASSERT_TRUE(order.empty());
}

TEST_F(CombinationsTest, order_not_empty) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"), Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-09-01"), Component::from_string("F 1 2010-12-01"),
        Component::from_string("F 1 2010-03-01"), Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-09-01"), Component::from_string("F 1 2010-12-01"),
    };
    std::vector<int> order = {1, 2, 3};
    ASSERT_EQ("Bundle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, order_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"), Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-09-01"), Component::from_string("F 1 2010-12-01"),
        Component::from_string("F 1 2010-03-01"), Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-09-01"), Component::from_string("F 1 2010-12-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Bundle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
    for (int i = 0, end = order.size(); i < end; ++i) {
        ASSERT_EQ(i % 4, (order[i] - 1) % 4);
    }
}

TEST_F(CombinationsTest, order_reverse) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-12-01"), Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-06-01"), Component::from_string("F 1 2010-03-01"),
        Component::from_string("F 1 2010-12-01"), Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-06-01"), Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Bundle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
    for (int i = 0, end = order.size(); i < end; ++i) {
        ASSERT_EQ(3 - (i % 4), (order[i] - 1) % 4);
    }
}

TEST_F(CombinationsTest, order_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-09-01"),  // 3 or 7
        Component::from_string("F 1 2010-06-01"),  // 2 or 6
        Component::from_string("F 1 2010-03-01"),  // 1 or 5
        Component::from_string("F 1 2010-03-01"),  // 1 or 5
        Component::from_string("F 1 2010-09-01"),  // 3 or 7
        Component::from_string("F 1 2010-12-01"),  // 4 or 8
        Component::from_string("F 1 2010-12-01"),  // 4 or 8
        Component::from_string("F 1 2010-06-01"),  // 2 or 6
    };
    std::vector<int> order;
    ASSERT_EQ("Bundle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
    ASSERT_EQ(2, (order[0] - 1) % 4);
    ASSERT_EQ(1, (order[1] - 1) % 4);
    ASSERT_EQ(0, (order[2] - 1) % 4);
    ASSERT_EQ(0, (order[3] - 1) % 4);
    ASSERT_EQ(2, (order[4] - 1) % 4);
    ASSERT_EQ(3, (order[5] - 1) % 4);
    ASSERT_EQ(3, (order[6] - 1) % 4);
    ASSERT_EQ(1, (order[7] - 1) % 4);
}

TEST_F(CombinationsTest, min_count_ok) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-03"),
    };
    std::vector<int> order;
    ASSERT_EQ("Options strip", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, min_count_fail) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Unclassified", combinations().classify(components, order));  // not "Options strip"
    ASSERT_TRUE(order.empty());
}

TEST_F(CombinationsTest, much_more) {
    const std::vector<Component> components{65536, Component::from_string("P 1 2000 2010-03-01")};
    std::vector<int> order;
    ASSERT_EQ("Options strip", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, much_more_fail) {
    std::vector<Component> components{65536, Component::from_string("P 1 2000 2010-03-01")};
    components[32777] = Component::from_string("P 2 2000 2010-03-01");
    std::vector<int> order;
    ASSERT_EQ("Unclassified", combinations().classify(components, order));  // not "Options strip"
    ASSERT_TRUE(order.empty());
}

// name: Inter commodity spread
// shortname: ICS
// identifier: 832c4a6e-bd64-11e2-a706-f9d5a0549fe0

TEST_F(CombinationsTest, Inter_commodity_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"),
        Component::from_string("F -1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Inter commodity spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Inter_commodity_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("F -1 2010-03-01"),
        Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Inter commodity spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Future calendar spread
// shortname: FCS
// identifier: d2e5b922-575b-11df-bcaa-5b4c80ccc924

TEST_F(CombinationsTest, Future_calendar_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"),
        Component::from_string("F -1 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Future_calendar_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("F -1 2010-03-02"),
        Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Future butterfly
// shortname: FB
// identifier: d45880f0-575b-11df-bc39-ebffbea5b361

TEST_F(CombinationsTest, Future_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"),
        Component::from_string("F -2 2010-03-02"),
        Component::from_string("F 1 2010-03-03"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Future_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-03"),
        Component::from_string("F -2 2010-03-02"),
        Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Future_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("F -2 2010-03-02"),
        Component::from_string("F 1 2010-03-03"),
        Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Future condor
// shortname: FCo
// identifier: d4efcc6c-575b-11df-83b2-61b63d9de8e6

TEST_F(CombinationsTest, Future_condor_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"),
        Component::from_string("F -1 2010-03-02"),
        Component::from_string("F -1 2010-03-03"),
        Component::from_string("F 1 2010-03-04"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Future_condor_reverse) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-04"),
        Component::from_string("F -1 2010-03-03"),
        Component::from_string("F -1 2010-03-02"),
        Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Future_condor_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"),
        Component::from_string("F -1 2010-03-03"),
        Component::from_string("F 1 2010-03-04"),
        Component::from_string("F -1 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Future condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Pack
// shortname: FP
// identifier: d59f3fee-575b-11df-80f3-4999e81f48b7

TEST_F(CombinationsTest, Pack_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"),
        Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-12-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Pack", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Pack_reverse) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-12-01"),
        Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Pack", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Pack_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-12-01"),
        Component::from_string("F 1 2010-03-01"),
        Component::from_string("F 1 2010-06-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Pack", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Bundle
// shortname: FBu
// identifier: d5de5684-575b-11df-98c2-858db0535f77

TEST_F(CombinationsTest, Bundle_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"), Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-09-01"), Component::from_string("F 1 2010-12-01"),
        Component::from_string("F 1 2010-03-01"), Component::from_string("F 1 2010-06-01"),
        Component::from_string("F 1 2010-09-01"), Component::from_string("F 1 2010-12-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Bundle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Bundle_reverse) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-12-01"), Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-06-01"), Component::from_string("F 1 2010-03-01"),
        Component::from_string("F 1 2010-12-01"), Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-06-01"), Component::from_string("F 1 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Bundle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Bundle_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("F 1 2010-03-01"), Component::from_string("F 1 2010-03-01"),
        Component::from_string("F 1 2010-06-01"), Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-06-01"), Component::from_string("F 1 2010-09-01"),
        Component::from_string("F 1 2010-12-01"), Component::from_string("F 1 2010-12-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Bundle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Strip
// shortname: FST
// identifier: d54c8d4e-575b-11df-87fa-b18f0b7bb14e

TEST_F(CombinationsTest, Strip_direct) {
    const std::vector<Component> components = {
        Component::from_string("F 10 2010-03-01"), Component::from_string("F 10 2010-03-01"),
        Component::from_string("F 10 2010-03-01"), Component::from_string("F 10 2010-03-01"),
        Component::from_string("F 10 2010-03-01"), Component::from_string("F 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strip", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Jelly roll
// shortname: JR
// identifier: d60e378c-575b-11df-80da-6b7a129b2e6e

TEST_F(CombinationsTest, Jelly_roll_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("P -1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Jelly roll", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Jelly_roll_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2100 2010-03-02"),
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Jelly roll", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Jelly_roll_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-02"),
        Component::from_string("C 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Jelly roll", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call butterfly
// shortname: CB
// identifier: 84326d36-6997-11df-9b02-53278935aea1

TEST_F(CombinationsTest, Call_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call calendar butterfly
// shortname: CCB
// identifier: 9ceef5e0-46c7-11e3-bf68-d7f6eb7a5d73

TEST_F(CombinationsTest, Call_calendar_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-03"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_calendar_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-03"),
        Component::from_string("C -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_calendar_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-03"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Skinny call butterfly
// shortname: SCB
// identifier: d40c4012-4058-11e2-9e42-15f1cfe99412

TEST_F(CombinationsTest, Skinny_call_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Skinny call butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Skinny_call_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Skinny call butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Skinny_call_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Skinny call butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put butterfly
// shortname: PB
// identifier: d6378a10-575b-11df-bf2a-5b9fe7473ef5

TEST_F(CombinationsTest, Put_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put calendar butterfly
// shortname: PCB
// identifier: c53cf542-46c7-11e3-b146-86241b7145b3

TEST_F(CombinationsTest, Put_calendar_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-03"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_calendar_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-03"),
        Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_calendar_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-03"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Skinny put butterfly
// shortname: SPB
// identifier: dfb1af38-4058-11e2-af2c-0b5c4a040bf8

TEST_F(CombinationsTest, Skinny_put_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Skinny put butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Skinny_put_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Skinny put butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Skinny_put_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Skinny put butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call spread
// shortname: CS
// identifier: d66d894e-575b-11df-aa9c-29eced234898

TEST_F(CombinationsTest, Call_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put spread
// shortname: PS
// identifier: d69d460c-575b-11df-b797-6fa7e19cbce1

TEST_F(CombinationsTest, Put_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call calendar spread
// shortname: CCS
// identifier: d6cc31d8-575b-11df-963c-8fda34c8aa53

TEST_F(CombinationsTest, Call_calendar_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_calendar_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put calendar spread
// shortname: PCS
// identifier: d6f9d20a-575b-11df-af6b-fb9ace19a2d8

TEST_F(CombinationsTest, Put_calendar_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_calendar_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call diagonal calendar spread
// shortname: CDCS
// identifier: d7263fe8-575b-11df-84fe-e9326bd90939

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put diagonal calendar spread
// shortname: PDCS
// identifier: d755e2f2-575b-11df-b8c9-cd1412e9dfb2

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Guts
// shortname: G
// identifier: d781b512-575b-11df-ad05-fbcf6661a2b7

TEST_F(CombinationsTest, Guts_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Guts_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x1 ratio call spread
// shortname: RCS
// identifier: d7afce52-575b-11df-ba71-914fffe03048

TEST_F(CombinationsTest, 2x1_ratio_call_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 2 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_call_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x1 ratio call spread
// shortname: RCS 3x1
// identifier: 0a13a7b4-4058-11e2-8b12-3e0a4c21765b

TEST_F(CombinationsTest, 3x1_ratio_call_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x1 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x1_ratio_call_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 3 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x1 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 1x2 ratio call spread
// shortname: RCS 1x2
// identifier: 7fbbea36-24cd-11e2-baf8-9ed5f9fd6e3d

TEST_F(CombinationsTest, 1x2_ratio_call_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x2 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 1x2_ratio_call_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x2 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 1x3 ratio call spread
// shortname: RCS 1x3
// identifier: 861c4e2a-24cd-11e2-923f-492bac84c799

TEST_F(CombinationsTest, 1x3_ratio_call_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x3 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 1x3_ratio_call_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x3 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3 ratio call spread
// shortname: RCS 2x3
// identifier: 86f597fa-71dc-11e2-bda3-c2529fa05804

TEST_F(CombinationsTest, 2x3_ratio_call_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2000 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3_ratio_call_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3 ratio call spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x1 ratio put spread
// shortname: RPS
// identifier: d7da465a-575b-11df-a441-dba311f084fd

TEST_F(CombinationsTest, 2x1_ratio_put_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 2 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_put_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 2 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x1 ratio put spread
// shortname: RPS 3x1
// identifier: 167f3b1c-4058-11e2-8ed4-529af72e765a

TEST_F(CombinationsTest, 3x1_ratio_put_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 3 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x1 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x1_ratio_put_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x1 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 1x2 ratio put spread
// shortname: RPS 1x2
// identifier: ea38fc3c-24cd-11e2-8130-2c7835671f71

TEST_F(CombinationsTest, 1x2_ratio_put_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -2 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x2 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 1x2_ratio_put_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -2 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x2 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 1x3 ratio put spread
// shortname: RPS 1x3
// identifier: ef816c92-24cd-11e2-aebe-28de187c5fa5

TEST_F(CombinationsTest, 1x3_ratio_put_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -3 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x3 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 1x3_ratio_put_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -3 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("1x3 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3 ratio put spread
// shortname: RPS 2x3
// identifier: 88fed8a4-71dc-11e2-a0b9-b301f88ea076

TEST_F(CombinationsTest, 2x3_ratio_put_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 2 2000 2010-03-01"),
        Component::from_string("P -3 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3_ratio_put_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -3 1900 2010-03-01"),
        Component::from_string("P 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3 ratio put spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Iron butterfly
// shortname: IBF
// identifier: d804fab2-575b-11df-b359-23fdb8cf86b0

TEST_F(CombinationsTest, Iron_butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Combo
// shortname: Co
// identifier: d82ff0c8-575b-11df-ab8a-d7326a8338dc

TEST_F(CombinationsTest, Combo_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Combo", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Combo_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 1900 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Combo", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Strangle
// shortname: St
// identifier: d85aae8a-575b-11df-86af-69e65d689981

TEST_F(CombinationsTest, Strangle_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Strangle_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call ladder
// shortname: CL
// identifier: d880ac48-575b-11df-adaa-e9372fb32dad

TEST_F(CombinationsTest, Call_ladder_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_ladder_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_ladder_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put ladder
// shortname: PL
// identifier: d8aef2b0-575b-11df-bace-b5cba375818e

TEST_F(CombinationsTest, Put_ladder_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_ladder_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_ladder_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle calendar spread
// shortname: StrCS
// identifier: d9070acc-575b-11df-84b7-33e39ac3eb9a

TEST_F(CombinationsTest, Straddle_calendar_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_calendar_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_calendar_spread_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Diagonal straddle calendar spread
// shortname: DStrCS
// identifier: da3350fe-575b-11df-adbd-671ceb3b412c

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("C 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle
// shortname: Str
// identifier: da5a2620-575b-11df-9029-c3f2a99ff460

TEST_F(CombinationsTest, Straddle_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call condor
// shortname: CCo
// identifier: da84b6f6-575b-11df-ab0d-3f4d8e5f7559

TEST_F(CombinationsTest, Call_condor_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C 1 2300 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_condor_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2300 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_condor_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2300 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put condor
// shortname: PCo
// identifier: e3f47ac4-69a7-11df-916f-a594496ed53e

TEST_F(CombinationsTest, Put_condor_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P -1 2200 2010-03-01"),
        Component::from_string("P 1 2300 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_condor_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2300 2010-03-01"),
        Component::from_string("P -1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_condor_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2300 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Box
// shortname: B
// identifier: dab04d02-575b-11df-a1d4-f7d8a8a61767

TEST_F(CombinationsTest, Box_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Box", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Box_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Box", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Box_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Box", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Synthetic conversion-reversal
// shortname: Synth C-R
// identifier: db288010-575b-11df-a8e4-b9955814d526

TEST_F(CombinationsTest, Synthetic_conversion_reversal_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Synthetic conversion-reversal", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Synthetic_conversion_reversal_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Synthetic conversion-reversal", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Fence
// shortname: F
// identifier: a1d6ef22-24cc-11e2-bbfb-9d41b819eeb7

TEST_F(CombinationsTest, Fence_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Fence", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Fence_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Fence", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Iron condor
// shortname: ICo
// identifier: db6bcc08-575b-11df-bf3c-cbc75d213591

TEST_F(CombinationsTest, Iron_condor_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("C -1 2300 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_condor_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2300 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_condor_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2300 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call spread vs put
// shortname: 3-Way CSvP
// identifier: dbb48c68-575b-11df-8f60-b7f400367baf

TEST_F(CombinationsTest, Call_spread_vs_put_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs put", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_spread_vs_put_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs put", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_spread_vs_put_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs put", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put spread vs call
// shortname: 3-Way PSvC
// identifier: dc18e0b4-575b-11df-b188-c59feaa63a84

TEST_F(CombinationsTest, Put_spread_vs_call_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs call", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_spread_vs_call_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs call", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_spread_vs_call_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs call", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle vs call
// shortname: 3-Way StrvC
// identifier: dc63a1da-575b-11df-b3d0-b13c7ca30768

TEST_F(CombinationsTest, Straddle_vs_call_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs call", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_call_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs call", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_call_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs call", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle vs put
// shortname: 3-Way StrvP
// identifier: d3f9f50c-575c-11df-b9a9-47a16437aad4

TEST_F(CombinationsTest, Straddle_vs_put_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs put", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_put_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs put", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_put_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs put", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call double
// shortname: CD
// identifier: cc582fde-71db-11e2-9cb8-71dc70e19108

TEST_F(CombinationsTest, Call_double_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call double", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_double_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call double", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put double
// shortname: PD
// identifier: d2827b3a-71db-11e2-827f-7bb96aeed8ce

TEST_F(CombinationsTest, Put_double_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P 1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put double", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_double_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put double", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle strip
// shortname: STrS
// identifier: f3914366-71df-11e2-828e-fa5177e7d318

TEST_F(CombinationsTest, Straddle_strip_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"), Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-06-01"), Component::from_string("P 1 2000 2010-06-01"),
        Component::from_string("C 1 2000 2010-09-01"), Component::from_string("P 1 2000 2010-09-01"),
        Component::from_string("C 1 2000 2010-12-01"), Component::from_string("P 1 2000 2010-12-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-12-01"), Component::from_string("C 1 2000 2010-12-01"),
        Component::from_string("P 1 2000 2010-09-01"), Component::from_string("C 1 2000 2010-09-01"),
        Component::from_string("P 1 2000 2010-06-01"), Component::from_string("C 1 2000 2010-06-01"),
        Component::from_string("P 1 2000 2010-03-01"), Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-09-01"), Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-06-01"), Component::from_string("C 1 2000 2010-06-01"),
        Component::from_string("P 1 2000 2010-09-01"), Component::from_string("P 1 2000 2010-12-01"),
        Component::from_string("C 1 2000 2010-12-01"), Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle strip jumps
// shortname: STrSJ
// identifier: f3914366-71df-11e2-828e-fa5177e7d319

TEST_F(CombinationsTest, Straddle_strip_jumps_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"), Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-03"),  // 2d
        Component::from_string("P 1 2000 2010-04-01"),  // 1m
        Component::from_string("C 1 2000 2010-05-01"),  // 2m
        Component::from_string("P 1 2000 2010-04-30"),  // 60d
        Component::from_string("C 1 2000 2010-12-01"),  // 3q
        Component::from_string("P 1 2000 2013-03-01"),  // 3y
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip jumps", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_jumps_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2013-03-01"), Component::from_string("C 1 2000 2010-12-01"),
        Component::from_string("P 1 2000 2010-04-30"), Component::from_string("C 1 2000 2010-05-01"),
        Component::from_string("P 1 2000 2010-04-01"), Component::from_string("C 1 2000 2010-03-03"),
        Component::from_string("P 1 2000 2010-03-01"), Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip jumps", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_jumps_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"), Component::from_string("P 1 2000 2013-03-01"),
        Component::from_string("C 1 2000 2010-05-01"), Component::from_string("C 1 2000 2010-03-03"),
        Component::from_string("C 1 2000 2010-12-01"), Component::from_string("P 1 2000 2010-04-01"),
        Component::from_string("P 1 2000 2010-04-30"), Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip jumps", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_jumps_leap_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 1999-12-31"), Component::from_string("P 1 2000 1999-12-31"),
        Component::from_string("C 1 2000 2000-01-02"),  // 2d
        Component::from_string("P 1 2000 2000-01-31"),  // 1m
        Component::from_string("C 1 2000 2000-03-02"),  // 2m
        Component::from_string("P 1 2000 2000-02-29"),  // 60d
        Component::from_string("C 1 2000 2000-10-01"),  // 3q
        Component::from_string("P 1 2000 2002-12-31"),  // 3y
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip jumps", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_jumps_leap_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2002-12-31"), Component::from_string("C 1 2000 2000-10-01"),
        Component::from_string("P 1 2000 2000-02-29"), Component::from_string("C 1 2000 2000-03-02"),
        Component::from_string("P 1 2000 2000-01-31"), Component::from_string("C 1 2000 2000-01-02"),
        Component::from_string("P 1 2000 1999-12-31"), Component::from_string("C 1 2000 1999-12-31"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip jumps", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_jumps_leap_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2000-02-29"), Component::from_string("C 1 2000 2000-01-02"),
        Component::from_string("P 1 2000 2000-01-31"), Component::from_string("C 1 2000 2000-10-01"),
        Component::from_string("P 1 2000 2002-12-31"), Component::from_string("C 1 2000 2000-03-02"),
        Component::from_string("C 1 2000 1999-12-31"), Component::from_string("P 1 2000 1999-12-31"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle strip jumps", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_strip_jumps_leap_fail) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 1999-12-31"), Component::from_string("P 1 2000 1999-12-31"),
        Component::from_string("C 1 2000 2000-01-02"), Component::from_string("P 1 2000 2000-01-31"),
        Component::from_string("C 1 2000 2000-03-02"), Component::from_string("P 1 2000 2000-03-01"),
        Component::from_string("C 1 2000 2000-10-01"), Component::from_string("P 1 2000 2002-12-31"),
    };
    std::vector<int> order;
    ASSERT_EQ("Options strip", combinations().classify(components, order));  // not "Straddle strip jumps"
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Options strip
// shortname: OS
// identifier: d8da1af8-575b-11df-b293-75f47d9296ce

TEST_F(CombinationsTest, Options_strip_direct) {
    const std::vector<Component> components = {
        Component::from_string("O 1 2000 2010-03-01"), Component::from_string("O 1 2000 2010-03-01"),
        Component::from_string("O 1 2000 2010-03-01"), Component::from_string("O 1 2000 2010-03-01"),
        Component::from_string("O 1 2000 2010-03-01"), Component::from_string("O 1 2000 2010-03-01"),
        Component::from_string("O 1 2000 2010-03-01"), Component::from_string("O 1 2000 2010-03-01"),
        Component::from_string("O 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Options strip", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3x2 Ratio Call Butterfly
// shortname: CB232
// identifier: 52b15904-d60f-11e9-9e70-d35f11fcf204

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2000 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("C 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3x2 Ratio Put Butterfly
// shortname: PB232
// identifier: 5d24103e-d60f-11e9-a780-a754448b24e1

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 2 2000 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 2 2200 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
        Component::from_string("P 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -3 2100 2010-03-01"),
        Component::from_string("P 2 2000 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x2 Ratio Put Spread
// shortname: BR23
// identifier: ac3619d8-d60f-11e9-9940-933356d52e71

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -2 2000 2010-03-01"),
        Component::from_string("P 3 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("P -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x2 Ratio Call Spread
// shortname: BU23
// identifier: e8943bee-d60f-11e9-a130-234d3ce1fd19

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -2 2000 2010-03-01"),
        Component::from_string("C 3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 3 2100 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle Fly
// shortname: STDF
// identifier: 17e24ec2-d610-11e9-8900-fdfa41561f51

TEST_F(CombinationsTest, Straddle_Fly_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-02"), Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-03"),  Component::from_string("P 1 2000 2010-03-03"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_Fly_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-03"),  Component::from_string("C 1 2000 2010-03-03"),
        Component::from_string("P -2 2000 2010-03-02"), Component::from_string("C -2 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-01"),  Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_Fly_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-03"), Component::from_string("C -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-01"), Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-03"), Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Risky Swap
// shortname: RSWP
// identifier: 73f8df20-d613-11e9-acc0-25607308d734

TEST_F(CombinationsTest, Risky_Swap_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("C -1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Risky_Swap_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Risky_Swap_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call Spread Swap
// shortname: BUSWP
// identifier: 57d57174-d617-11e9-9270-f28fc5476b8b

TEST_F(CombinationsTest, Call_Spread_Swap_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-02"),
        Component::from_string("C 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_Spread_Swap_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-02"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_Spread_Swap_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put Spread Swap
// shortname: BRSWP
// identifier: d2a7717c-d617-11e9-8ac0-78bbde1ae1ee

TEST_F(CombinationsTest, Put_Spread_Swap_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-02"),
        Component::from_string("P 1 1900 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_Spread_Swap_reverse) {
    const std::vector<Component> components = {
        Component::from_string("P 1 1900 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-02"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_Spread_Swap_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 1900 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle swap
// shortname: StrSW
// identifier: 0dccbf54-5288-11ea-b500-646921a68401

TEST_F(CombinationsTest, Straddle_swap_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_swap_reverse) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_swap_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle swap", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call volatility trade
// shortname: CVolT
// identifier: d4705472-575c-11df-a24a-19059ce82a41

TEST_F(CombinationsTest, Call_volatility_trade_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call volatility trade", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_volatility_trade_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call volatility trade", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put volatility trade
// shortname: PVol
// identifier: d4e254f0-575c-11df-977e-a3da1431391a

TEST_F(CombinationsTest, Put_volatility_trade_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put volatility trade", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_volatility_trade_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put volatility trade", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call spread vs underlying
// shortname: CSvU
// identifier: d552985a-575c-11df-a6fb-a5d0d3804e38

TEST_F(CombinationsTest, Call_spread_vs_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_spread_vs_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_spread_vs_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put spread vs underlying
// shortname: PSvU
// identifier: d5c00eb2-575c-11df-8b6a-b10cf42a451d

TEST_F(CombinationsTest, Put_spread_vs_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_spread_vs_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_spread_vs_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle vs buy underlying
// shortname: StvbU
// identifier: d6324220-575c-11df-9a29-0b66d65515d6

TEST_F(CombinationsTest, Straddle_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle vs sell underlying
// shortname: StvsU
// identifier: d6a0fbde-575c-11df-aa15-6bb4d6d97a84

TEST_F(CombinationsTest, Straddle_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Strangle vs buy underlying
// shortname: StrvbU
// identifier: d70da4aa-575c-11df-bcd2-a73c367931cb

TEST_F(CombinationsTest, Strangle_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Strangle_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Strangle_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Strangle vs sell underlying
// shortname: StrvsU
// identifier: d76e8d74-575c-11df-b24c-a323e0ba1e89

TEST_F(CombinationsTest, Strangle_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Strangle_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Strangle_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Strangle vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call spread vs sell put vs underlying
// shortname: CSvPvU
// identifier: d7dc15d8-575c-11df-905d-ed9e24470620

TEST_F(CombinationsTest, Call_spread_vs_sell_put_vs_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs sell put vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_spread_vs_sell_put_vs_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs sell put vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_spread_vs_sell_put_vs_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call spread vs sell put vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put spread vs sell call vs underlying
// shortname: PSvCvU
// identifier: d84e355a-575c-11df-b763-d308db7c0f96

TEST_F(CombinationsTest, Put_spread_vs_sell_call_vs_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs sell call vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_spread_vs_sell_call_vs_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs sell call vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_spread_vs_sell_call_vs_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put spread vs sell call vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call ladder vs buy underlying
// shortname: CLvbU
// identifier: d8ba1626-575c-11df-9fb7-05b72b6fedcf

TEST_F(CombinationsTest, Call_ladder_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_ladder_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_ladder_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call ladder vs sell underlying
// shortname: CLvsU
// identifier: d92c9ac0-575c-11df-be27-e3e980864d7c

TEST_F(CombinationsTest, Call_ladder_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_ladder_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_ladder_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call ladder vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put ladder vs buy underlying
// shortname: PLvbU
// identifier: d9998900-575c-11df-80a5-950ba54fb119

TEST_F(CombinationsTest, Put_ladder_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_ladder_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_ladder_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put ladder vs sell underlying
// shortname: PLvsU
// identifier: da01df8c-575c-11df-9360-7bcf62c9aac2

TEST_F(CombinationsTest, Put_ladder_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_ladder_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_ladder_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put ladder vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Combo vs underlying
// shortname: CvU
// identifier: da6e1b98-575c-11df-853b-5ddaf9880ab5

TEST_F(CombinationsTest, Combo_vs_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 1900 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Combo vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Combo_vs_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 1900 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Combo vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Combo_vs_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Combo vs underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call calendar spread vs buy underlying
// shortname: CCSvbU
// identifier: dade3e32-575c-11df-adcf-ad645cef97af

TEST_F(CombinationsTest, Call_calendar_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_calendar_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_calendar_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call calendar spread vs sell underlying
// shortname: CCSvsU
// identifier: db41619c-575c-11df-b741-3f32ccccbb26

TEST_F(CombinationsTest, Call_calendar_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_calendar_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_calendar_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put calendar spread vs buy underlying
// shortname: PCSvbU
// identifier: dba8dfe8-575c-11df-bf95-dde76e6420b7

TEST_F(CombinationsTest, Put_calendar_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_calendar_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_calendar_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put calendar spread vs sell underlying
// shortname: PCSvsU
// identifier: 48600454-575d-11df-bae4-0f52c8c94937

TEST_F(CombinationsTest, Put_calendar_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_calendar_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_calendar_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x1 ratio call spread vs buy underlying
// shortname: RCSvbU
// identifier: 48d32c68-575d-11df-82b5-c3ec844b4544

TEST_F(CombinationsTest, 2x1_ratio_call_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 2 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_call_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 2 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_call_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x1 ratio call spread vs sell underlying
// shortname: RCSvsU
// identifier: 494ec292-575d-11df-a267-e96a655a2a68

TEST_F(CombinationsTest, 2x1_ratio_call_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 2 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_call_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 2 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_call_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio call spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x1 ratio put spread vs buy underlying
// shortname: RPSvbU
// identifier: 49cee7f6-575d-11df-b77e-c789229c6ec7

TEST_F(CombinationsTest, 2x1_ratio_put_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 2 1900 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_put_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 2 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_put_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 2 1900 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x1 ratio put spread vs sell underlying
// shortname: RPSvsU
// identifier: 4a416a42-575d-11df-9979-0741afb5dcb2

TEST_F(CombinationsTest, 2x1_ratio_put_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 2 1900 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_put_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 2 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x1_ratio_put_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 2 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x1 ratio put spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Conversion-reversal
// shortname: C-R
// identifier: 4aad0950-575d-11df-8e7d-213126f37438

TEST_F(CombinationsTest, Conversion_reversal_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Conversion-reversal", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Conversion_reversal_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Conversion-reversal", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Conversion_reversal_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Conversion-reversal", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle calendar spread vs buy underlying
// shortname: StrCSvbU
// identifier: 079e135a-673f-11e0-b848-56e685aa5647

TEST_F(CombinationsTest, Straddle_calendar_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_calendar_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_calendar_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"), Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle calendar spread vs sell underlying
// shortname: StrCSvsU
// identifier: 08601626-673f-11e0-9806-b13b3be4d7e2

TEST_F(CombinationsTest, Straddle_calendar_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_calendar_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C 1 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_calendar_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-02"),  Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call diagonal calendar spread vs buy underlying
// shortname: CDCSvbU
// identifier: 9b61ec14-673b-11e0-9223-cbb2491a61f6

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call diagonal calendar spread vs sell underlying
// shortname: CDCSvsU
// identifier: a2a3f710-673b-11e0-a30e-b5fdd2349174

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_diagonal_calendar_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call diagonal calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put diagonal calendar spread vs buy underlying
// shortname: PDCSvbU
// identifier: a7255810-673b-11e0-a9ae-45e719a6a710

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put diagonal calendar spread vs sell underlying
// shortname: PDCSvsU
// identifier: ac4f7bfe-673b-11e0-86f3-65f344c77f11

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_diagonal_calendar_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put diagonal calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Guts vs buy underlying
// shortname: GvbU
// identifier: edd98306-673d-11e0-bf2a-3b59618df41b

TEST_F(CombinationsTest, Guts_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Guts_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Guts_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Guts vs sell underlying
// shortname: GvsU
// identifier: f6a0cb8e-673d-11e0-9d83-9477b038971f

TEST_F(CombinationsTest, Guts_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Guts_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Guts_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Guts vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call butterfly vs buy underlying
// shortname: CBvbU
// identifier: b0d59244-673b-11e0-a8bc-19778dfc8309

TEST_F(CombinationsTest, Call_butterfly_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_butterfly_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_butterfly_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call butterfly vs sell underlying
// shortname: CBvsU
// identifier: b5eb345a-673b-11e0-95d5-348e71aab397

TEST_F(CombinationsTest, Call_butterfly_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_butterfly_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_butterfly_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -2 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put butterfly vs buy underlying
// shortname: PBvbU
// identifier: badea97e-673b-11e0-9ae3-1559b2748b71

TEST_F(CombinationsTest, Put_butterfly_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_butterfly_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_butterfly_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put butterfly vs sell underlying
// shortname: PBvsU
// identifier: c168bb22-673b-11e0-ab27-9a0350901e03

TEST_F(CombinationsTest, Put_butterfly_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_butterfly_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_butterfly_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2200 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -2 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Iron butterfly vs buy underlying
// shortname: IBFvbU
// identifier: c609e67e-673b-11e0-a189-7ec6e50244a3

TEST_F(CombinationsTest, Iron_butterfly_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),  Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_butterfly_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),  Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_butterfly_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),  Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Iron butterfly vs sell underlying
// shortname: IBFvsU
// identifier: cc3bf30c-673b-11e0-b75f-caaf290173d9

TEST_F(CombinationsTest, Iron_butterfly_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),  Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_butterfly_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C -1 2200 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-01"),  Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_butterfly_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2100 2010-03-01"),  Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"), Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron butterfly vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Diagonal straddle calendar spread vs buy underlying
// shortname: DStrCSvbU
// identifier: d10f4456-673b-11e0-987c-5a229ee213ad

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),  Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("P 1 2100 2010-03-02"),  Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-01"), Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),      Component::from_string("P 1 2100 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Diagonal straddle calendar spread vs sell underlying
// shortname: DStrCSvsU
// identifier: d6d85d00-673b-11e0-bb1a-2eff4ae226b1

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P 1 2100 2010-03-02"),  Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("P 1 2100 2010-03-02"),  Component::from_string("C -1 2000 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Diagonal_straddle_calendar_spread_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2100 2010-03-02"),  Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-01"), Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Diagonal straddle calendar spread vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call condor vs buy underlying
// shortname: CCovbU
// identifier: db73a7ac-673b-11e0-b9eb-da4e94ef7e0c

TEST_F(CombinationsTest, Call_condor_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"), Component::from_string("C 1 2300 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_condor_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C 1 2300 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"), Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_condor_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"), Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2300 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call condor vs sell underlying
// shortname: CCovsU
// identifier: e02e4a2c-673b-11e0-9019-0990e42c840c

TEST_F(CombinationsTest, Call_condor_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"), Component::from_string("C 1 2300 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_condor_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C 1 2300 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"), Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_condor_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("C 1 2300 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2200 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put condor vs buy underlying
// shortname: PCovbU
// identifier: e62fe4e4-673b-11e0-b393-5a8af0edac7a

TEST_F(CombinationsTest, Put_condor_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),  Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P -1 2200 2010-03-01"), Component::from_string("P 1 2300 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_condor_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("P 1 2300 2010-03-01"),
        Component::from_string("P -1 2200 2010-03-01"), Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_condor_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("P -1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"), Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P 1 2300 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put condor vs sell underlying
// shortname: PCovsU
// identifier: eb859826-673b-11e0-ba21-4ba6b29bd06e

TEST_F(CombinationsTest, Put_condor_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),  Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P -1 2200 2010-03-01"), Component::from_string("P 1 2300 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_condor_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("P 1 2300 2010-03-01"),
        Component::from_string("P -1 2200 2010-03-01"), Component::from_string("P -1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_condor_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("P -1 2200 2010-03-01"),
        Component::from_string("P -1 2100 2010-03-01"), Component::from_string("P 1 2300 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Iron condor vs buy underlying
// shortname: ICovbU
// identifier: efe6690e-673b-11e0-8702-7bb4a871b929

TEST_F(CombinationsTest, Iron_condor_vs_buy_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),  Component::from_string("C -1 2300 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_condor_vs_buy_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C -1 2300 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),  Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_condor_vs_buy_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2100 2010-03-01"), Component::from_string("C -1 2300 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"), Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor vs buy underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Iron condor vs sell underlying
// shortname: ICovsU
// identifier: f3f6992e-673b-11e0-b642-b69bcabc0bed

TEST_F(CombinationsTest, Iron_condor_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),  Component::from_string("C -1 2300 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_condor_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C -1 2300 2010-03-01"),
        Component::from_string("C 1 2200 2010-03-01"),  Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Iron_condor_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2200 2010-03-01"),  Component::from_string("U -10 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("P 1 2100 2010-03-01"),
        Component::from_string("C -1 2300 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Iron condor vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Fence vs sell underlying
// shortname: FvsU
// identifier: e6872a50-7e79-11e6-91d0-753ff5baa8fb

TEST_F(CombinationsTest, Fence_vs_sell_underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Fence vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Fence_vs_sell_underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Fence vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Fence_vs_sell_underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Fence vs sell underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle Fly versus Long Underlying
// shortname: STDF+U
// identifier: 83b38596-d618-11e9-af50-b55ffde069e4

TEST_F(CombinationsTest, Straddle_Fly_versus_Long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-02"), Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-03"),  Component::from_string("P 1 2000 2010-03-03"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_Fly_versus_Long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("P 1 2000 2010-03-03"),
        Component::from_string("C 1 2000 2010-03-03"),  Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("C -2 2000 2010-03-02"), Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_Fly_versus_Long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-03"),  Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P -2 2000 2010-03-02"), Component::from_string("C -2 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-03"),  Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Straddle Fly versus Short Underlying
// shortname: STDF-U
// identifier: 1d1018f8-d619-11e9-81b0-e020efedb2bf

TEST_F(CombinationsTest, Straddle_Fly_versus_Short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-02"), Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("C 1 2000 2010-03-03"),  Component::from_string("P 1 2000 2010-03-03"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_Fly_versus_Short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("P 1 2000 2010-03-03"),
        Component::from_string("C 1 2000 2010-03-03"),  Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("C -2 2000 2010-03-02"), Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Straddle_Fly_versus_Short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -2 2000 2010-03-02"), Component::from_string("P 1 2000 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-03"),  Component::from_string("P -2 2000 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C 1 2000 2010-03-03"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Straddle Fly versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3x2 Ratio Call Butterfly versus long Underlying
// shortname: CB232+U
// identifier: 27fe01d4-d61a-11e9-afa0-1498443d0605

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_versus_long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2000 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly versus long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_versus_long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly versus long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_versus_long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 2 2000 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly versus long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3x2 Ratio Call Butterfly versus short Underlying
// shortname: CB232-U
// identifier: 9f45ae9a-d61a-11e9-89d0-9172dade6fde

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_versus_short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 2 2000 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly versus short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_versus_short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly versus short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Call_Butterfly_versus_short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -3 2100 2010-03-01"),
        Component::from_string("C 2 2200 2010-03-01"),
        Component::from_string("C 2 2000 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Call Butterfly versus short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3x2 Ratio Put Butterfly versus Long Underlying
// shortname: PB232+U
// identifier: a63ff2d2-d61a-11e9-aa80-98618ff12710

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_versus_Long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 2 2000 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_versus_Long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
        Component::from_string("P 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_versus_Long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 2 2000 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 2x3x2 Ratio Put Butterfly versus Short Underlying
// shortname: PB232-U
// identifier: ac005dec-d61a-11e9-9dd0-4da497e46877

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_versus_Short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 2 2000 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_versus_Short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
        Component::from_string("P 2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 2x3x2_Ratio_Put_Butterfly_versus_Short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 2 2200 2010-03-01"),
        Component::from_string("P 2 2000 2010-03-01"),
        Component::from_string("P -3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("2x3x2 Ratio Put Butterfly versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x2 Ratio Put Spread Long Underlying
// shortname: BR23+U
// identifier: afe04daa-d61a-11e9-9b70-6f7cbaf4641e

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_Long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -2 2000 2010-03-01"),
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_Long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("P -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_Long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("P -2 2000 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x2 Ratio Put Spread Short Underlying
// shortname: BR23-U
// identifier: b4a5e58e-d61a-11e9-93d0-0c166ee87d94

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_Short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -2 2000 2010-03-01"),
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_Short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("P -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Put_Spread_Short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 3 1900 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("P -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Put Spread Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x2 Ratio Call Spread Long Underlying
// shortname: BU23+U
// identifier: ba02d7f8-d61a-11e9-b240-5c348771993e

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_Long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -2 2000 2010-03-01"),
        Component::from_string("C 3 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_Long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C 3 2100 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_Long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 3 2100 2010-03-01"),
        Component::from_string("U 10 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: 3x2 Ratio Call Spread Short Underlying
// shortname: BU23-U
// identifier: c3a5d59e-d61a-11e9-95f0-83b04df63725

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_Short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C -2 2000 2010-03-01"),
        Component::from_string("C 3 2100 2010-03-01"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_Short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 3 2100 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, 3x2_Ratio_Call_Spread_Short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),
        Component::from_string("C -2 2000 2010-03-01"),
        Component::from_string("C 3 2100 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("3x2 Ratio Call Spread Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call Spread Swap versus Long Underlying
// shortname: BUSWP+U
// identifier: c8a07266-d61a-11e9-bf60-890a9da8041e

TEST_F(CombinationsTest, Call_Spread_Swap_versus_Long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-02"), Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_Spread_Swap_versus_Long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-02"), Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_Spread_Swap_versus_Long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2000 2010-03-02"), Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Call Spread Swap versus Short Underlying
// shortname: BUSWP-U
// identifier: cd08d7da-d61a-11e9-9860-65c755a35707

TEST_F(CombinationsTest, Call_Spread_Swap_versus_Short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2000 2010-03-01"),  Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-02"), Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_Spread_Swap_versus_Short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C 1 2100 2010-03-02"),
        Component::from_string("C -1 2000 2010-03-02"), Component::from_string("C -1 2100 2010-03-01"),
        Component::from_string("C 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Call_Spread_Swap_versus_Short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-01"), Component::from_string("U -10 2010-03-01"),
        Component::from_string("C 1 2100 2010-03-02"),  Component::from_string("C 1 2000 2010-03-01"),
        Component::from_string("C -1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Call Spread Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put Spread Swap versus Long Underlying
// shortname: BRSWP+U
// identifier: d0ddec60-d61a-11e9-b460-6afa587a7212

TEST_F(CombinationsTest, Put_Spread_Swap_versus_Long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),  Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-02"), Component::from_string("P 1 1900 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_Spread_Swap_versus_Long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("P 1 1900 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-02"), Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_Spread_Swap_versus_Long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("P 1 1900 2010-03-02"),  Component::from_string("U 10 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-02"), Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Put Spread Swap versus Short Underlying
// shortname: BRSWP-U
// identifier: d5484354-d61a-11e9-89b0-3205afbe756c

TEST_F(CombinationsTest, Put_Spread_Swap_versus_Short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P 1 2000 2010-03-01"),  Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-02"), Component::from_string("P 1 1900 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_Spread_Swap_versus_Short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("P 1 1900 2010-03-02"),
        Component::from_string("P -1 2000 2010-03-02"), Component::from_string("P -1 1900 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Put_Spread_Swap_versus_Short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("P -1 2000 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-01"),  Component::from_string("P 1 1900 2010-03-02"),
        Component::from_string("P -1 1900 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Put Spread Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Risky Swap versus Long Underlying
// shortname: RSWP+U
// identifier: d97055b6-d61a-11e9-ac30-8b3686a5bca2

TEST_F(CombinationsTest, Risky_Swap_versus_Long_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C -1 2100 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Risky_Swap_versus_Long_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U 10 2010-03-01"),      Component::from_string("C -1 2100 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Risky_Swap_versus_Long_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C 1 2100 2010-03-01"),  Component::from_string("P 1 2000 2010-03-02"),
        Component::from_string("U 10 2010-03-01"),      Component::from_string("P -1 2000 2010-03-01"),
        Component::from_string("C -1 2100 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap versus Long Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

// name: Risky Swap versus Short Underlying
// shortname: RSWP-U
// identifier: df635d7e-d61a-11e9-b210-e1d79e1bd2d8

TEST_F(CombinationsTest, Risky_Swap_versus_Short_Underlying_direct) {
    const std::vector<Component> components = {
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C -1 2100 2010-03-02"),
        Component::from_string("U -10 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Risky_Swap_versus_Short_Underlying_reverse) {
    const std::vector<Component> components = {
        Component::from_string("U -10 2010-03-01"),     Component::from_string("C -1 2100 2010-03-02"),
        Component::from_string("P 1 2000 2010-03-02"),  Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

TEST_F(CombinationsTest, Risky_Swap_versus_Short_Underlying_shuffle) {
    const std::vector<Component> components = {
        Component::from_string("C -1 2100 2010-03-02"), Component::from_string("C 1 2100 2010-03-01"),
        Component::from_string("P -1 2000 2010-03-01"), Component::from_string("U -10 2010-03-01"),
        Component::from_string("P 1 2000 2010-03-02"),
    };
    std::vector<int> order;
    ASSERT_EQ("Risky Swap versus Short Underlying", combinations().classify(components, order));
    ASSERT_EQ(components.size(), order.size());
    ASSERT_TRUE(check_order_basic(order));
}

}  // anonymous namespace
