/*
 * File:        vectorscope_geometry_test.cpp
 * Module:      orc-tests/gui/unit
 * Purpose:     Regression tests for shared vectorscope geometry helpers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Simon Inns
 */

#include <gtest/gtest.h>

#include <cmath>

#include "preview/vectorscope_geometry.h"

namespace gui_unit_test {

TEST(VectorscopeGeometryTest, plotGeometryMatchesVectorscopeRasterMapping)
{
    const orc::gui::VectorscopePlotGeometry geometry;

    EXPECT_EQ(geometry.canvas_size, 1024);
    EXPECT_DOUBLE_EQ(geometry.centre_point.x(), 512.0);
    EXPECT_DOUBLE_EQ(geometry.centre_point.y(), 512.0);
    EXPECT_DOUBLE_EQ(geometry.plot_area.left(), 0.0);
    EXPECT_DOUBLE_EQ(geometry.plot_area.top(), 0.0);
    EXPECT_DOUBLE_EQ(geometry.plot_area.right(), 1024.0);
    EXPECT_DOUBLE_EQ(geometry.plot_area.bottom(), 1024.0);

    const QPointF origin = geometry.mapUV(0.0, 0.0);
    EXPECT_DOUBLE_EQ(origin.x(), 512.0);
    EXPECT_DOUBLE_EQ(origin.y(), 512.0);

    const QPointF top_left = geometry.mapUV(-orc::gui::kVectorscopeSignedFullScale,
                                            orc::gui::kVectorscopeSignedFullScale);
    EXPECT_DOUBLE_EQ(top_left.x(), 0.0);
    EXPECT_DOUBLE_EQ(top_left.y(), 0.0);

    const QPointF bottom_right = geometry.mapUV(orc::gui::kVectorscopeSignedFullScale,
                                                -orc::gui::kVectorscopeSignedFullScale);
    EXPECT_DOUBLE_EQ(bottom_right.x(), 1024.0);
    EXPECT_DOUBLE_EQ(bottom_right.y(), 1024.0);
}

TEST(VectorscopeGeometryTest, ntscAndPalTargetsShareTheSameDecodedUvSpace)
{
    constexpr double kIreRange = 50000.0;

    const orc::UVSample pal_target = orc::gui::vectorscopeTargetUv(4, 1.0, kIreRange, orc::VideoSystem::PAL);
    const orc::UVSample ntsc_target = orc::gui::vectorscopeTargetUv(4, 1.0, kIreRange, orc::VideoSystem::NTSC);

    EXPECT_DOUBLE_EQ(ntsc_target.u, pal_target.u);
    EXPECT_DOUBLE_EQ(ntsc_target.v, pal_target.v);
}

TEST(VectorscopeGeometryTest, ntscDisplayTargetsApplyDecoderSpaceCalibration)
{
    constexpr double kIreRange = 50000.0;

    const orc::UVSample raw_target = orc::gui::vectorscopeTargetUv(4, 0.75, kIreRange, orc::VideoSystem::NTSC);
    const orc::UVSample display_target = orc::gui::vectorscopeDisplayTargetUv(4, 0.75, kIreRange, orc::VideoSystem::NTSC);

    EXPECT_NEAR(display_target.u, raw_target.u * orc::gui::kNtscDisplayTargetUScale, 1e-9);
    EXPECT_NEAR(display_target.v, raw_target.v * orc::gui::kNtscDisplayTargetVScale, 1e-9);
}

TEST(VectorscopeGeometryTest, palDisplayTargetsRemainUnchanged)
{
    constexpr double kIreRange = 50000.0;

    const orc::UVSample raw_target = orc::gui::vectorscopeTargetUv(4, 0.75, kIreRange, orc::VideoSystem::PAL);
    const orc::UVSample display_target = orc::gui::vectorscopeDisplayTargetUv(4, 0.75, kIreRange, orc::VideoSystem::PAL);

    EXPECT_DOUBLE_EQ(display_target.u, raw_target.u);
    EXPECT_DOUBLE_EQ(display_target.v, raw_target.v);
}

TEST(VectorscopeGeometryTest, seventyFivePercentTargetScalesSampleSpaceMagnitude)
{
    constexpr double kIreRange = 65535.0;

    const orc::UVSample full_target = orc::gui::vectorscopeTargetUv(6, 1.0, kIreRange, orc::VideoSystem::PAL);
    const orc::UVSample partial_target = orc::gui::vectorscopeTargetUv(6, 0.75, kIreRange, orc::VideoSystem::PAL);

    const double full_magnitude = std::hypot(full_target.u, full_target.v);
    const double partial_magnitude = std::hypot(partial_target.u, partial_target.v);

    EXPECT_NEAR(partial_magnitude, full_magnitude * 0.75, 1e-9);
}

} // namespace gui_unit_test