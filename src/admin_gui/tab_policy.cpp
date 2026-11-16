#include "admin_gui/tabs.hpp"

namespace dataguard::admin::ui {

GtkWidget* buildPolicyTab(AppWidgets* app) {
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);

    app->orgEntry = gtk_entry_new();
    app->secretsCheck = gtk_check_button_new_with_label("Scan API Keys, Tokens & Private Keys");
    app->piiCheck =
        gtk_check_button_new_with_label("Scan Personal Data (Credit Cards, SSN, IBAN, Email)");
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

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Organization:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app->orgEntry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app->secretsCheck, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), app->piiCheck, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), app->entropyCheck, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), app->ipCheck, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), app->blockCriticalCheck, 0, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Minimum Severity:"), 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app->minSevCombo, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Redaction Mode:"), 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app->redactionCombo, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Entropy Threshold:"), 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app->entropySpin, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Max Findings Before Block:"), 0, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app->maxFindingsSpin, 1, 9, 1, 1);

    return grid;
}

}  // namespace dataguard::admin::ui

