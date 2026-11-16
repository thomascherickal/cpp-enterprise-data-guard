#include "admin_gui/admin_controller.hpp"

#include <gtk/gtk.h>

#include <string>
#include <vector>

namespace {

struct AppWidgets {
    dataguard::admin::AdminController controller;
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
    dataguard::Policy p = app->controller.policy();
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

void onSavePolicyClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    syncControllerFromUi(app);

    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Export Enterprise Policy JSON", GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, nullptr);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "policy.json");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::string err;
        if (app->controller.savePolicyToFile(path, err)) {
            std::string msg = std::string("Saved policy to ") + path;
            gtk_label_set_text(GTK_LABEL(app->statusLabel), msg.c_str());
        } else {
            gtk_label_set_text(GTK_LABEL(app->statusLabel), err.c_str());
        }
        g_free(path);
    }
    gtk_widget_destroy(dialog);
}

void onLoadPolicyClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Load Enterprise Policy JSON", GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, nullptr);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::string err;
        if (app->controller.loadPolicyFromFile(path, err)) {
            syncUiFromController(app);
            std::string msg = std::string("Loaded policy from ") + path;
            gtk_label_set_text(GTK_LABEL(app->statusLabel), msg.c_str());
        } else {
            gtk_label_set_text(GTK_LABEL(app->statusLabel), err.c_str());
        }
        g_free(path);
    }
    gtk_widget_destroy(dialog);
}

void onResetClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    app->controller.resetToDefaults();
    syncUiFromController(app);
    gtk_label_set_text(GTK_LABEL(app->statusLabel), "Policy reset to Enterprise defaults.");
}

void onAddAllowlistClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    std::string entry = gtk_entry_get_text(GTK_ENTRY(app->allowlistEntry));
    if (app->controller.addAllowlistEntry(entry)) {
        gtk_entry_set_text(GTK_ENTRY(app->allowlistEntry), "");
        syncUiFromController(app);
        gtk_label_set_text(GTK_LABEL(app->statusLabel), "Added allowlist entry.");
    } else {
        gtk_label_set_text(GTK_LABEL(app->statusLabel), "Allowlist entry empty or already exists.");
    }
}

void onAddCustomRuleClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    dataguard::CustomRule rule;
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

void onRunTestScanClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    syncControllerFromUi(app);
    std::string sample = getTextViewContent(app->testInputView);
    auto result = app->controller.runTestScan(sample);
    setTextViewContent(app->testOutputView, result.toJson(2));
    std::string summary = "Scan complete: " + std::to_string(result.findings.size()) +
                          " finding(s), riskScore=" + std::to_string(result.riskScore) +
                          (result.blocked ? " [BLOCKED]" : " [ALLOWED]");
    gtk_label_set_text(GTK_LABEL(app->statusLabel), summary.c_str());
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

void activateGui(GtkApplication* gtkApp, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    app->window = gtk_application_window_new(gtkApp);
    gtk_window_set_title(GTK_WINDOW(app->window),
                         "Enterprise Data Guard — Admin Control Centre");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 920, 660);

    GtkWidget* rootBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(rootBox), 12);
    gtk_container_add(GTK_CONTAINER(app->window), rootBox);

    GtkWidget* header = gtk_label_new(nullptr);
    gtk_label_set_markup(
        GTK_LABEL(header),
        "<b>Enterprise Data Guard — C++ Admin Control Centre</b>  "
        "<small>(Shared Core Engine v1.0.0)</small>");
    gtk_box_pack_start(GTK_BOX(rootBox), header, FALSE, FALSE, 4);

    GtkWidget* notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(rootBox), notebook, TRUE, TRUE, 0);

    // Tab 1: Policy Configuration
    GtkWidget* policyGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(policyGrid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(policyGrid), 12);
    gtk_container_set_border_width(GTK_CONTAINER(policyGrid), 12);

    app->orgEntry = gtk_entry_new();
    app->secretsCheck = gtk_check_button_new_with_label("Scan API Keys, Tokens & Private Keys");
    app->piiCheck = gtk_check_button_new_with_label("Scan Personal Data (Credit Cards, SSN, IBAN, Email)");
    app->entropyCheck = gtk_check_button_new_with_label("Enable High-Entropy Bearer Detection");
    app->ipCheck = gtk_check_button_new_with_label("Flag IPv4 Addresses");
    app->blockCriticalCheck = gtk_check_button_new_with_label("Block Prompt on Critical Finding");

    app->minSevCombo = gtk_combo_box_text_new();
    for (const char* s : {"LOW", "MEDIUM", "HIGH", "CRITICAL"}) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->minSevCombo), s);
    }
    app->redactionCombo = gtk_combo_box_text_new();
    for (const char* m : {"PLACEHOLDER", "MASK", "REMOVE"}) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->redactionCombo), m);
    }
    app->entropySpin = gtk_spin_button_new_with_range(2.0, 6.0, 0.1);
    app->maxFindingsSpin = gtk_spin_button_new_with_range(0, 100, 1);

    gtk_grid_attach(GTK_GRID(policyGrid), gtk_label_new("Organization:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->orgEntry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->secretsCheck, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->piiCheck, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->entropyCheck, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->ipCheck, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->blockCriticalCheck, 0, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), gtk_label_new("Minimum Severity:"), 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->minSevCombo, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), gtk_label_new("Redaction Mode:"), 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->redactionCombo, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), gtk_label_new("Entropy Threshold:"), 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->entropySpin, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), gtk_label_new("Max Findings Before Block:"), 0, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(policyGrid), app->maxFindingsSpin, 1, 9, 1, 1);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), policyGrid,
                             gtk_label_new("Policy Configuration"));

    // Tab 2: Allowlist & Custom Rules
    GtkWidget* rulesBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(rulesBox), 12);

    GtkWidget* allowRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    app->allowlistEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->allowlistEntry), "Allowlist token or email...");
    GtkWidget* addAllowBtn = gtk_button_new_with_label("Add Allowlist Entry");
    g_signal_connect(addAllowBtn, "clicked", G_CALLBACK(onAddAllowlistClicked), app);
    gtk_box_pack_start(GTK_BOX(allowRow), app->allowlistEntry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(allowRow), addAllowBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rulesBox), allowRow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rulesBox), makeScrolledTextView(&app->allowlistView, FALSE),
                       TRUE, TRUE, 0);

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
    gtk_box_pack_start(GTK_BOX(rulesBox), customRow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rulesBox), makeScrolledTextView(&app->customRuleView, FALSE),
                       TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), rulesBox,
                             gtk_label_new("Allowlist & Custom Rules"));

    // Tab 3: Live Scanner Test Bench
    GtkWidget* testBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(testBox), 12);
    gtk_box_pack_start(GTK_BOX(testBox), makeScrolledTextView(&app->testInputView, TRUE),
                       TRUE, TRUE, 0);
    setTextViewContent(app->testInputView,
                       "const awsKey = \"AKIA1234567890ABCDEF\";\n"
                       "Customer: alice@company.com, SSN: 123-45-6789\n");
    GtkWidget* runScanBtn = gtk_button_new_with_label("Run C++ Policy Scan");
    g_signal_connect(runScanBtn, "clicked", G_CALLBACK(onRunTestScanClicked), app);
    gtk_box_pack_start(GTK_BOX(testBox), runScanBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(testBox), makeScrolledTextView(&app->testOutputView, FALSE),
                       TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), testBox,
                             gtk_label_new("Live Scanner Test Bench"));

    // Footer action bar
    GtkWidget* actionRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget* loadBtn = gtk_button_new_with_label("Load Policy JSON...");
    GtkWidget* saveBtn = gtk_button_new_with_label("Export Policy JSON...");
    GtkWidget* resetBtn = gtk_button_new_with_label("Reset Defaults");
    g_signal_connect(loadBtn, "clicked", G_CALLBACK(onLoadPolicyClicked), app);
    g_signal_connect(saveBtn, "clicked", G_CALLBACK(onSavePolicyClicked), app);
    g_signal_connect(resetBtn, "clicked", G_CALLBACK(onResetClicked), app);
    gtk_box_pack_start(GTK_BOX(actionRow), loadBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actionRow), saveBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actionRow), resetBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rootBox), actionRow, FALSE, FALSE, 0);

    app->statusLabel = gtk_label_new("Ready.");
    gtk_widget_set_halign(app->statusLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(rootBox), app->statusLabel, FALSE, FALSE, 2);

    syncUiFromController(app);
    gtk_widget_show_all(app->window);
}

}  // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--headless" || arg == "--help" || arg == "--export-policy" ||
            arg == "--scan") {
            return dataguard::admin::runHeadlessCli(argc, argv);
        }
    }

    AppWidgets app;
    GtkApplication* gtkApp =
        gtk_application_new("org.dataguard.AdminControlCentre", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtkApp, "activate", G_CALLBACK(activateGui), &app);
    int status = g_application_run(G_APPLICATION(gtkApp), argc, argv);
    g_object_unref(gtkApp);
    return status;
}

