#include "outputs/envelopes.h"

#include "mymenu.h"
#include "menuitems_object.h"
#include "submenuitem_bar.h"
#include "mymenu/menuitems_envelopegraph.h"

#ifdef ENABLE_SCREEN
    void EnvelopeOutput::make_menu_items(Menu *menu, int index) {
        //#ifdef ENABLE_ENVELOPE_MENUS
            char label[40];
            snprintf(label, 40, "Envelope %i: %s", index, this->label);
            menu->add_page(label);

            SubMenuItemColumns *sub_menu_item_columns = new SubMenuItemColumns("Options", 3);

            sub_menu_item_columns->add(new ObjectNumberControl<EnvelopeOutput,byte>("Attack",  this, &EnvelopeOutput::set_attack,  &EnvelopeOutput::get_attack,    nullptr, 0, 127, true, true));
            sub_menu_item_columns->add(new ObjectNumberControl<EnvelopeOutput,byte>("Hold",    this, &EnvelopeOutput::set_hold,    &EnvelopeOutput::get_hold,      nullptr, 0, 127, true, true));
            sub_menu_item_columns->add(new ObjectNumberControl<EnvelopeOutput,byte>("Decay",   this, &EnvelopeOutput::set_decay,   &EnvelopeOutput::get_decay,     nullptr, 0, 127, true, true));
            sub_menu_item_columns->add(new ObjectNumberControl<EnvelopeOutput,byte>("Sustain", this, &EnvelopeOutput::set_sustain, &EnvelopeOutput::get_sustain,   nullptr, 0, 127, true, true));
            sub_menu_item_columns->add(new ObjectNumberControl<EnvelopeOutput,byte>("Release", this, &EnvelopeOutput::set_release, &EnvelopeOutput::get_release,   nullptr, 0, 127, true, true));
            sub_menu_item_columns->add(new ObjectToggleControl<EnvelopeOutput>     ("Invert",  this, &EnvelopeOutput::set_invert,  &EnvelopeOutput::is_invert,     nullptr));

            menu->add(new EnvelopeDisplay("Graph", this));

            menu->add(sub_menu_item_columns);
        //#endif
    }
#endif