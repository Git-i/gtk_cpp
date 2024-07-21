#include <gtkmm.h>
#include <iostream>
#include "main_window.h"

int main (int argc, char *argv[]) {
    // Create a new application
    auto app = Gtk::Application::create("org.git_i.lchat");
    app->make_window_and_run<MainWindow>(argc, argv);
}
