const std = @import("std");
const xcb = @import("xcb");

pub fn main() !void {
    var display: ?[*]const u8 = null;
    var screen: ?[*]c_int = null;

    var conn = try xcb.Xcb.connect(display, screen);
    defer conn.disconnect();

    var s = conn.setupRootsIterator();
    const window_id = conn.generateId();
    const x: i16 = 0;
    const y: i16 = 0;
    const window_width: u16 = 640;
    const window_height: u16 = 480;
    const border_width: u16 = 0;
    const win_class = xcb.XCB_WINDOW_CLASS_INPUT_OUTPUT;
    const depth = xcb.XCB_COPY_FROM_PARENT;
    const mask = xcb.XCB_CW_BACK_PIXEL;
    const values: [*]const u32 = &[_]u32{0x00ff00};

    _ = conn.createWindow(depth, window_id, s.root, x, y, window_width, window_height, border_width, win_class, s.root_visual, mask, values);
    _ = conn.mapWindow(window_id);
    _ = conn.flush();
    std.time.sleep(1e10);
} //
