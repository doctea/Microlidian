#pragma once

#ifdef ENABLE_SCREEN
#ifdef ENABLE_ARRANGER

#include "menuitems.h"
#include "arranger.h"

// Pinned panel showing current arranger song position.
// Shows: playlist slot / plays / section name / bar
// Colour-coded: green when advancing, red when paused.
class ProgressionPinnedMenuItem : public PinnedPanelMenuItem {
public:
    ProgressionPinnedMenuItem() : PinnedPanelMenuItem("ProgPos") {}

    virtual int display(Coord pos, bool selected, bool opened) override;
};

#endif  // ENABLE_ARRANGER
#endif  // ENABLE_SCREEN
