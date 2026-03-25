/*
 * File:        efm_sink_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for EFMSinkStage parameter contracts and trigger validation
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>

#include "../../include/video_field_representation_mock.h"
#include "../../include/observation_context_interface_mock.h"
#include "../../../../orc/core/stages/efm_sink/efm_sink_stage.h"

namespace orc_unit_test
{
    using testing::NiceMock;
    using testing::Return;

    TEST(EFMSinkStageTest, descriptorDefaults_includeOutputPathAndDecodeMode)
    {
        orc::EFMSinkStage stage;
        const auto descriptors = stage.get_parameter_descriptors();

        auto output_it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "output_path";
        });
        auto mode_it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "decode_mode";
        });

        ASSERT_NE(output_it, descriptors.end());
        EXPECT_EQ(output_it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(output_it->file_extension_hint, ".wav");
        EXPECT_EQ(std::get<std::string>(*output_it->constraints.default_value), "");

        ASSERT_NE(mode_it, descriptors.end());
        EXPECT_EQ(mode_it->type, orc::ParameterType::STRING);
        EXPECT_EQ(std::get<std::string>(*mode_it->constraints.default_value), "audio");
        EXPECT_EQ(mode_it->constraints.allowed_strings.size(), 2u);
    }

    TEST(EFMSinkStageTest, triggerFails_whenNoInputProvided)
    {
        orc::EFMSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({}, {}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: EFMSink requires one input (VideoFieldRepresentation)");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }

    TEST(EFMSinkStageTest, triggerFails_whenInputHasNoEfm)
    {
        orc::EFMSinkStage stage;
        MockObservationContext observation_context;
        auto vfr = std::make_shared<NiceMock<MockVideoFieldRepresentation>>();

        EXPECT_CALL(*vfr, has_efm()).WillOnce(Return(false));

        const bool result = stage.trigger({vfr}, {{"output_path", std::string("out.bin")}}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: EFMSink: input VFR has no EFM data (no EFM file in source?)");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }

    TEST(EFMSinkStageTest, triggerFails_whenOutputPathMissing)
    {
        orc::EFMSinkStage stage;
        MockObservationContext observation_context;
        auto vfr = std::make_shared<NiceMock<MockVideoFieldRepresentation>>();

        EXPECT_CALL(*vfr, has_efm()).WillOnce(Return(true));

        const bool result = stage.trigger({vfr}, {}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: EFMSink: output_path parameter is required");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }
}
