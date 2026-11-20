#pragma once

#include <gtk/gtk.h>

namespace dataguard::admin::ui {

void onSavePolicyClicked(GtkButton* button, gpointer userData);
void onLoadPolicyClicked(GtkButton* button, gpointer userData);
void onResetClicked(GtkButton* button, gpointer userData);

}  // namespace dataguard::admin::ui

