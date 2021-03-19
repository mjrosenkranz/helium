const builtin = @import("builtin");
const std = @import("std");
const Builder = std.build.Builder;
const Mode = builtin.Mode;

pub fn build(b: *Builder) void {
    const mode = b.standardReleaseOptions();
    const target = b.standardTargetOptions(.{});
    const exe = b.addExecutable("example", "src/surface_xcb.zig");
    exe.addPackage(.{ .name = "xcb", .path = "src/xcb.zig" });
    exe.setBuildMode(mode);
    exe.setTarget(target);
    exe.linkLibC();
    exe.linkSystemLibrary("cairo");
    exe.linkSystemLibrary("xcb");
    exe.linkSystemLibrary("pangocairo");
    exe.install(); // uncomment to build ALL exes (it takes ~2 minutes)
    // exes_step.dependOn(&exe.step);

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    const desc = "Run it!";
    const run_step = b.step("example", desc);
    run_step.dependOn(&run_cmd.step);

    // b.default_step.dependOn(test_all_modes_step);
}
