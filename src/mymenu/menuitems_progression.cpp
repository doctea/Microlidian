#include "Config.h"

#if defined(ENABLE_SCREEN) && defined(ENABLE_ARRANGER)

#include "mymenu/menuitems_progression.h"
#include "arranger.h"
#include "bpm.h"

int ProgressionPinnedMenuItem::display(Coord pos, bool selected, bool opened) {
    int y = pos.y;
    tft->setCursor(0, y);

    if (menu->expand_pinned_level() > 0) {
        tft->setTextSize(2);
    } else {
        tft->setTextSize(0);
        tft->println("^^^^^^^^^^^^^^^^^^^^^^^^^");
        return tft->getCursorY() - 12;
    }

    if (arranger == nullptr) {
        tft->println("No arranger");
        return tft->getCursorY();
    }

    // Playlist position / plays — green if advancing, red if paused
    if (arranger->is_playlist_mode()) {
        tft->setTextColor(GREEN, BLACK);
    } else {
        tft->setTextColor(RED, BLACK);
    }
    tft->printf("%i/%i ", arranger->playlist_position + 1, NUM_PLAYLIST_SLOTS);

    tft->setTextColor(C_WHITE, BLACK);
    tft->printf("%i/%i ",
        arranger->current_section_plays + 1,
        arranger->get_current_playlist_repeats()
    );

    // Section name (short: up to 8 chars to leave room for bar)
    tft->setTextColor(C_WHITE, BLACK);
    tft->printf("%-8.8s ", (char*)get_section_name(arranger->current_section));

    // Bar position — green if advancing, red if paused
    if (!arranger->is_bar_mode()) {
        tft->setTextColor(GREEN, BLACK);
    } else {
        tft->setTextColor(RED, BLACK);
    }
    const uint8_t sec_len = arranger->song_sections[arranger->current_section].length;
    tft->printf("%i/%i", max(0, (int)arranger->current_bar) + 1, (int)sec_len);
    tft->setTextColor(C_WHITE, BLACK);

    // Loop mode indicator
    if (arranger->is_section_mode()) {
        tft->printf(" [L]");
    }
    tft->println();

    y = tft->getCursorY() - 4;
    return y;
}

#endif  // ENABLE_SCREEN && ENABLE_ARRANGER
