/*
 * File:        hackdac_sink_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for HackdacSinkStage parameter contracts and trigger validation
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>

#include "../../include/video_field_representation_mock.h"
#include "../../include/observation_context_interface_mock.h"
#include "../../../../orc/core/stages/hackdac_sink/hackdac_sink_stage.h"

namespace orc_unit_test
{
    using testing::NiceMock;

    TEST(HackdacSinkStageTest, descriptorDefaults_outputPathIsEmptyHdac)
    {
        orc::HackdacSinkStage stage;
        const auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "output_path";
        });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".hdac");
        ASSERT_TRUE(it->constraints.default_value.has_value());
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
        EXPECT_TRUE(it->constraints.required);
    }

    TEST(HackdacSinkStageTest, triggerFails_whenNoInputProvided)
    {
        orc::HackdacSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({}, {{"output_path", std::string("out.hdac")}}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: No input connected");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }

    TEST(HackdacSinkStageTest, triggerFails_whenInputIsNotVideoFieldRepresentation)
    {
        orc::HackdacSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({nullptr}, {{"output_path", std::string("out.hdac")}}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: Input is not a VideoFieldRepresentation");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }

    TEST(HackdacSinkStageTest, triggerFails_whenOutputPathMissing)
    {
        orc::HackdacSinkStage stage;
        MockObservationContext observation_context;
        auto vfr = std::make_shared<NiceMock<MockVideoFieldRepresentation>>();

        const bool result = stage.trigger({vfr}, {}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: output_path parameter is required and must be a string");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }
}
