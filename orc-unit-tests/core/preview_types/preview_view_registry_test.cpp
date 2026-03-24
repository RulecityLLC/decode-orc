/*
 * File:        preview_view_registry_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for Phase 3 preview view registry behavior.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>

#include <memory>

#include "../../../orc/core/include/preview_view_registry.h"
#include "../../../orc/core/include/colour_preview_provider.h"
#include "../../../orc/core/stages/stage.h"

namespace orc_unit_test {

namespace {

class TestPreviewStage final : public orc::DAGStage, public orc::IStagePreviewCapability {
public:
    explicit TestPreviewStage(std::vector<orc::VideoDataType> supported_types)
        : supported_types_(std::move(supported_types))
    {
    }

    std::string version() const override
    {
        return "1.0.0";
    }

    orc::NodeTypeInfo get_node_type_info() const override
    {
        return {
            orc::NodeType::TRANSFORM,
            "test_preview_stage",
            "Test Preview Stage",
            "Unit-test preview capability stage",
            1,
            1,
            1,
            1,
            orc::VideoFormatCompatibility::ALL,
            orc::SinkCategory::CORE,
        };
    }

    std::vector<orc::ArtifactPtr> execute(
        const std::vector<orc::ArtifactPtr>&,
        const std::map<std::string, orc::ParameterValue>&,
        orc::ObservationContext&) override
    {
        return {};
    }

    size_t required_input_count() const override
    {
        return 1;
    }

    size_t output_count() const override
    {
        return 1;
    }

    orc::StagePreviewCapability get_preview_capability() const override
    {
        orc::StagePreviewCapability capability{};
        capability.supported_data_types = supported_types_;
        capability.navigation_extent.item_count = 10;
        capability.navigation_extent.granularity = 1;
        capability.navigation_extent.item_label = "field";
        capability.geometry.active_width = 720;
        capability.geometry.active_height = 576;
        capability.geometry.display_aspect_ratio = 4.0 / 3.0;
        capability.geometry.dar_correction_factor = 0.7;
        return capability;
    }

private:
    std::vector<orc::VideoDataType> supported_types_;
};

class TestColourPreviewStage final
    : public orc::DAGStage
    , public orc::IStagePreviewCapability
    , public orc::IColourPreviewProvider {
public:
    std::string version() const override
    {
        return "1.0.0";
    }

    orc::NodeTypeInfo get_node_type_info() const override
    {
        return {
            orc::NodeType::SINK,
            "test_colour_preview_stage",
            "Test Colour Preview Stage",
            "Unit-test colour preview capability stage",
            1,
            1,
            1,
            1,
            orc::VideoFormatCompatibility::ALL,
            orc::SinkCategory::CORE,
        };
    }

    std::vector<orc::ArtifactPtr> execute(
        const std::vector<orc::ArtifactPtr>&,
        const std::map<std::string, orc::ParameterValue>&,
        orc::ObservationContext&) override
    {
        return {};
    }

    size_t required_input_count() const override
    {
        return 1;
    }

    size_t output_count() const override
    {
        return 1;
    }

    orc::StagePreviewCapability get_preview_capability() const override
    {
        orc::StagePreviewCapability capability{};
        capability.supported_data_types = {orc::VideoDataType::ColourNTSC};
        capability.navigation_extent.item_count = 4;
        capability.navigation_extent.granularity = 1;
        capability.navigation_extent.item_label = "frame";
        capability.geometry.active_width = 2;
        capability.geometry.active_height = 2;
        capability.geometry.display_aspect_ratio = 4.0 / 3.0;
        capability.geometry.dar_correction_factor = 1.0;
        return capability;
    }

    std::optional<orc::ColourFrameCarrier> get_colour_preview_carrier(
        uint64_t frame_index,
        orc::PreviewNavigationHint) const override
    {
        orc::ColourFrameCarrier carrier{};
        carrier.data_type = orc::VideoDataType::ColourNTSC;
        carrier.colorimetry = orc::ColorimetricMetadata::default_ntsc();
        carrier.frame_index = frame_index;
        carrier.width = 2;
        carrier.height = 2;
        carrier.y_plane = {0.25, 0.5, 0.75, 1.0};
        carrier.u_plane = {0.0, 0.1, -0.1, 0.0};
        carrier.v_plane = {0.0, -0.1, 0.1, 0.0};
        carrier.white_16b_ire = 65535.0;
        carrier.black_16b_ire = 0.0;

        orc::VectorscopeData vectorscope{};
        vectorscope.width = 2;
        vectorscope.height = 2;
        vectorscope.field_number = frame_index;
        vectorscope.samples.emplace_back(12.0, -8.0, 0);
        carrier.vectorscope_data = vectorscope;

        return carrier;
    }
};

struct TestViewState {
    bool request_called{false};
    bool export_called{false};
    bool fail_request{false};
    bool fail_export{false};
};

class TestPreviewView final : public orc::IPreviewView {
public:
    explicit TestPreviewView(std::shared_ptr<TestViewState> state)
        : state_(std::move(state))
    {
    }

    std::vector<orc::VideoDataType> supported_data_types() const override
    {
        return {orc::VideoDataType::CompositeNTSC};
    }

    orc::PreviewViewDataResult request_data(
        orc::VideoDataType,
        const orc::PreviewCoordinate&) override
    {
        state_->request_called = true;

        if (state_->fail_request) {
            return {false, "request failed", orc::PreviewViewPayloadKind::None, std::nullopt, std::nullopt};
        }

        orc::PreviewImage image{};
        image.width = 1;
        image.height = 1;
        image.rgb_data = {0, 0, 0};

        return {true, "", orc::PreviewViewPayloadKind::Image, image, std::nullopt};
    }

    orc::PreviewViewExportResult export_as(
        const std::string&,
        const std::string&) const override
    {
        state_->export_called = true;

        if (state_->fail_export) {
            return {false, "export failed"};
        }

        return {true, ""};
    }

private:
    std::shared_ptr<TestViewState> state_;
};

orc::DAG build_test_dag_with_stage(const std::shared_ptr<orc::DAGStage>& stage)
{
    orc::DAG dag;
    orc::DAGNode node;
    node.node_id = orc::NodeID(1);
    node.stage = stage;
    dag.add_node(std::move(node));
    return dag;
}

} // namespace

TEST(PreviewViewRegistryTest, registerAndListViews)
{
    orc::PreviewViewRegistry registry;

    auto state = std::make_shared<TestViewState>();
    const bool registered = registry.register_view(
        {"test.view", "Test View", {orc::VideoDataType::CompositeNTSC}},
        [state](orc::NodeID) {
            return std::make_unique<TestPreviewView>(state);
        });

    ASSERT_TRUE(registered);

    const auto listed = registry.list_views();
    ASSERT_EQ(listed.size(), 1u);
    EXPECT_EQ(listed[0].id, "test.view");
    EXPECT_EQ(listed[0].display_name, "Test View");
}

TEST(PreviewViewRegistryTest, duplicateRegistrationFails)
{
    orc::PreviewViewRegistry registry;
    auto state = std::make_shared<TestViewState>();

    ASSERT_TRUE(registry.register_view(
        {"test.view", "Test View", {orc::VideoDataType::CompositeNTSC}},
        [state](orc::NodeID) {
            return std::make_unique<TestPreviewView>(state);
        }));

    EXPECT_FALSE(registry.register_view(
        {"test.view", "Duplicate", {orc::VideoDataType::CompositeNTSC}},
        [state](orc::NodeID) {
            return std::make_unique<TestPreviewView>(state);
        }));
}

TEST(PreviewViewRegistryTest, applicableViewsFilterByStageDataType)
{
    orc::PreviewViewRegistry registry;
    auto state = std::make_shared<TestViewState>();

    ASSERT_TRUE(registry.register_view(
        {"test.view", "Test View", {orc::VideoDataType::CompositeNTSC}},
        [state](orc::NodeID) {
            return std::make_unique<TestPreviewView>(state);
        }));

    auto stage = std::make_shared<TestPreviewStage>(
        std::vector<orc::VideoDataType>{orc::VideoDataType::CompositeNTSC});
    const auto dag = build_test_dag_with_stage(stage);

    const auto supported = registry.get_applicable_views(dag, orc::NodeID(1), orc::VideoDataType::CompositeNTSC);
    EXPECT_EQ(supported.size(), 1u);

    const auto unsupported = registry.get_applicable_views(dag, orc::NodeID(1), orc::VideoDataType::ColourPAL);
    EXPECT_TRUE(unsupported.empty());
}

TEST(PreviewViewRegistryTest, requestAndExportDispatchToViewInstance)
{
    orc::PreviewViewRegistry registry;
    auto state = std::make_shared<TestViewState>();

    ASSERT_TRUE(registry.register_view(
        {"test.view", "Test View", {orc::VideoDataType::CompositeNTSC}},
        [state](orc::NodeID) {
            return std::make_unique<TestPreviewView>(state);
        }));

    auto stage = std::make_shared<TestPreviewStage>(
        std::vector<orc::VideoDataType>{orc::VideoDataType::CompositeNTSC});
    const auto dag = build_test_dag_with_stage(stage);

    orc::PreviewCoordinate coordinate{};
    coordinate.data_type_context = orc::VideoDataType::CompositeNTSC;

    const auto request_result = registry.request_data(
        dag,
        orc::NodeID(1),
        "test.view",
        orc::VideoDataType::CompositeNTSC,
        coordinate);

    EXPECT_TRUE(request_result.success);
    EXPECT_TRUE(state->request_called);

    const auto export_result = registry.export_as(
        orc::NodeID(1),
        "test.view",
        "png",
        "unused");

    EXPECT_TRUE(export_result.success);
    EXPECT_TRUE(state->export_called);
}

TEST(PreviewViewRegistryTest, requestErrorPropagates)
{
    orc::PreviewViewRegistry registry;
    auto state = std::make_shared<TestViewState>();
    state->fail_request = true;

    ASSERT_TRUE(registry.register_view(
        {"test.view", "Test View", {orc::VideoDataType::CompositeNTSC}},
        [state](orc::NodeID) {
            return std::make_unique<TestPreviewView>(state);
        }));

    auto stage = std::make_shared<TestPreviewStage>(
        std::vector<orc::VideoDataType>{orc::VideoDataType::CompositeNTSC});
    const auto dag = build_test_dag_with_stage(stage);

    const auto request_result = registry.request_data(
        dag,
        orc::NodeID(1),
        "test.view",
        orc::VideoDataType::CompositeNTSC,
        orc::PreviewCoordinate{});

    EXPECT_FALSE(request_result.success);
    EXPECT_EQ(request_result.error_message, "request failed");
}

TEST(PreviewViewRegistryTest, exportErrorPropagates)
{
    orc::PreviewViewRegistry registry;
    auto state = std::make_shared<TestViewState>();
    state->fail_export = true;

    ASSERT_TRUE(registry.register_view(
        {"test.view", "Test View", {orc::VideoDataType::CompositeNTSC}},
        [state](orc::NodeID) {
            return std::make_unique<TestPreviewView>(state);
        }));

    auto stage = std::make_shared<TestPreviewStage>(
        std::vector<orc::VideoDataType>{orc::VideoDataType::CompositeNTSC});
    const auto dag = build_test_dag_with_stage(stage);

    const auto request_result = registry.request_data(
        dag,
        orc::NodeID(1),
        "test.view",
        orc::VideoDataType::CompositeNTSC,
        orc::PreviewCoordinate{});
    ASSERT_TRUE(request_result.success);

    const auto export_result = registry.export_as(
        orc::NodeID(1),
        "test.view",
        "png",
        "unused");

    EXPECT_FALSE(export_result.success);
    EXPECT_EQ(export_result.error_message, "export failed");
}

TEST(PreviewViewRegistryTest, vectorscopeRequestDoesNotDependOnImageRenderSideChannel)
{
    orc::PreviewViewRegistry registry;

    auto stage = std::make_shared<TestColourPreviewStage>();
    auto dag = std::make_shared<orc::DAG>(build_test_dag_with_stage(stage));

    // Register default views with no preview renderer instance; vectorscope view
    // should still work because it reads directly from colour carriers.
    orc::PreviewViewRegistry::register_default_views(registry, dag, nullptr);

    orc::PreviewCoordinate coordinate{};
    coordinate.field_index = 2;
    coordinate.line_index = 0;
    coordinate.sample_offset = 0;
    coordinate.data_type_context = orc::VideoDataType::ColourNTSC;

    const auto vectorscope_result = registry.request_data(
        *dag,
        orc::NodeID(1),
        "preview.vectorscope",
        orc::VideoDataType::ColourNTSC,
        coordinate);

    ASSERT_TRUE(vectorscope_result.success);
    EXPECT_EQ(vectorscope_result.payload_kind, orc::PreviewViewPayloadKind::Vectorscope);
    ASSERT_TRUE(vectorscope_result.vectorscope.has_value());
    ASSERT_EQ(vectorscope_result.vectorscope->samples.size(), 1u);
    EXPECT_DOUBLE_EQ(vectorscope_result.vectorscope->samples[0].u, 12.0);
    EXPECT_DOUBLE_EQ(vectorscope_result.vectorscope->samples[0].v, -8.0);
}

} // namespace orc_unit_test
