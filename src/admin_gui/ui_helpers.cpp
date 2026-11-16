#include "admin_gui/ui_helpers.hpp"

namespace dataguard::admin::ui {

void setComboText(GtkComboBoxText* combo, const std::string& value,
                  const std::vector<std::string>& options) {
    for (std::size_t i = 0; i < options.size(); ++i) {
        if (options[i] == value) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), static_cast<gint>(i));
            return;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
}

void setTextViewContent(GtkWidget* textView, const std::string& content) {
    GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    gtk_text_buffer_set_text(buf, content.c_str(), static_cast<gint>(content.size()));
}

std::string getTextViewContent(GtkWidget* textView) {
    GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    gchar* raw = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
    std::string out = raw ? raw : "";
    g_free(raw);
    return out;
}

GtkWidget* makeScrolledTextView(GtkWidget** outView, gboolean editable) {
    GtkWidget* scroll = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    GtkWidget* view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), editable);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), view);
    *outView = view;
    return scroll;
}

void syncUiFromController(AppWidgets* app) {
    const auto& p = app->controller.policy();
    gtk_entry_set_text(GTK_ENTRY(app->orgEntry), p.organization.c_str());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->secretsCheck), p.enableSecrets);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->piiCheck), p.enablePii);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->entropyCheck), p.enableEntropy);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->ipCheck), p.enableIpAddresses);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->blockCriticalCheck), p.blockOnCritical);
    setComboText(GTK_COMBO_BOX_TEXT(app->minSevCombo), p.minSeverity,
                 {"LOW", "MEDIUM", "HIGH", "CRITICAL"});
    setComboText(GTK_COMBO_BOX_TEXT(app->redactionCombo), p.redactionMode,
                 {"PLACEHOLDER", "MASK", "REMOVE"});
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->entropySpin), p.entropyThreshold);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->maxFindingsSpin), p.maxFindingsBeforeBlock);

    std::string allowText;
    for (const auto& item : p.allowlist) {
        allowText += item + "\n";
    }
    setTextViewContent(app->allowlistView, allowText);

    std::string rulesText;
    for (const auto& r : p.customRules) {
        rulesText += r.id + " [" + r.severity + "] => " + r.pattern + "\n";
    }
    setTextViewContent(app->customRuleView, rulesText);
}

void syncControllerFromUi(AppWidgets* app) {
    Policy p = app->controller.policy();
    p.organization = gtk_entry_get_text(GTK_ENTRY(app->orgEntry));
    p.enableSecrets = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->secretsCheck));
    p.enablePii = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->piiCheck));
    p.enableEntropy = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->entropyCheck));
    p.enableIpAddresses = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->ipCheck));
    p.blockOnCritical = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->blockCriticalCheck));

    gchar* minSev = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->minSevCombo));
    if (minSev) {
        p.minSeverity = minSev;
        g_free(minSev);
    }
    gchar* redMode = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->redactionCombo));
    if (redMode) {
        p.redactionMode = redMode;
        g_free(redMode);
    }
    p.entropyThreshold = gtk_spin_button_get_value(GTK_SPIN_BUTTON(app->entropySpin));
    p.maxFindingsBeforeBlock =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->maxFindingsSpin));
    app->controller.setPolicy(p);
}

}  // namespace dataguard::admin::ui

