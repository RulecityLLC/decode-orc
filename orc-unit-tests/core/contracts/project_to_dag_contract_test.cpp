/*
 * File:        project_to_dag_contract_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Phase 5 contracts for project-to-DAG wiring
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "../include/public_stage_inventory.h"
#include "../../../orc/core/include/project.h"
#include "../../../orc/core/include/project_to_dag.h"

namespace orc_unit_test
{
    namespace
    {
        struct StageChain {
            std::string source;
            std::string middle;
            std::string sink;
        };

        std::optional<StageChain> find_representative_chain()
        {
            std::vector<std::string> source_names;
            std::vector<std::string> middle_names;
            std::vector<std::string> sink_names;

            for (const auto& spec : public_stage_specs()) {
                if (!spec.registry_backed) {
                    continue;
                }
                const auto info = spec.create()->get_node_type_info();

                if (spec.family == PublicStageFamily::Source) {
                    source_names.push_back(info.stage_name);
                } else if (spec.family == PublicStageFamily::Transform) {
                    middle_names.push_back(info.stage_name);
                } else {
                    sink_names.push_back(info.stage_name);
                }
            }

            for (const auto& source : source_names) {
                for (const auto& middle : middle_names) {
                    if (!orc::is_connection_valid(source, middle)) {
                        continue;
                    }

                    for (const auto& sink : sink_names) {
                        if (orc::is_connection_valid(middle, sink)) {
                            return StageChain{source, middle, sink};
                        }
                    }
                }
            }

            return std::nullopt;
        }

        std::string first_source_stage_name()
        {
            for (const auto& spec : public_stage_specs()) {
                if (spec.registry_backed && spec.family == PublicStageFamily::Source) {
                    return spec.create()->get_node_type_info().stage_name;
                }
            }

            return {};
        }
    }

    TEST(ProjectToDagContractTest, convertsRepresentativePublicPipeline)
    {
        const auto chain = find_representative_chain();
        ASSERT_TRUE(chain.has_value());

        auto project = orc::project_io::create_empty_project("phase5-contract", orc::VideoSystem::Unknown, orc::SourceType::Unknown);
        const auto source_id = orc::project_io::add_node(project, chain->source, 0.0, 0.0);
        const auto middle_id = orc::project_io::add_node(project, chain->middle, 100.0, 0.0);
        const auto sink_id = orc::project_io::add_node(project, chain->sink, 200.0, 0.0);

        orc::project_io::add_edge(project, source_id, middle_id);
        orc::project_io::add_edge(project, middle_id, sink_id);

        const auto dag = orc::project_to_dag(project);

        ASSERT_NE(dag, nullptr);
        EXPECT_TRUE(dag->validate());
        EXPECT_EQ(dag->nodes().size(), 3u);
        ASSERT_EQ(dag->output_nodes().size(), 1u);
        EXPECT_EQ(dag->output_nodes().front(), sink_id);
    }

    TEST(ProjectToDagContractTest, placeholderSourcesPassSourceValidation)
    {
        const auto source_stage_name = first_source_stage_name();
        ASSERT_FALSE(source_stage_name.empty());

        auto project = orc::project_io::create_empty_project("phase5-placeholder", orc::VideoSystem::Unknown, orc::SourceType::Unknown);
        orc::project_io::add_node(project, source_stage_name, 0.0, 0.0);

        const auto dag = orc::project_to_dag(project);
        ASSERT_NE(dag, nullptr);
        EXPECT_NO_THROW(orc::validate_source_nodes(dag));
    }

    TEST(ProjectToDagContractTest, unknownStageInProjectFailsCleanly)
    {
        auto project = orc::project_io::create_empty_project("phase5-invalid", orc::VideoSystem::Unknown, orc::SourceType::Unknown);

        std::vector<orc::ProjectDAGNode> nodes = {
            {
                orc::NodeID(1),
                "phase5_missing_stage",
                orc::NodeType::TRANSFORM,
                "Missing",
                "Missing",
                0.0,
                0.0,
                {}
            }
        };

        orc::project_io::update_project_dag(project, nodes, {});

        EXPECT_THROW(orc::project_to_dag(project), orc::ProjectConversionError);
    }
} // namespace orc_unit_test