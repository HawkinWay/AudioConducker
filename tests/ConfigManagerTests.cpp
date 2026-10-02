#include <gtest/gtest.h>

#include "AudioConducker/core/ConfigManager.h"

using namespace AudioConducker;

TEST(ConfigManagerTests, ClampAttackTime){
    ConfigManager config;
    config.setAttackTime(-0.5f);
    EXPECT_FLOAT_EQ(config.getAttackTime(), 0.005f);
    config.setAttackTime(100.f);
    EXPECT_FLOAT_EQ(config.getAttackTime(), 0.5f);
}

TEST(ConfigManagerTests, DuckAmountRange){
    ConfigManager config;
    config.setDuckAmount(120.f);
    EXPECT_FLOAT_EQ(config.getDuckAmount(), 100.f);
    config.setDuckAmount(-20.f);
    EXPECT_FLOAT_EQ(config.getDuckAmount(), 0.f);
}