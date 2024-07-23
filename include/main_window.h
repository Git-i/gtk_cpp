#pragma once
#include "asio/io_context.hpp"
#include "chat_client.h"
#include "giomm/appinfo.h"
#include "giomm/application.h"
#include "giomm/listmodel.h"
#include "giomm/liststore.h"
#include "glib.h"
#include "glibmm/objectbase.h"
#include "glibmm/refptr.h"
#include "gtk/gtk.h"
#include "gtkmm/builder.h"
#include "gtkmm/entry.h"
#include "gtkmm/enums.h"
#include "gtkmm/label.h"
#include "gtkmm/listbox.h"
#include "gtkmm/listitem.h"
#include "gtkmm/listitemfactory.h"
#include "gtkmm/liststore.h"
#include "gtkmm/listview.h"
#include "gtkmm/object.h"
#include "gtkmm/revealer.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/signallistitemfactory.h"
#include "gtkmm/singleselection.h"
#include "gtkmm/window.h"
#include "message.h"
#include "sigc++/functors/ptr_fun.h"
#include <gtkmm.h>
#include <memory>
class GlibChat: public Glib::Object
{
public:
    Chat chat;
    GlibChat() : Glib::ObjectBase(typeid(GlibChat)){}
    GlibChat(const Chat& ch) : Glib::ObjectBase(typeid(GlibChat)), chat(ch) 
    {
    }
    virtual ~GlibChat() override
    {

    }   
};

static void setup_cb(const Glib::RefPtr<Gtk::ListItem>& item)
{
    auto label = Gtk::make_managed<Gtk::Label>("Empty");
    item->set_child(*label);
}
class MainWindow : public Gtk::Window 
{
public:

    MainWindow(asio::io_context& ctx, const char* sv_name, const char* sv_port, const char* user_name) : 
        cl(ctx, sv_name, sv_port, user_name),
        m_sendMessage("Send"),
        list(Gio::ListStore<GlibChat>::create()),
        messages(
            Gtk::NoSelection::create(list),
            Gtk::SignalListItemFactory::create()
        ),
        m_chatBox(Gtk::Orientation::HORIZONTAL),
        m_layoutBox(Gtk::Orientation::VERTICAL),
        m_ctx(ctx),
        t([this]{m_ctx.run();})
    {
        set_default_size(1280, 720);
        set_title("Lchat client");

        m_sendMessage.signal_clicked().connect([this]
        {
            cl.SendMessage(m_chatBar.get_buffer()->get_text(), 0);
            m_chatBar.get_buffer()->set_text("");
        });
        cl.on_message = [this](const Chat& msg)
        {
            auto ptr = std::static_pointer_cast<Gio::ListStore<GlibChat>>(list);
            ptr->append(
                std::make_shared<GlibChat>(msg)
            );
        };

        m_chatBox.set_halign(Gtk::Align::CENTER);
        m_chatBox.set_valign(Gtk::Align::FILL);

        m_chatBox.append(m_chatBar);
        m_chatBox.append(m_sendMessage);

        auto factory = std::static_pointer_cast<Gtk::SignalListItemFactory>(messages.get_factory());
        factory->signal_setup().    connect(sigc::ptr_fun(setup_cb));
        factory->signal_bind().connect(sigc::mem_fun(*this, &MainWindow::bind_cb));
        m_messageWindow.set_child(messages);

        m_layoutBox.append(m_messageWindow);
        m_layoutBox.append(m_chatBox);

        set_child(m_layoutBox);
    }
    void bind_cb(const Glib::RefPtr<Gtk::ListItem>& item)
    {
        auto it = item->get_item();
        if(auto chat = std::dynamic_pointer_cast<GlibChat>(it))
        {
            auto label = dynamic_cast<Gtk::Label*>(item->get_child());
            label->set_label(cl.GetUserName(chat->chat.user) + ":" + chat->chat.text);
        }
    }
private:
    ChatClient cl;
    Gtk::Entry m_chatBar;
    Gtk::Button m_sendMessage;
    Gtk::ScrolledWindow m_messageWindow;
    Glib::RefPtr<Gio::ListModel> list;
    Gtk::ListView messages;
    Gtk::Box m_chatBox;
    Gtk::Box m_layoutBox;
    asio::io_context& m_ctx;
    std::thread t;
};