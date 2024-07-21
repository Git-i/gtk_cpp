#pragma once
#include <gkkmm.h>
class MainWindow : public Gtk::Window 
{
public:
    MainWindow(): m_button("Hello")
    {
        m_button.signal_clicked().connect([]() {

        });
        set_child(m_button);
    }
private:
    Gtk::Button m_button;
};