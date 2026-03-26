/*
 * File:        vectorscope_analysis_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for vectorscope extraction from ComponentFrame
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>

#include "../../../../orc/core/analysis/vectorscope/vectorscope_analysis.h"
#include "../../../../orc/core/stages/chroma_sink/decoders/componentframe.h"

namespace orc_unit_test
{
TEST(VectorscopeAnalysisTest, extractFromComponentFrameUsesActivePictureWindow)
{
    orc::SourceParameters source_parameters;
    source_parameters.field_width = 8;
    source_parameters.field_height = 4;
    source_parameters.active_video_start = 2;
    source_parameters.active_video_end = 6;
    source_parameters.first_active_frame_line = 1;
    source_parameters.last_active_frame_line = 6;

    ComponentFrame frame;
    frame.init(source_parameters, false);

    for (int32_t y = 0; y < frame.getHeight(); ++y) {
        double* u_line = frame.u(y);
        double* v_line = frame.v(y);

        for (int32_t x = 0; x < frame.getWidth(); ++x) {
            const bool inside_active =
                y >= source_parameters.first_active_frame_line &&
                y < source_parameters.last_active_frame_line &&
                x >= source_parameters.active_video_start &&
                x < source_parameters.active_video_end;

            if (inside_active) {
                u_line[x] = static_cast<double>((y * 100) + x);
                v_line[x] = -static_cast<double>((y * 100) + x);
            } else {
                u_line[x] = 10000.0;
                v_line[x] = -10000.0;
            }
        }
    }

    const auto data = orc::VectorscopeAnalysisTool::extractFromComponentFrame(
        frame,
        source_parameters,
        42,
        1);

    EXPECT_EQ(data.field_number, 42u);
    EXPECT_EQ(data.width, 4u);
    EXPECT_EQ(data.height, 5u);
    ASSERT_EQ(data.samples.size(), 20u);

    bool saw_first_field = false;
    bool saw_second_field = false;

    for (const auto& sample : data.samples) {
        EXPECT_LT(std::abs(sample.u), 10000.0);
        EXPECT_LT(std::abs(sample.v), 10000.0);

        if (sample.field_id == 0) {
            saw_first_field = true;
        }
        if (sample.field_id == 1) {
            saw_second_field = true;
        }
    }

    EXPECT_TRUE(saw_first_field);
    EXPECT_TRUE(saw_second_field);
}
}