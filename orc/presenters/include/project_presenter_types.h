/*
 * File:        project_presenter_types.h
 * Module:      orc-presenters
 * Purpose:     Shared project presenter view types
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Simon Inns
 */

#pragma once

#include <functional>
#include <string>
#include <node_id.h>
#include <node_type.h>

namespace orc::presenters {

/**
 * @brief Video format enumeration for GUI use
 */
enum class VideoFormat {
    NTSC,
    PAL,
    Unknown
};

/**
 * @brief Source type enumeration for GUI use
 */
enum class SourceType {
    Composite,
    YC,
    Unknown
};

/**
 * @brief Information about a stage available in the registry
 */
struct StageInfo {
    std::string name;           ///< Internal stage name
    std::string display_name;   ///< User-friendly display name
    std::string description;    ///< Stage description
    NodeType node_type;         ///< Type of node
    bool is_source;             ///< True if this is a source stage
    bool is_sink;               ///< True if this is a sink stage
};

/**
 * @brief Information about a node in the project
 */
struct NodeInfo {
    NodeID node_id;             ///< Node identifier
    std::string stage_name;     ///< Stage type name
    std::string label;          ///< User-assigned label
    double x_position;          ///< X position in graph
    double y_position;          ///< Y position in graph
    bool can_remove;            ///< Whether node can be removed
    bool can_trigger;           ///< Whether node can be triggered
    bool can_inspect;           ///< Whether node can be inspected
    std::string remove_reason;  ///< Reason if cannot remove
    std::string trigger_reason; ///< Reason if cannot trigger
    std::string inspect_reason; ///< Reason if cannot inspect
};

/**
 * @brief Edge between two nodes
 */
struct EdgeInfo {
    NodeID source_node;         ///< Source node ID
    NodeID target_node;         ///< Target node ID
};

/**
 * @brief Progress callback for batch operations
 */
using ProgressCallback = std::function<void(size_t current, size_t total, const std::string& message)>;

} // namespace orc::presenters
