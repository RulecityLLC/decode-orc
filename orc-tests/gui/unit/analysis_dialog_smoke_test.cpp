/*
 * File:        analysis_dialog_smoke_test.cpp
 * Module:      orc-tests/gui/unit
 * Purpose:     Offscreen smoke tests for analysis dialog subclasses (Phase 9)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Simon Inns
 */

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>

#include "burstlevelanalysisdialog.h"
#include "dropoutanalysisdialog.h"
#include "snranalysisdialog.h"
#include "ntscobserverdialog.h"
#include "qualitymetricsdialog.h"
#include "vbidialog.h"
#include "linescopedialog.h"
#include "hintsdialog.h"
#include "fieldtimingdialog.h"
#include "inspection_dialog.h"
#include "presenters/include/stage_inspection_view_models.h"

// GenericAnalysisDialog and DropoutEditorDialog are intentionally omitted:
// - GenericAnalysisDialog requires a live AnalysisPresenter* and AnalysisToolInfo, which
//   cannot be constructed without a running pipeline. An integration-level test would be
//   appropriate once a mock AnalysisPresenter seam is extracted.
// - DropoutEditorDialog requires a DropoutPresenter* and a shared_ptr field representation,
//   which similarly require presenter infrastructure beyond the scope of offscreen smoke tests.

namespace gui_unit_test {

namespace {

QApplication& ensureApplication()
{
    if (auto* existing_app = qobject_cast<QApplication*>(QCoreApplication::instance())) {
        return *existing_app;
    }

    static int argc = 3;
    static char app_name[] = "orc-gui-analysis-dialog-smoke-test";
    static char platform_opt[] = "-platform";
    static char platform_val[] = "offscreen";
    static char* argv[] = {app_name, platform_opt, platform_val, nullptr};
    static QApplication* app = [] {
        auto* created_app = new QApplication(argc, argv);
        created_app->setQuitOnLastWindowClosed(false);
        return created_app;
    }();
    return *app;
}

} // namespace

// ---------------------------------------------------------------------------
// AnalysisDialogBase subclasses
// ---------------------------------------------------------------------------

TEST(AnalysisDialogSmokeTest, burstLevelAnalysisDialog_canShowAndClose)
{
    (void)ensureApplication();

    BurstLevelAnalysisDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, dropoutAnalysisDialog_canShowAndClose)
{
    (void)ensureApplication();

    DropoutAnalysisDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, snrAnalysisDialog_canShowAndClose)
{
    (void)ensureApplication();

    SNRAnalysisDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

// ---------------------------------------------------------------------------
// Analysis and observation dialogs (simple QWidget* parent constructors)
// ---------------------------------------------------------------------------

TEST(AnalysisDialogSmokeTest, ntscObserverDialog_canShowAndClose)
{
    (void)ensureApplication();

    NtscObserverDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, qualityMetricsDialog_canShowAndClose)
{
    (void)ensureApplication();

    QualityMetricsDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, vbiDialog_canShowAndClose)
{
    (void)ensureApplication();

    VBIDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, lineScopeDialog_canShowAndClose)
{
    (void)ensureApplication();

    LineScopeDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, hintsDialog_canShowAndClose)
{
    (void)ensureApplication();

    HintsDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, fieldTimingDialog_canShowAndClose)
{
    (void)ensureApplication();

    FieldTimingDialog dialog;

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

TEST(AnalysisDialogSmokeTest, inspectionDialog_canShowAndClose)
{
    (void)ensureApplication();

    orc::presenters::StageInspectionView report;
    report.summary = "smoke test report";
    report.items.emplace_back("key", "value");

    orc::InspectionDialog dialog(report);

    dialog.show();
    QCoreApplication::processEvents();

    EXPECT_TRUE(dialog.isVisible());

    dialog.close();
    QCoreApplication::processEvents();

    EXPECT_FALSE(dialog.isVisible());
}

} // namespace gui_unit_test
