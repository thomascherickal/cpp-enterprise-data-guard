#include "admin_gui/tabs.hpp"

#include <string>

namespace dataguard::admin::ui {

namespace {

void onAddAllowlistClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    std::string entry = gtk_entry_get_text(GTK_ENTRY(app->allowlistEntry));
    if (app->controller.addAllowlistEntry(entry)) {
        gtk_entry_set_text(GTK_ENTRY(app->allowlistEntry), "");
        syncUiFromController(app);
        gtk_label_set_text(GTK_LABEL(app->statusLabel), "Added allowlist entry.");
    } else {
        gtk_label_set_text(GTK_LABEL(app->statusLabel),
                           "Allowlist entry empty or already exists.");
    }
}

void onAddCustomRuleClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    CustomRule rule;
    rule.id = gtk_entry_get_text(GTK_ENTRY(app->customRuleId));
    rule.pattern = gtk_entry_get_text(GTK_ENTRY(app->customRulePattern));
    gchar* sev = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->customRuleSev));
    if (sev) {
        rule.severity = sev;
        g_free(sev);
    }
    rule.category = "CUSTOM";
    rule.description = "Admin custom rule";

    std::string err;
    if (app->controller.addCustomRule(rule, err)) {
        gtk_entry_set_text(GTK_ENTRY(app->customRuleId), "");
        gtk_entry_set_text(GTK_ENTRY(app->customRulePattern), "");
        syncUiFromController(app);
        gtk_label_set_text(GTK_LABEL(app->statusLabel), "Added custom rule.");
    } else {
        gtk_label_set_text(GTK_LABEL(app->statusLabel), err.c_str());
    }
}

}  // namespace

GtkWidget* buildRulesTab(AppWidgets* app) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget* allowRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    app->allowlistEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->allowlistEntry),
                                   "Allowlist token or email...");
    GtkWidget* addAllowBtn = gtk_button_new_with_label("Add Allowlist Entry");
    g_signal_connect(addAllowBtn, "clicked", G_CALLBACK(onAddAllowlistClicked), app);
    gtk_box_pack_start(GTK_BOX(allowRow), app->allowlistEntry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(allowRow), addAllowBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), allowRow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), makeScrolledTextView(&app->allowlistView, FALSE), TRUE, TRUE,
                       0);

    GtkWidget* customRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    app->customRuleId = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->customRuleId), "Rule ID (e.g. CUST-1)");
    app->customRulePattern = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->customRulePattern), "Regex pattern...");
    app->customRuleSev = gtk_combo_box_text_new();
    for (const char* s : {"LOW", "MEDIUM", "HIGH", "CRITICAL"}) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->customRuleSev), s);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->customRuleSev), 2);
    GtkWidget* addRuleBtn = gtk_button_new_with_label("Add Custom Rule");
    g_signal_connect(addRuleBtn, "clicked", G_CALLBACK(onAddCustomRuleClicked), app);
    gtk_box_pack_start(GTK_BOX(customRow), app->customRuleId, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(customRow), app->customRulePattern, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(customRow), app->customRuleSev, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(customRow), addRuleBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), customRow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), makeScrolledTextView(&app->customRuleView, FALSE), TRUE, TRUE,
                       0);

    return box;
}

}  // namespace dataguard::admin::ui

