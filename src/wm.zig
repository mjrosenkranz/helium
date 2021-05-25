const std = @import("std");
usingnamespace @import("c.zig");
const events = @import("events.zig");

pub var conn: *xcb_connection_t = undefined;
pub var screen: *xcb_screen_t = undefined;

pub fn setup() !void {
    conn = xcb_connect(null, null).?;
    // check if we connected
    if (xcb_connection_has_error(conn) == 1) {
        std.debug.warn("cannot connect to display\n", .{});
        return error.CouldNotConnect;
    }

    // loop through screens
    var iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
    screen = @ptrCast(*xcb_screen_t, iter.data);

    const values: u32 = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_BUTTON_PRESS;
    // subscribe to events
    _ = xcb_change_window_attributes(conn, screen.root, XCB_CW_EVENT_MASK, &values);
    _ = xcb_flush(conn);
}

pub fn run() !void {
    std.debug.warn("Waiting for events...\n", .{});
    while (true) {
        while (xcb_poll_for_event(conn)) |e| {
            events.handle_event(e);
        }
        _ = xcb_flush(conn);
    }
}

pub fn shutdown() void {
    xcb_disconnect(conn);
}
