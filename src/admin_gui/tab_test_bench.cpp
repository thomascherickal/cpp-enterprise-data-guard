#include "admin_gui/tabs.hpp"

#include <string>

namespace dataguard::admin::ui {

namespace {

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

}  // namespace

GtkWidget* buildTestBenchTab(AppWidgets* app) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
    gtk_box_pack_start(GTK_BOX(box), makeScrolledTextView(&app->testInputView, TRUE), TRUE, TRUE,
                       0);
    setTextViewContent(app->testInputView,
                       "const awsKey = \"AKIA1234567890ABCDEF\";\n"
                       "Customer: alice@company.com, SSN: 123-45-6789\n");
    GtkWidget* runScanBtn = gtk_button_new_with_label("Run C++ Policy Scan");
    gtk_style_context_add_class(gtk_widget_get_style_context(runScanBtn), "azure-primary");
    g_signal_connect(runScanBtn, "clicked", G_CALLBACK(onRunTestScanClicked), app);
    gtk_box_pack_start(GTK_BOX(box), runScanBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), makeScrolledTextView(&app->testOutputView, FALSE), TRUE, TRUE,
                       0);
    return box;
}

}  // namespace dataguard::admin::ui

