#pragma once

#include "admin_gui/ui_helpers.hpp"

namespace dataguard::admin::ui {

GtkWidget* buildPolicyTab(AppWidgets* app);
GtkWidget* buildRulesTab(AppWidgets* app);
GtkWidget* buildTestBenchTab(AppWidgets* app);

}  // namespace dataguard::admin::ui

