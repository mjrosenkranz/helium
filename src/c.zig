//! C imports.
//! https://gitlab.freedesktop.org/cairo/cairo/-/blob/master/doc/public/cairo-sections.txt
pub usingnamespace @cImport({
    // XCB is only required when using the XCB surface backend for Cairo.
    @cInclude("xcb/xcb.h");
    @cInclude("xcb/xproto.h");
});
