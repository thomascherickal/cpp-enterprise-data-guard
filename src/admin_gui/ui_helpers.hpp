#pragma once

#include "admin_gui/admin_controller.hpp"

#include <gtk/gtk.h>
#include <string>
#include <vector>

namespace dataguard::admin::ui {

struct AppWidgets {
    AdminController controller;
    GtkWidget* window = nullptr;
    GtkWidget* orgEntry = nullptr;
    GtkWidget* secretsCheck = nullptr;
    GtkWidget* piiCheck = nullptr;
    GtkWidget* entropyCheck = nullptr;
    GtkWidget* ipCheck = nullptr;
    GtkWidget* blockCriticalCheck = nullptr;
    GtkWidget* minSevCombo = nullptr;
    GtkWidget* redactionCombo = nullptr;
    GtkWidget* entropySpin = nullptr;
    GtkWidget* maxFindingsSpin = nullptr;
    GtkWidget* allowlistEntry = nullptr;
    GtkWidget* allowlistView = nullptr;
    GtkWidget* customRuleId = nullptr;
    GtkWidget* customRulePattern = nullptr;
    GtkWidget* customRuleSev = nullptr;
    GtkWidget* customRuleView = nullptr;
    GtkWidget* testInputView = nullptr;
    GtkWidget* testOutputView = nullptr;
    GtkWidget* statusLabel = nullptr;
};

void setComboText(GtkComboBoxText* combo, const std::string& value,
                  const std::vector<std::string>& options);
void setTextViewContent(GtkWidget* textView, const std::string& content);
std::string getTextViewContent(GtkWidget* textView);
GtkWidget* makeScrolledTextView(GtkWidget** outView, gboolean editable);

void syncUiFromController(AppWidgets* app);
void syncControllerFromUi(AppWidgets* app);
void applyThemeCss();

}  // namespace dataguard::admin::ui

