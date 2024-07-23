#pragma once
#include <gtkmm.h>
class MainWindow : public Gtk::Window 
{
public:
    MainWindow(): m_button("Hello, World")
    {
        set_default_size(1280, 720);
        set_title("Lchat client");
        set_child(m_button);
    }
private:
    Gtk::Button m_button;
};