/*
 * File:        ntsc_comp_source_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for NTSCCompSourceStage parameter descriptors, defaults,
 *              set_parameters validation, and stage interface invariants.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>
#include <algorithm>

#include "../../../../orc/core/stages/ntsc_comp_source/ntsc_comp_source_stage.h"
#include "../../../../orc/core/include/observation_context.h"

namespace orc_unit_test
{
    // =========================================================================
    // Stage interface invariants
    // =========================================================================

    TEST(NTSCCompSourceStageTest, requiredInputCount_isZero)
    {
        orc::NTSCCompSourceStage stage;
        EXPECT_EQ(stage.required_input_count(), 0u);
    }

    TEST(NTSCCompSourceStageTest, outputCount_isOne)
    {
        orc::NTSCCompSourceStage stage;
        EXPECT_EQ(stage.output_count(), 1u);
    }

    TEST(NTSCCompSourceStageTest, nodeTypeInfo_hasSourceType)
    {
        orc::NTSCCompSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.type, orc::NodeType::SOURCE);
    }

    TEST(NTSCCompSourceStageTest, nodeTypeInfo_hasExpectedStageName)
    {
        orc::NTSCCompSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.stage_name, "NTSC_Comp_Source");
    }

    TEST(NTSCCompSourceStageTest, nodeTypeInfo_formatCompatibilityIsNtscOnly)
    {
        orc::NTSCCompSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.compatible_formats, orc::VideoFormatCompatibility::NTSC_ONLY);
    }

    // =========================================================================
    // Parameter descriptor tests
    // =========================================================================

    TEST(NTSCCompSourceStageTest, parameterDescriptors_containsInputPath)
    {
        orc::NTSCCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "input_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".tbc");
    }

    TEST(NTSCCompSourceStageTest, parameterDescriptors_containsPcmPath)
    {
        orc::NTSCCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "pcm_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".pcm");
    }

    TEST(NTSCCompSourceStageTest, parameterDescriptors_containsEfmPath)
    {
        orc::NTSCCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "efm_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".efm");
    }

    TEST(NTSCCompSourceStageTest, descriptorDefaults_inputPath_isEmptyString)
    {
        orc::NTSCCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "input_path"; });

        ASSERT_NE(it, descriptors.end());
        ASSERT_TRUE(it->constraints.default_value.has_value());
        ASSERT_TRUE(std::holds_alternative<std::string>(*it->constraints.default_value));
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
    }

    TEST(NTSCCompSourceStageTest, descriptorDefaults_pcmPath_isEmptyString)
    {
        orc::NTSCCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "pcm_path"; });

        ASSERT_NE(it, descriptors.end());
        ASSERT_TRUE(it->constraints.default_value.has_value());
        ASSERT_TRUE(std::holds_alternative<std::string>(*it->constraints.default_value));
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
    }

    TEST(NTSCCompSourceStageTest, descriptorDefaults_efmPath_isEmptyString)
    {
        orc::NTSCCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "efm_path"; });

        ASSERT_NE(it, descriptors.end());
        ASSERT_TRUE(it->constraints.default_value.has_value());
        ASSERT_TRUE(std::holds_alternative<std::string>(*it->constraints.default_value));
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
    }

    TEST(NTSCCompSourceStageTest, parameterDescriptors_allParametersAreOptional)
    {
        orc::NTSCCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        for (const auto& desc : descriptors) {
            EXPECT_FALSE(desc.constraints.required)
                << "Parameter '" << desc.name << "' should be optional";
        }
    }

    // =========================================================================
    // set_parameters validation tests
    // =========================================================================

    TEST(NTSCCompSourceStageTest, setParameters_acceptsValidStringMap)
    {
        orc::NTSCCompSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"input_path", std::string("/some/file.tbc")},
            {"pcm_path",   std::string("")},
            {"efm_path",   std::string("")}
        };

        EXPECT_TRUE(stage.set_parameters(params));
    }

    TEST(NTSCCompSourceStageTest, setParameters_rejectsNonStringInputPath)
    {
        orc::NTSCCompSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"input_path", int32_t(42)}
        };

        EXPECT_FALSE(stage.set_parameters(params));
    }

    TEST(NTSCCompSourceStageTest, setParameters_acceptsEmptyMap)
    {
        orc::NTSCCompSourceStage stage;
        EXPECT_TRUE(stage.set_parameters({}));
    }

    // =========================================================================
    // execute() contract tests
    // =========================================================================

    TEST(NTSCCompSourceStageTest, execute_throwsWhenInputProvided)
    {
        orc::NTSCCompSourceStage stage;
        orc::ObservationContext observation_context;

        EXPECT_THROW(
            stage.execute({nullptr}, {}, observation_context),
            std::runtime_error);
    }

    TEST(NTSCCompSourceStageTest, execute_returnsEmptyWhenInputPathMissing)
    {
        orc::NTSCCompSourceStage stage;
        orc::ObservationContext observation_context;

        const auto outputs = stage.execute({}, {}, observation_context);

        EXPECT_TRUE(outputs.empty());
    }

    TEST(NTSCCompSourceStageTest, execute_returnsEmptyWhenInputPathEmpty)
    {
        orc::NTSCCompSourceStage stage;
        orc::ObservationContext observation_context;

        const auto outputs = stage.execute({}, {{"input_path", std::string("")}}, observation_context);

        EXPECT_TRUE(outputs.empty());
    }

} // namespace orc_unit_test
