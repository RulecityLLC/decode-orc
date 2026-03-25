/*
 * File:        burst_level_analysis_sink_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for BurstLevelAnalysisSinkStage contracts and trigger behavior
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>

#include "../../include/video_field_representation_mock.h"
#include "../../include/observation_context_interface_mock.h"
#include "../../../../orc/core/include/observation_context.h"
#include "../../../../orc/core/stages/burst_level_analysis_sink/burst_level_analysis_sink_stage.h"

namespace orc_unit_test
{
    using testing::NiceMock;
    using testing::Return;

    TEST(BurstLevelAnalysisSinkStageTest, descriptorDefaults_includeOutputAndWriteCsv)
    {
        orc::BurstLevelAnalysisSinkStage stage;
        const auto descriptors = stage.get_parameter_descriptors();

        auto output_it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "output_path";
        });
        auto write_csv_it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "write_csv";
        });

        ASSERT_NE(output_it, descriptors.end());
        EXPECT_EQ(output_it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(std::get<std::string>(*output_it->constraints.default_value), "");

        ASSERT_NE(write_csv_it, descriptors.end());
        EXPECT_EQ(write_csv_it->type, orc::ParameterType::BOOL);
        EXPECT_FALSE(std::get<bool>(*write_csv_it->constraints.default_value));
    }

    TEST(BurstLevelAnalysisSinkStageTest, triggerFails_whenNoInputProvided)
    {
        orc::BurstLevelAnalysisSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({}, {}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: No input connected");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }

    TEST(BurstLevelAnalysisSinkStageTest, triggerSucceeds_whenInputRangeIsEmpty)
    {
        orc::BurstLevelAnalysisSinkStage stage;
        orc::ObservationContext observation_context;
        auto vfr = std::make_shared<NiceMock<MockVideoFieldRepresentation>>();

        EXPECT_CALL(*vfr, field_range()).WillOnce(Return(orc::FieldIDRange(orc::FieldID(0), orc::FieldID(0))));

        const bool result = stage.trigger({vfr}, {}, observation_context);

        EXPECT_TRUE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Burst level analysis complete");
        EXPECT_TRUE(stage.has_results());
        EXPECT_EQ(stage.total_frames(), 0);
        EXPECT_TRUE(stage.frame_stats().empty());
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }
}
