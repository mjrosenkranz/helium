const std = @import("std");
usingnamespace @import("xcb");

pub fn main() !void {
    // connect to the display given by the DISPLAY variable
    // default to screen 0
    //var conn = try xcb.conn.connect(null, null);
    var conn = xcb_connect(null, null);
    // check if we connected
    if (xcb_connection_has_error(conn) == 1) {
        std.debug.warn("cannot connect to display\n", .{});
        return error.CouldNotConnect;
    }
    defer xcb_disconnect(conn);

    const s = @ptrCast(*xcb_screen_t, xcb_setup_roots_iterator(xcb_get_setup(conn)).data);
    const window_id = xcb_generate_id(conn);
    const x: i16 = 0;
    const y: i16 = 0;
    const window_width: u16 = 640;
    const window_height: u16 = 480;
    const border_width: u16 = 0;
    const win_class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
    const depth = XCB_COPY_FROM_PARENT;
    const mask = XCB_CW_BACK_PIXEL;
    const values: [*]const u32 = &[_]u32{0x00ff00};

    _ = xcb_create_window(conn, depth, window_id, s.root, x, y, window_width, window_height, border_width, win_class, s.root_visual, mask, values);
    _ = xcb_map_window(conn, window_id);
    _ = xcb_flush(conn);

    var buf: [20]u8 = undefined;
    _ = try std.io.getStdIn().read(&buf);
}
