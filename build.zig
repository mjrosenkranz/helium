const builtin = @import("builtin");
const std = @import("std");
const Builder = std.build.Builder;
const Mode = builtin.Mode;

pub fn build(b: *Builder) void {
    const mode = b.standardReleaseOptions();
    const target = b.standardTargetOptions(.{});
    const exe = b.addExecutable("example", "src/main.zig");
    exe.addPackage(.{ .name = "xcb", .path = "src/xcb_header.zig" });
    exe.setBuildMode(mode);
    exe.setTarget(target);
    exe.linkLibC();
    exe.linkSystemLibrary("xcb");
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    const desc = "Run it!";
    const run_step = b.step("run", desc);
    run_step.dependOn(&run_cmd.step);
}
