/*
 * File:        preview_carrier_conversion_test.cpp
 * Module:      orc-core-tests
 * Purpose:     Unit tests for Phase 2 colour carrier conversion.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>

#include "../../../orc/view-types/orc_preview_carriers.h"
#include "../../../orc/core/include/colour_preview_conversion.h"

namespace orc_unit_test
{

TEST(ColourFrameCarrierTest, invalidWhenPlaneSizesDoNotMatchDimensions)
{
    orc::ColourFrameCarrier carrier{};
    carrier.width = 2;
    carrier.height = 2;
    carrier.y_plane = {1.0, 2.0, 3.0};
    carrier.u_plane = {0.0, 0.0, 0.0};
    carrier.v_plane = {0.0, 0.0, 0.0};

    EXPECT_FALSE(carrier.is_valid());
}

TEST(ColourFrameCarrierTest, validWhenAllPlanesMatchDimensions)
{
    orc::ColourFrameCarrier carrier{};
    carrier.width = 2;
    carrier.height = 2;
    carrier.black_16b_ire = 0.0;
    carrier.white_16b_ire = 1000.0;
    carrier.y_plane = {200.0, 300.0, 400.0, 500.0};
    carrier.u_plane = {0.0, 0.0, 0.0, 0.0};
    carrier.v_plane = {0.0, 0.0, 0.0, 0.0};

    EXPECT_TRUE(carrier.is_valid());
}

TEST(ColourCarrierConversionTest, producesValidRgbImage)
{
    orc::ColourFrameCarrier carrier{};
    carrier.width = 2;
    carrier.height = 1;
    carrier.black_16b_ire = 0.0;
    carrier.white_16b_ire = 1000.0;
    carrier.colorimetry = orc::ColorimetricMetadata::default_ntsc();
    carrier.y_plane = {200.0, 800.0};
    carrier.u_plane = {0.0, 0.0};
    carrier.v_plane = {0.0, 0.0};

    const auto image = orc::render_preview_from_colour_carrier(carrier);
    ASSERT_TRUE(image.is_valid());
    ASSERT_EQ(image.width, 2u);
    ASSERT_EQ(image.height, 1u);

    // Neutral chroma should yield grayscale output.
    EXPECT_EQ(image.rgb_data[0], image.rgb_data[1]);
    EXPECT_EQ(image.rgb_data[1], image.rgb_data[2]);
    EXPECT_EQ(image.rgb_data[3], image.rgb_data[4]);
    EXPECT_EQ(image.rgb_data[4], image.rgb_data[5]);

    // Higher luma should produce a brighter output pixel.
    EXPECT_LT(image.rgb_data[0], image.rgb_data[3]);
}

TEST(ColourCarrierConversionTest, matrixSelectionAffectsRgbResult)
{
    orc::ColourFrameCarrier ntsc{};
    ntsc.width = 1;
    ntsc.height = 1;
    ntsc.black_16b_ire = 0.0;
    ntsc.white_16b_ire = 1000.0;
    ntsc.colorimetry = orc::ColorimetricMetadata::default_ntsc();
    ntsc.y_plane = {500.0};
    ntsc.u_plane = {150.0};
    ntsc.v_plane = {50.0};

    orc::ColourFrameCarrier fcc = ntsc;
    fcc.colorimetry.matrix_coefficients = orc::ColorimetricMatrixCoefficients::NTSC1953_FCC;

    const auto ntsc_image = orc::render_preview_from_colour_carrier(ntsc);
    const auto fcc_image = orc::render_preview_from_colour_carrier(fcc);

    ASSERT_TRUE(ntsc_image.is_valid());
    ASSERT_TRUE(fcc_image.is_valid());

    const bool differs = ntsc_image.rgb_data[0] != fcc_image.rgb_data[0]
        || ntsc_image.rgb_data[1] != fcc_image.rgb_data[1]
        || ntsc_image.rgb_data[2] != fcc_image.rgb_data[2];

    EXPECT_TRUE(differs);
}

} // namespace orc_unit_test
