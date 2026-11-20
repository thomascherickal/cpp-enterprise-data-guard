#include "admin_gui/ui_helpers.hpp"

namespace dataguard::admin::ui {

void applyThemeCss() {
    static const char* kThemeCss = R"CSS(
        window, .background { background-color: #0b0e14; color: #ffffff; }
        label, checkbutton { color: #ffffff; }
        notebook { background-color: #141820; border: 1px solid #2e3646; }
        notebook > header { background-color: #0b0e14; border-bottom: 1px solid #2e3646; }
        notebook > header tab {
            background-color: #1a202c; color: #cbd5e1; padding: 8px 16px;
            border: 1px solid #2e3646; border-bottom: none;
        }
        notebook > header tab:checked {
            background-color: #004e8c; color: #ffffff;
            font-weight: bold; border-color: #004e8c;
        }
        notebook > stack { background-color: #141820; color: #ffffff; }
        entry, spinbutton, textview, textview text {
            background-color: #1e2430; color: #ffffff;
            border: 1px solid #2e3646; border-radius: 4px; caret-color: #0063b1;
        }
        entry:focus, spinbutton:focus, textview:focus { border-color: #0063b1; }
        button {
            background-image: none; background-color: #1e2430; color: #ffffff;
            border: 1px solid #3a4559; border-radius: 5px; padding: 6px 14px; font-weight: 600;
        }
        button:hover { background-color: #283040; border-color: #0063b1; }
        button.azure-primary { background-color: #004e8c; border-color: #004e8c; color: #ffffff; }
        button.azure-primary:hover { background-color: #0063b1; border-color: #0078d4; }
        checkbutton check { background-color: #1e2430; border: 1px solid #4a5568; color: #ffffff; }
        checkbutton:checked check { background-color: #004e8c; border-color: #004e8c; }
        combobox button { background-color: #1e2430; color: #ffffff; }
    )CSS";

    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, kThemeCss, -1, nullptr);
    GdkScreen* screen = gdk_screen_get_default();
    if (screen != nullptr) {
        gtk_style_context_add_provider_for_screen(
            screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    g_object_unref(provider);
}

}  // namespace dataguard::admin::ui

