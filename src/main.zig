const std = @import("std");
const wm = @import("wm.zig");

pub fn main() !void {
    //var state = wm.WM.new();
    try wm.setup();
    try wm.run();

    wm.shutdown();
}
