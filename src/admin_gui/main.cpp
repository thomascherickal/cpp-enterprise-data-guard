#include "admin_gui/admin_controller.hpp"
#include "admin_gui/tabs.hpp"
#include "admin_gui/ui_helpers.hpp"

#include <gtk/gtk.h>
#include <string>

namespace {

using dataguard::admin::ui::AppWidgets;

void onSavePolicyClicked(GtkButton*, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    dataguard::admin::ui::syncControllerFromUi(app);

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
            dataguard::admin::ui::syncUiFromController(app);
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
    dataguard::admin::ui::syncUiFromController(app);
    gtk_label_set_text(GTK_LABEL(app->statusLabel), "Policy reset to Enterprise defaults.");
}

void activateGui(GtkApplication* gtkApp, gpointer userData) {
    auto* app = static_cast<AppWidgets*>(userData);
    dataguard::admin::ui::applyThemeCss();
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
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dataguard::admin::ui::buildPolicyTab(app),
                             gtk_label_new("Policy Configuration"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dataguard::admin::ui::buildRulesTab(app),
                             gtk_label_new("Allowlist & Custom Rules"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             dataguard::admin::ui::buildTestBenchTab(app),
                             gtk_label_new("Live Scanner Test Bench"));

    GtkWidget* actionRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget* loadBtn = gtk_button_new_with_label("Load Policy JSON...");
    GtkWidget* saveBtn = gtk_button_new_with_label("Export Policy JSON...");
    gtk_style_context_add_class(gtk_widget_get_style_context(saveBtn), "azure-primary");
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

    dataguard::admin::ui::syncUiFromController(app);
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
