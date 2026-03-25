/*
 * File:        cc_sink_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for CCSinkStage parameter contracts and trigger validation
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>

#include <algorithm>

#include "../../include/observation_context_interface_mock.h"
#include "../../../../orc/core/stages/cc_sink/cc_sink_stage.h"

namespace orc_unit_test
{
    TEST(CCSinkStageTest, descriptorDefaults_includeExpectedOutputAndFormat)
    {
        orc::CCSinkStage stage;
        const auto descriptors = stage.get_parameter_descriptors();

        auto output_it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "output_path";
        });
        auto format_it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "format";
        });

        ASSERT_NE(output_it, descriptors.end());
        EXPECT_EQ(output_it->type, orc::ParameterType::FILE_PATH);
        ASSERT_TRUE(output_it->constraints.default_value.has_value());
        EXPECT_EQ(std::get<std::string>(*output_it->constraints.default_value), "");

        ASSERT_NE(format_it, descriptors.end());
        EXPECT_EQ(format_it->type, orc::ParameterType::STRING);
        EXPECT_EQ(format_it->constraints.allowed_strings.size(), 2u);
        EXPECT_EQ(std::get<std::string>(*format_it->constraints.default_value), "Scenarist SCC");
    }

    TEST(CCSinkStageTest, triggerStatus_isIdleWhenNotProcessing)
    {
        orc::CCSinkStage stage;
        EXPECT_EQ(stage.get_trigger_status(), "Idle");
    }

    TEST(CCSinkStageTest, triggerFails_whenNoInputProvided)
    {
        orc::CCSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({}, {}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_FALSE(stage.is_trigger_in_progress());
        EXPECT_EQ(stage.get_trigger_status(), "Idle");
    }

    TEST(CCSinkStageTest, triggerFails_whenInputIsNotVideoFieldRepresentation)
    {
        orc::CCSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({nullptr}, {{"output_path", std::string("out.scc")}}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_FALSE(stage.is_trigger_in_progress());
        EXPECT_EQ(stage.get_trigger_status(), "Idle");
    }
}
