#include "admin_gui/actions.hpp"

#include "admin_gui/ui_helpers.hpp"

#include <string>

namespace dataguard::admin::ui {

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
        std::string msg = app->controller.savePolicyToFile(path, err)
                              ? std::string("Saved policy to ") + path
                              : err;
        gtk_label_set_text(GTK_LABEL(app->statusLabel), msg.c_str());
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

}  // namespace dataguard::admin::ui

