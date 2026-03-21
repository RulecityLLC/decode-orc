/*
 * File:        ntsc_yc_source_stage_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for NTSCYCSourceStage parameter descriptors, defaults,
 *              set_parameters validation, and stage interface invariants.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>
#include <algorithm>

#include "../../../../orc/core/stages/ntsc_yc_source/ntsc_yc_source_stage.h"
#include "../../../../orc/core/include/observation_context.h"

namespace orc_unit_test
{
    // =========================================================================
    // Stage interface invariants
    // =========================================================================

    TEST(NTSCYCSourceStageTest, requiredInputCount_isZero)
    {
        orc::NTSCYCSourceStage stage;
        EXPECT_EQ(stage.required_input_count(), 0u);
    }

    TEST(NTSCYCSourceStageTest, outputCount_isOne)
    {
        orc::NTSCYCSourceStage stage;
        EXPECT_EQ(stage.output_count(), 1u);
    }

    TEST(NTSCYCSourceStageTest, nodeTypeInfo_hasSourceType)
    {
        orc::NTSCYCSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.type, orc::NodeType::SOURCE);
    }

    TEST(NTSCYCSourceStageTest, nodeTypeInfo_hasExpectedStageName)
    {
        orc::NTSCYCSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.stage_name, "NTSC_YC_Source");
    }

    TEST(NTSCYCSourceStageTest, nodeTypeInfo_formatCompatibilityIsNtscOnly)
    {
        orc::NTSCYCSourceStage stage;
        auto info = stage.get_node_type_info();
        EXPECT_EQ(info.compatible_formats, orc::VideoFormatCompatibility::NTSC_ONLY);
    }

    // =========================================================================
    // Parameter descriptor tests
    // =========================================================================

    TEST(NTSCYCSourceStageTest, parameterDescriptors_containsYPath)
    {
        orc::NTSCYCSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "y_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".tbcy");
    }

    TEST(NTSCYCSourceStageTest, parameterDescriptors_containsCPath)
    {
        orc::NTSCYCSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "c_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".tbcc");
    }

    TEST(NTSCYCSourceStageTest, parameterDescriptors_containsDbPath)
    {
        orc::NTSCYCSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "db_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".db");
    }

    TEST(NTSCYCSourceStageTest, parameterDescriptors_containsPcmPath)
    {
        orc::NTSCYCSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "pcm_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".pcm");
    }

    TEST(NTSCYCSourceStageTest, parameterDescriptors_containsEfmPath)
    {
        orc::NTSCYCSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        auto it = std::find_if(descriptors.begin(), descriptors.end(),
            [](const orc::ParameterDescriptor& d) { return d.name == "efm_path"; });

        ASSERT_NE(it, descriptors.end());
        EXPECT_EQ(it->type, orc::ParameterType::FILE_PATH);
        EXPECT_EQ(it->file_extension_hint, ".efm");
    }

    TEST(NTSCYCSourceStageTest, descriptorDefaults_allPathsAreEmptyString)
    {
        orc::NTSCYCSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        for (const auto& desc : descriptors) {
            ASSERT_TRUE(desc.constraints.default_value.has_value())
                << "Descriptor '" << desc.name << "' has no default_value";
            ASSERT_TRUE(std::holds_alternative<std::string>(*desc.constraints.default_value))
                << "Descriptor '" << desc.name << "' default is not a string";
            EXPECT_EQ(std::get<std::string>(*desc.constraints.default_value), "")
                << "Descriptor '" << desc.name << "' default is not empty string";
        }
    }

    TEST(NTSCYCSourceStageTest, parameterDescriptors_allParametersAreOptional)
    {
        orc::NTSCYCSourceStage stage;
        auto descriptors = stage.get_parameter_descriptors();

        for (const auto& desc : descriptors) {
            EXPECT_FALSE(desc.constraints.required)
                << "Parameter '" << desc.name << "' should be optional";
        }
    }

    // =========================================================================
    // get_parameters / parity tests
    // =========================================================================

    TEST(NTSCYCSourceStageTest, getParameters_defaultYPathIsEmptyString)
    {
        orc::NTSCYCSourceStage stage;
        auto params = stage.get_parameters();

        auto it = params.find("y_path");
        ASSERT_NE(it, params.end());
        ASSERT_TRUE(std::holds_alternative<std::string>(it->second));
        EXPECT_EQ(std::get<std::string>(it->second), "");
    }

    TEST(NTSCYCSourceStageTest, getParameters_defaultCPathIsEmptyString)
    {
        orc::NTSCYCSourceStage stage;
        auto params = stage.get_parameters();

        auto it = params.find("c_path");
        ASSERT_NE(it, params.end());
        ASSERT_TRUE(std::holds_alternative<std::string>(it->second));
        EXPECT_EQ(std::get<std::string>(it->second), "");
    }

    TEST(NTSCYCSourceStageTest, getParameters_defaultDbPathIsEmptyString)
    {
        orc::NTSCYCSourceStage stage;
        auto params = stage.get_parameters();

        auto it = params.find("db_path");
        ASSERT_NE(it, params.end());
        ASSERT_TRUE(std::holds_alternative<std::string>(it->second));
        EXPECT_EQ(std::get<std::string>(it->second), "");
    }

    // =========================================================================
    // set_parameters validation tests
    // =========================================================================

    TEST(NTSCYCSourceStageTest, setParameters_acceptsValidStringMap)
    {
        orc::NTSCYCSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"y_path",   std::string("/some/file.tbcy")},
            {"c_path",   std::string("/some/file.tbcc")},
            {"db_path",  std::string("")},
            {"pcm_path", std::string("")},
            {"efm_path", std::string("")}
        };

        EXPECT_TRUE(stage.set_parameters(params));
    }

    TEST(NTSCYCSourceStageTest, setParameters_rejectsNonStringYPath)
    {
        orc::NTSCYCSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"y_path", int32_t(99)}
        };

        EXPECT_FALSE(stage.set_parameters(params));
    }

    TEST(NTSCYCSourceStageTest, setParameters_rejectsNonStringCPath)
    {
        orc::NTSCYCSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"c_path", int32_t(99)}
        };

        EXPECT_FALSE(stage.set_parameters(params));
    }

    TEST(NTSCYCSourceStageTest, setParameters_persistsValues)
    {
        orc::NTSCYCSourceStage stage;
        const std::map<std::string, orc::ParameterValue> params = {
            {"y_path", std::string("/luma.tbcy")},
            {"c_path", std::string("/chroma.tbcc")}
        };

        ASSERT_TRUE(stage.set_parameters(params));

        auto persisted = stage.get_parameters();
        EXPECT_EQ(std::get<std::string>(persisted.at("y_path")), "/luma.tbcy");
        EXPECT_EQ(std::get<std::string>(persisted.at("c_path")), "/chroma.tbcc");
    }

    TEST(NTSCYCSourceStageTest, setParameters_acceptsEmptyMap)
    {
        orc::NTSCYCSourceStage stage;
        EXPECT_TRUE(stage.set_parameters({}));
    }

    // =========================================================================
    // execute() contract tests
    // =========================================================================

    TEST(NTSCYCSourceStageTest, execute_throwsWhenInputProvided)
    {
        orc::NTSCYCSourceStage stage;
        orc::ObservationContext observation_context;

        EXPECT_THROW(
            stage.execute({nullptr}, {}, observation_context),
            std::runtime_error);
    }

    TEST(NTSCYCSourceStageTest, execute_returnsEmptyWhenYPathMissing)
    {
        orc::NTSCYCSourceStage stage;
        orc::ObservationContext observation_context;

        const auto outputs = stage.execute({}, {{"c_path", std::string("c.tbcc")}}, observation_context);

        EXPECT_TRUE(outputs.empty());
    }

    TEST(NTSCYCSourceStageTest, execute_returnsEmptyWhenCPathMissing)
    {
        orc::NTSCYCSourceStage stage;
        orc::ObservationContext observation_context;

        const auto outputs = stage.execute({}, {{"y_path", std::string("y.tbcy")}}, observation_context);

        EXPECT_TRUE(outputs.empty());
    }

} // namespace orc_unit_test
