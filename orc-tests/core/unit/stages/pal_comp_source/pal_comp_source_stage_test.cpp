/*
 * File:        pal_comp_source_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for PALCompSourceStage parameter descriptors, defaults,
 *              set_parameters validation, and stage interface invariants.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>
#include <algorithm>

#include "../../../../orc/core/stages/pal_comp_source/pal_comp_source_stage.h"
#include "../../../../orc/core/include/observation_context.h"

namespace orc_unit_test
{
    // =========================================================================
    // Stage interface invariants
    // =========================================================================

    TEST(PALCompSourceStageTest, requiredInputCount_isZero)
    {
        orc::PALCompSourceStage stage;
        EXPECT_EQ(stage.required_input_count(), 0u);
    }

    TEST(PALCompSourceStageTest, outputCount_isOne)
    {
        orc::PALCompSourceStage stage;
        EXPECT_EQ(stage.output_count(), 1u);
    }

    TEST(PALCompSourceStageTest, nodeTypeInfo_hasSourceType)
    {
        orc::PALCompSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.type, orc::NodeType::SOURCE);
    }

    TEST(PALCompSourceStageTest, nodeTypeInfo_hasExpectedStageName)
    {
        orc::PALCompSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.stage_name, "PAL_Comp_Source");
    }

    TEST(PALCompSourceStageTest, nodeTypeInfo_formatCompatibilityIsPalOnly)
    {
        orc::PALCompSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.compatible_formats, orc::VideoFormatCompatibility::PAL_ONLY);
    }

    // =========================================================================
    // Parameter descriptor tests
    // =========================================================================

    TEST(PALCompSourceStageTest, parameterDescriptors_containsInputPath)
    {
        orc::PALCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "input_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".tbc");
    }

    TEST(PALCompSourceStageTest, parameterDescriptors_containsPcmPath)
    {
        orc::PALCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "pcm_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".pcm");
    }

    TEST(PALCompSourceStageTest, parameterDescriptors_containsEfmPath)
    {
        orc::PALCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "efm_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".efm");
    }

    TEST(PALCompSourceStageTest, descriptorDefaults_inputPath_isEmptyString)
    {
        orc::PALCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "input_path"; });

        ASSERT_NE(it, descriptors.end());
        ASSERT_TRUE(it->constraints.default_value.has_value());
        ASSERT_TRUE(std::holds_alternative<std::string>(*it->constraints.default_value));
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
    }

    TEST(PALCompSourceStageTest, descriptorDefaults_pcmPath_isEmptyString)
    {
        orc::PALCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "pcm_path"; });

        ASSERT_NE(it, descriptors.end());
        ASSERT_TRUE(it->constraints.default_value.has_value());
        ASSERT_TRUE(std::holds_alternative<std::string>(*it->constraints.default_value));
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
    }

    TEST(PALCompSourceStageTest, descriptorDefaults_efmPath_isEmptyString)
    {
        orc::PALCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "efm_path"; });

        ASSERT_NE(it, descriptors.end());
        ASSERT_TRUE(it->constraints.default_value.has_value());
        ASSERT_TRUE(std::holds_alternative<std::string>(*it->constraints.default_value));
        EXPECT_EQ(std::get<std::string>(*it->constraints.default_value), "");
    }

    TEST(PALCompSourceStageTest, parameterDescriptors_allParametersAreOptional)
    {
        orc::PALCompSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        for (const auto& desc : descriptors) {
            EXPECT_FALSE(desc.constraints.required)
                << "Parameter '" << desc.name << "' should be optional";
        }
    }

    // =========================================================================
    // set_parameters validation tests
    // =========================================================================

    TEST(PALCompSourceStageTest, setParameters_acceptsValidStringMap)
    {
        orc::PALCompSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"input_path", std::string("/some/file.tbc")},
            {"pcm_path",   std::string("")},
            {"efm_path",   std::string("")}
        };

        EXPECT_TRUE(stage.set_parameters(params));
    }

    TEST(PALCompSourceStageTest, setParameters_rejectsNonStringInputPath)
    {
        orc::PALCompSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"input_path", int32_t(42)}
        };

        EXPECT_FALSE(stage.set_parameters(params));
    }

    TEST(PALCompSourceStageTest, setParameters_acceptsEmptyMap)
    {
        orc::PALCompSourceStage stage;
        EXPECT_TRUE(stage.set_parameters({}));
    }

    // =========================================================================
    // execute() contract tests
    // =========================================================================

    TEST(PALCompSourceStageTest, execute_throwsWhenInputProvided)
    {
        orc::PALCompSourceStage stage;
        orc::ObservationContext observation_context;

        EXPECT_THROW(
            stage.execute({nullptr}, {}, observation_context),
            std::runtime_error);
    }

    TEST(PALCompSourceStageTest, execute_returnsEmptyWhenInputPathMissing)
    {
        orc::PALCompSourceStage stage;
        orc::ObservationContext observation_context;

        const auto outputs = stage.execute({}, {}, observation_context);

        EXPECT_TRUE(outputs.empty());
    }

    TEST(PALCompSourceStageTest, execute_returnsEmptyWhenInputPathEmpty)
    {
        orc::PALCompSourceStage stage;
        orc::ObservationContext observation_context;

        const auto outputs = stage.execute({}, {{"input_path", std::string("")}}, observation_context);

        EXPECT_TRUE(outputs.empty());
    }

} // namespace orc_unit_test
