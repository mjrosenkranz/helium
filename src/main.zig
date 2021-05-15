const std = @import("std");
usingnamespace @import("c.zig");

const State = struct {
    conn: *xcb_connection_t,
    screen: *xcb_screen_t,
    const Self = @This();

    pub fn new() Self {
        return .{
            .conn = undefined,
            .screen = undefined
        };
    }

    pub fn setup(self: *Self) !void {
        self.conn = xcb_connect(null, null).?;
        // check if we connected
        if (xcb_connection_has_error(self.conn) == 1) {
            std.debug.warn("cannot connect to display\n", .{});
            return error.CouldNotConnect;
        }

        // loop through screens
        var iter = xcb_setup_roots_iterator(xcb_get_setup(self.conn));
        self.screen = @ptrCast(*xcb_screen_t, iter.data);

        const values: u32 = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_BUTTON_PRESS;
        // subscribe to events
        _ = xcb_change_window_attributes(self.conn, self.screen.root, XCB_CW_EVENT_MASK, &values);
        _ = xcb_flush(self.conn);
    }

    pub fn event_loop(self: *Self) !void {
        std.debug.warn("Waiting for events...\n", .{});
        //var event: ?*xcb_generic_event_t = undefined;
        while (true) {
            while (xcb_poll_for_event(self.conn)) |e| {
                std.debug.warn("Event: {}\n", .{e.*.response_type & ~@as(u32, 0x80)});
            }
            _ = xcb_flush(self.conn);
        }
    }

    pub fn shutdown(self: *Self) void {
        xcb_disconnect(self.conn);
    }
};

pub fn main() !void {
    var state = State.new();
    try state.setup();
    try state.event_loop();

    state.shutdown();
}
