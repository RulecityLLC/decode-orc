/*
 * File:        ld_sink_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for LDSinkStage parameter contracts and trigger validation
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>

#include <algorithm>

#include "../../include/observation_context_interface_mock.h"
#include "../../../../orc/core/stages/ld_sink/ld_sink_stage.h"

namespace orc_unit_test
{
    TEST(LDSinkStageTest, descriptorDefaults_outputPathIsEmptyTbc)
    {
        orc::LDSinkStage stage;
        const auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "output_path";
        });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".tbc");
        ASSERT_TRUE(it->constraints.default_value.has_value());
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");

        const auto params = stage.get_parameters();
        ASSERT_TRUE(params.find("output_path") != params.end());
        EXPECT_EQ(std::get<std::string>(params.at("output_path")), "");
    }

    TEST(LDSinkStageTest, triggerFails_whenOutputPathMissing)
    {
        orc::LDSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({}, {}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: No output path specified");
    }

    TEST(LDSinkStageTest, triggerFails_whenOutputPathEmpty)
    {
        orc::LDSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({}, {{"output_path", std::string("")}}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: Output path is empty");
    }

    TEST(LDSinkStageTest, triggerFails_whenInputMissingOrWrongType)
    {
        orc::LDSinkStage stage;
        MockObservationContext observation_context;

        EXPECT_FALSE(stage.trigger({}, {{"output_path", std::string("out")}}, observation_context));
        EXPECT_EQ(stage.get_trigger_status(), "Error: No input connected");

        EXPECT_FALSE(stage.trigger({nullptr}, {{"output_path", std::string("out")}}, observation_context));
        EXPECT_EQ(stage.get_trigger_status(), "Error: Input is not a video field representation");
    }
}
