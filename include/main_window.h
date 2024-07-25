#pragma once
#include "asio/io_context.hpp"
#include "chat_client.h"
#include <gtkmm.h>
#include <limits>
#include <memory>
#include <string>
#include "libnotify/notify.h"
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
    auto name = Gtk::make_managed<Gtk::Label>("User | Date");
    auto grid = Gtk::make_managed<Gtk::Grid>();
    grid->attach(*name, 0, 0);
    grid->attach(*label, 0, 1);
    item->set_child(*grid);
}
class MainWindow : public Gtk::Window 
{
public:

    MainWindow(asio::io_context& ctx, const char* sv_name, const char* sv_port, const char* user_name) : 
        cl(ctx, sv_name, sv_port, user_name),
        list(Gio::ListStore<GlibChat>::create()),
        messages(
            Gtk::NoSelection::create(list),
            Gtk::SignalListItemFactory::create()
        ),
        m_ctx(ctx),
        t([this]{m_ctx.run();})
    {
        set_default_size(1280, 720);
        set_title("Lchat client");

        m_sendMessage.set_icon_name("document-send");
        m_sendMessage.signal_clicked().connect([this]
        {
            cl.SendMessage(m_chatBar.get_buffer()->get_text(), 0);
            m_chatBar.get_buffer()->set_text("");
        });
        m_chatBar.signal_activate().connect([this]
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
            if(msg.user == cl.GetId())
            {

            }
        };

        auto factory = std::static_pointer_cast<Gtk::SignalListItemFactory>(messages.get_factory());
        factory->signal_setup().connect(sigc::ptr_fun(setup_cb));
        factory->signal_bind().connect(sigc::mem_fun(*this, &MainWindow::bind_cb));
        m_messageWindow.set_child(messages);
        m_messageWindow.set_min_content_height(get_allocated_height() - 10);
        m_messageWindow.set_max_content_height(get_allocated_height() + 10);
        
        m_layoutGrid.set_valign(Gtk::Align::FILL);
        m_layoutGrid.set_halign(Gtk::Align::FILL);
        m_layoutGrid.set_vexpand_set(true);
        m_layoutGrid.set_hexpand_set(true);
        m_layoutGrid.set_vexpand(true);
        m_layoutGrid.set_hexpand(true);

        m_messageWindow.set_valign(Gtk::Align::FILL);
        m_messageWindow.set_halign(Gtk::Align::FILL);
        m_messageWindow.set_vexpand_set(true);
        m_messageWindow.set_hexpand_set(true);
        m_messageWindow.set_vexpand(true);
        m_messageWindow.set_hexpand(true);

        m_chatBar.set_halign(Gtk::Align::FILL);
        m_chatBar.set_hexpand_set(true);
        m_chatBar.set_hexpand(true);

        m_layoutGrid.attach(m_messageWindow, 0, 0, 2);
        m_layoutGrid.attach(m_chatBar, 0, 1);
        m_layoutGrid.attach(m_sendMessage, 1, 1);
        set_child(m_layoutGrid);
    }
    void bind_cb(const Glib::RefPtr<Gtk::ListItem>& item)
    {
        auto it = item->get_item();
        if(auto chat = std::dynamic_pointer_cast<GlibChat>(it))
        {
            auto grid = dynamic_cast<Gtk::Grid*>(item->get_child());
            auto align = chat->chat.user == cl.GetId() ? Gtk::Align::END : Gtk::Align::START;
            grid->set_halign(align);
            auto label = dynamic_cast<Gtk::Label*>(grid->get_child_at(0, 1));
            label->set_markup(GenerateMarkup(chat->chat.text));
            auto user = dynamic_cast<Gtk::Label*>(grid->get_child_at(0, 0));
            user->set_markup("<small>" + cl.GetUserName(chat->chat.user) + " | " + chat->chat.time.ToString() + "</small>");
        }
    }
    Glib::ustring GenerateMarkup(Glib::ustring str)
    {
        Escape(str);
        auto pos = str.find("t&lt;");
        while(pos != str.npos)
        {
            auto close_pos = str.find("&gt;", pos);
            if(close_pos == str.npos) break;
            auto internal = str.substr(pos+5, close_pos - pos - 4);
            if(str[pos+5] == '\\')
            {
                str.erase(pos + 5, 1);
            }
            else 
            {
                auto id = std::numeric_limits<uint32_t>::max();
                try {
                    id = std::stoi(internal);
                } catch (const std::exception& e) {}
                auto name = cl.GetUserName(id);
                auto color = cl.GetUserColor(id);
                Escape(name);
                str.replace(pos, close_pos - pos + 4, Glib::ustring(
                    Glib::ustring("<span foreground=\"" + ColorToString(color)+
                    "\" style=\"italic\">")
                     + '@' + name + 
                     "</span>"
                ));
            }
            pos = str.find("t&lt;", pos);
        }
        return str;
    }
    Glib::ustring ColorToString(Color colour)
    {
        Glib::ustring str = "#";
        str += std::format("{:0>2x}{:0>2x}{:0>2x}{:0>2x}", 
            colour.r,
            colour.g,
            colour.b,
            colour.a);
        return str;
    }
    void Escape(Glib::ustring& str)
    {
        auto pos = str.find("<");
        while(pos != str.npos)
        {
            str.replace(pos, 1, "&lt;");
            pos = str.find("<", pos+4);
        }
        pos = str.find(">");
        while(pos != str.npos)
        {
            str.replace(pos, 1, "&gt;");
            pos = str.find(">", pos+4);
        }
    }
private:
    ChatClient cl;
    Gtk::Entry m_chatBar;
    Gtk::Button m_sendMessage;
    Gtk::ScrolledWindow m_messageWindow;
    Glib::RefPtr<Gio::ListModel> list;
    Gtk::ListView messages;
    Gtk::Grid m_layoutGrid;
    asio::io_context& m_ctx;
    std::thread t;
};