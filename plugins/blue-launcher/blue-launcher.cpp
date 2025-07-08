#include <wayfire/plugin.hpp>
#include <wayfire/output.hpp>
#include <wayfire/core.hpp>
#include <wayfire/scene.hpp>
#include <wayfire/view.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/opengl.hpp>

class blue_launcher : public wf::plugin_interface_t
{
  wf::option_wrapper_t<wf::keybinding_t> toggle{"blue-launcher/toggle"};
  wf::signal_connection_t keybinding_connection;

  wf::scene::floating_inner_ptr overlay;
  bool visible = false;

public:
  void init() override
  {
    toggle.set("MOD4+SPACE");
    auto& core = wf::get_core();

    // Utwórz overlay - pusty kontener, który później możesz animować
    auto output = core.get_active_output();
    overlay = output->workspace->create_child<wf::scene::floating_inner_t>();
    overlay->set_z_order(1000);  // na wierzchu

    // Podłącz skrót
    keybinding_connection = core.input->keyboard->connect_key(toggle, [=] (auto) {
      toggle_overlay();
    });
  }

  void toggle_overlay()
  {
    if (visible)
    {
      // Ukryj z animacją fade out
      overlay->fade(0.0, 200);  // 200ms fade out
      visible = false;
    }
    else
    {
      // Pokaż z animacją fade in
      overlay->fade(1.0, 200);  // 200ms fade in
      visible = true;
    }
  }

  void fini() override
  {
    keybinding_connection.disconnect();
    overlay->destroy();
  }
};

extern "C"
{
  wf::plugin_interface_t * newInstance()
  {
    return new blue_launcher();
  }
}
