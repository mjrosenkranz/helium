const std = @import("std");
const wm = @import("wm.zig");
usingnamespace @import("c.zig");

const geom = struct {
    x: i16,
    y: i16,
    w: u16,
    h: u16,
};
pub fn get_geom(win: *xcb_drawable_t) geom {
    var g = xcb_get_geometry_reply(wm.conn,
        xcb_get_geometry(wm.conn, win.*),
        null).?.*;

    std.debug.warn("x {} y {} w {} h {}\n", .{
        g.x,
        g.y,
        g.width,
        g.height,
    });

    return .{
        .x = g.x,
        .y = g.y,
        .w = g.width,
        .h = g.height,
    };
}
