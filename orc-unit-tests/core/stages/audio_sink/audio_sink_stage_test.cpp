/*
 * File:        audio_sink_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for AudioSinkStage parameter contracts and trigger behavior
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>

#include "../../include/video_field_representation_mock.h"
#include "../../include/observation_context_interface_mock.h"
#include "../../../../orc/core/stages/audio_sink/audio_sink_stage.h"

namespace orc_unit_test
{
    using testing::NiceMock;
    using testing::Return;

    TEST(AudioSinkStageTest, stageInterface_invariantsMatchSink)
    {
        orc::AudioSinkStage stage;
        EXPECT_EQ(stage.required_input_count(), 1u);
        EXPECT_EQ(stage.output_count(), 0u);
        EXPECT_EQ(stage.get_node_type_info().type, orc::NodeType::SINK);
    }

    TEST(AudioSinkStageTest, descriptorDefaults_outputPathIsEmptyWav)
    {
        orc::AudioSinkStage stage;
        const auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(), [](const orc::ParameterDescriptor& d) {
            return d.name == "output_path";
        });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".wav");
        ASSERT_TRUE(it->constraints.default_value.has_value());
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
    }

    TEST(AudioSinkStageTest, triggerFails_whenNoInputProvided)
    {
        orc::AudioSinkStage stage;
        MockObservationContext observation_context;

        const bool result = stage.trigger({}, {}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: Audio sink requires one input (VideoFieldRepresentation)");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }

    TEST(AudioSinkStageTest, triggerFails_whenInputHasNoAudio)
    {
        orc::AudioSinkStage stage;
        MockObservationContext observation_context;
        auto vfr = std::make_shared<NiceMock<MockVideoFieldRepresentation>>();

        EXPECT_CALL(*vfr, has_audio()).WillOnce(Return(false));

        const bool result = stage.trigger({vfr}, {{"output_path", std::string("ignored.wav")}}, observation_context);

        EXPECT_FALSE(result);
        EXPECT_EQ(stage.get_trigger_status(), "Error: Input VFR does not have audio data (no PCM file specified in source?)");
        EXPECT_FALSE(stage.is_trigger_in_progress());
    }
}
