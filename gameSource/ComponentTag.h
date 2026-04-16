#ifndef COMPONENT_TAG_INCLUDED
#define COMPONENT_TAG_INCLUDED

enum ComponentTag {
    TAG_HAT = (1 << 0),
    TAG_TOP = (1 << 1),
    TAG_BOTTOM = (1 << 2),
    TAG_BACK = (1 << 3),
    TAG_PAGE_GAMEPLAY = (1 << 4),
    TAG_CUSTOM_SERVER = (1 << 5)
    };
#endif