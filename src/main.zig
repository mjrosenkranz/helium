const std = @import("std");
const wm = @import("wm.zig");

pub fn main() !void {
    var state = wm.WM.new();
    try state.setup();
    try state.run();

    state.shutdown();
}
