/*
 * File:        theme_manager_test.cpp
 * Module:      orc-tests/gui/unit
 * Purpose:     Unit tests for ThemeManager theme mode parsing and resolution
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Simon Inns
 */

#include <gtest/gtest.h>
#include <QString>
#include <QApplication>

// Include headers from their installed locations
#include "theme_manager.h"

namespace gui_unit_test
{

// =============================================================================
// ThemeManager Constructor and Mode Parsing Tests
// =============================================================================

TEST(ThemeManagerTest, constructor_autoMode_fromEmptyString)
{
    // Empty string should default to Auto mode
    ThemeManager manager("");
    EXPECT_EQ(manager.mode(), ThemeManager::Mode::Auto);
    EXPECT_FALSE(manager.hadInvalidMode());
}

TEST(ThemeManagerTest, constructor_autoMode_fromAutoString)
{
    // "auto" string should set Auto mode
    ThemeManager manager("auto");
    EXPECT_EQ(manager.mode(), ThemeManager::Mode::Auto);
    EXPECT_FALSE(manager.hadInvalidMode());
}

TEST(ThemeManagerTest, constructor_lightMode)
{
    // "light" string should set Light mode
    ThemeManager manager("light");
    EXPECT_EQ(manager.mode(), ThemeManager::Mode::Light);
    EXPECT_FALSE(manager.hadInvalidMode());
}

TEST(ThemeManagerTest, constructor_darkMode)
{
    // "dark" string should set Dark mode
    ThemeManager manager("dark");
    EXPECT_EQ(manager.mode(), ThemeManager::Mode::Dark);
    EXPECT_FALSE(manager.hadInvalidMode());
}

TEST(ThemeManagerTest, constructor_lightMode_caseInsensitive)
{
    // Case variations should be accepted for "light"
    ThemeManager manager1("LIGHT");
    EXPECT_EQ(manager1.mode(), ThemeManager::Mode::Light);
    EXPECT_FALSE(manager1.hadInvalidMode());
    
    ThemeManager manager2("Light");
    EXPECT_EQ(manager2.mode(), ThemeManager::Mode::Light);
    EXPECT_FALSE(manager2.hadInvalidMode());
}

TEST(ThemeManagerTest, constructor_darkMode_caseInsensitive)
{
    // Case variations should be accepted for "dark"
    ThemeManager manager1("DARK");
    EXPECT_EQ(manager1.mode(), ThemeManager::Mode::Dark);
    EXPECT_FALSE(manager1.hadInvalidMode());
    
    ThemeManager manager2("Dark");
    EXPECT_EQ(manager2.mode(), ThemeManager::Mode::Dark);
    EXPECT_FALSE(manager2.hadInvalidMode());
}

TEST(ThemeManagerTest, constructor_autoMode_caseInsensitive)
{
    // Case variations should be accepted for "auto"
    ThemeManager manager1("AUTO");
    EXPECT_EQ(manager1.mode(), ThemeManager::Mode::Auto);
    EXPECT_FALSE(manager1.hadInvalidMode());
    
    ThemeManager manager2("Auto");
    EXPECT_EQ(manager2.mode(), ThemeManager::Mode::Auto);
    EXPECT_FALSE(manager2.hadInvalidMode());
}

TEST(ThemeManagerTest, constructor_invalidMode_recordsError)
{
    // Invalid mode string should be recorded
    ThemeManager manager("invalid_mode");
    EXPECT_EQ(manager.mode(), ThemeManager::Mode::Auto);  // Defaults to Auto
    EXPECT_TRUE(manager.hadInvalidMode());
    EXPECT_EQ(manager.invalidMode().toStdString(), "invalid_mode");
}

TEST(ThemeManagerTest, constructor_invalidMode_preservesOriginal)
{
    // Invalid mode should preserve the original string (including leading/trailing spaces)
    ThemeManager manager("   Invalid   ");
    EXPECT_TRUE(manager.hadInvalidMode());
    // The invalidMode_ stores the untrimmed original string
    EXPECT_EQ(manager.invalidMode().toStdString(), "   Invalid   ");
}

// =============================================================================
// Mode and Name Queries Tests
// =============================================================================

TEST(ThemeManagerTest, modeName_returnsCorrectNameForAuto)
{
    ThemeManager manager("auto");
    EXPECT_EQ(manager.modeName().toStdString(), "auto");
}

TEST(ThemeManagerTest, modeName_returnsCorrectNameForLight)
{
    ThemeManager manager("light");
    EXPECT_EQ(manager.modeName().toStdString(), "light");
}

TEST(ThemeManagerTest, modeName_returnsCorrectNameForDark)
{
    ThemeManager manager("dark");
    EXPECT_EQ(manager.modeName().toStdString(), "dark");
}

TEST(ThemeManagerTest, shouldTrackSystemChanges_trueForAuto)
{
    // Auto mode should track system changes
    ThemeManager manager("auto");
    EXPECT_TRUE(manager.shouldTrackSystemChanges());
}

TEST(ThemeManagerTest, shouldTrackSystemChanges_falseForLight)
{
    // Light mode should not track system changes (explicitly set)
    ThemeManager manager("light");
    EXPECT_FALSE(manager.shouldTrackSystemChanges());
}

TEST(ThemeManagerTest, shouldTrackSystemChanges_falseForDark)
{
    // Dark mode should not track system changes (explicitly set)
    ThemeManager manager("dark");
    EXPECT_FALSE(manager.shouldTrackSystemChanges());
}

// =============================================================================
// Static String Conversion Tests
// =============================================================================

TEST(ThemeManagerTest, modeToString_auto)
{
    QString result = ThemeManager::modeToString(ThemeManager::Mode::Auto);
    EXPECT_EQ(result.toStdString(), "auto");
}

TEST(ThemeManagerTest, modeToString_light)
{
    QString result = ThemeManager::modeToString(ThemeManager::Mode::Light);
    EXPECT_EQ(result.toStdString(), "light");
}

TEST(ThemeManagerTest, modeToString_dark)
{
    QString result = ThemeManager::modeToString(ThemeManager::Mode::Dark);
    EXPECT_EQ(result.toStdString(), "dark");
}

TEST(ThemeManagerTest, colorSchemeToString_dark)
{
    QString result = ThemeManager::colorSchemeToString(Qt::ColorScheme::Dark);
    EXPECT_EQ(result.toStdString(), "dark");
}

TEST(ThemeManagerTest, colorSchemeToString_light)
{
    QString result = ThemeManager::colorSchemeToString(Qt::ColorScheme::Light);
    EXPECT_EQ(result.toStdString(), "light");
}

TEST(ThemeManagerTest, colorSchemeToString_unknown)
{
    QString result = ThemeManager::colorSchemeToString(Qt::ColorScheme::Unknown);
    EXPECT_EQ(result.toStdString(), "unknown");
}

// =============================================================================
// Resolution Tests
// =============================================================================

TEST(ThemeManagerTest, resolve_lightMode_returnsLight)
{
    ThemeManager manager("light");
    
    // Create a minimal QApplication for testing
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    
    EXPECT_EQ(resolution.mode, ThemeManager::Mode::Light);
    EXPECT_EQ(resolution.scheme, Qt::ColorScheme::Light);
    EXPECT_FALSE(resolution.isDark);
    EXPECT_FALSE(resolution.usedPaletteFallback);
    EXPECT_EQ(resolution.source.toStdString(), "cli override");
}

TEST(ThemeManagerTest, resolve_darkMode_returnsDark)
{
    ThemeManager manager("dark");
    
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    
    EXPECT_EQ(resolution.mode, ThemeManager::Mode::Dark);
    EXPECT_EQ(resolution.scheme, Qt::ColorScheme::Dark);
    EXPECT_TRUE(resolution.isDark);
    EXPECT_FALSE(resolution.usedPaletteFallback);
    EXPECT_EQ(resolution.source.toStdString(), "cli override");
}

TEST(ThemeManagerTest, resolve_autoMode_usesSystemScheme)
{
    ThemeManager manager("auto");
    
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    
    // Auto mode should resolve to something (either Light or Dark)
    EXPECT_EQ(resolution.mode, ThemeManager::Mode::Auto);
    EXPECT_TRUE(
        resolution.scheme == Qt::ColorScheme::Light ||
        resolution.scheme == Qt::ColorScheme::Dark ||
        resolution.scheme == Qt::ColorScheme::Unknown
    );
    
    // Should indicate what was used (style hints or palette fallback)
    EXPECT_TRUE(
        resolution.source.contains("style hints") ||
        resolution.source.contains("palette fallback")
    );
}

TEST(ThemeManagerTest, resolve_lightMode_noFallback)
{
    // Light mode should not use palette fallback (it's explicitly set)
    ThemeManager manager("light");
    
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    EXPECT_FALSE(resolution.usedPaletteFallback);
}

TEST(ThemeManagerTest, resolve_darkMode_noFallback)
{
    // Dark mode should not use palette fallback (it's explicitly set)
    ThemeManager manager("dark");
    
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    EXPECT_FALSE(resolution.usedPaletteFallback);
}

// =============================================================================
// Resolution Structure Tests
// =============================================================================

TEST(ThemeManagerTest, resolution_includesAllFields)
{
    ThemeManager manager("light");
    
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    
    // Verify all fields are populated
    EXPECT_FALSE(resolution.source.isEmpty());
    
    // isDark and usedPaletteFallback should have sensible values
    EXPECT_TRUE(resolution.isDark == false);  // Explicitly set light mode
    EXPECT_TRUE(resolution.usedPaletteFallback == false);
}

TEST(ThemeManagerTest, resolution_light_isDarkIsFalse)
{
    ThemeManager manager("light");
    
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    EXPECT_FALSE(resolution.isDark);
}

TEST(ThemeManagerTest, resolution_dark_isDarkIsTrue)
{
    ThemeManager manager("dark");
    
    char* argv[] = {const_cast<char*>("test")};
    int argc = 1;
    QApplication app(argc, argv);
    
    auto resolution = manager.resolve(app);
    EXPECT_TRUE(resolution.isDark);
}

} // namespace gui_unit_test
